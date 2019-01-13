/*
    LinKNX KNX home automation platform
    Copyright (C) 2007 Jean-François Meessen <linknx@ouaye.net>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "timermanager.h"
#include "suncalc.h"
#include "services.h"
#include <iostream>
#include <ctime>
#include <iomanip>

Logger& TimerManager::logger_m(Logger::getInstance("TimerManager"));

DateTime::DateTime(const tm *time)
{
	fields_m[0] = time->tm_year;	
	fields_m[1] = time->tm_mon;	
	fields_m[2] = time->tm_mday;	
	fields_m[3] = time->tm_hour;	
	fields_m[4] = time->tm_min;	
	freeFields_m = 0x1F;
	weekdays_m = 0;
}

int DateTime::getField(FieldType field) const
{
	return fields_m[field];	
}

void DateTime::setField(FieldType field, int value, bool fixesIfChanged)
{
	if (value == -1)
	{
		freeFields_m |= 1 << field;
	}
	else
	{
		// Year, month and day cannot be fixed if weekdays are constrained.
		bool fixes = fixesIfChanged && (field > Day || weekdays_m == 0);
		if (fixes) freeFields_m &= ~(1 << field);
		bool changed = fields_m[field] != value;
		fields_m[field] = value;
		if (field != Minute)
		{
			resetFieldIfFree((FieldType)(field + 1), true);
		}
	}
}

bool DateTime::isFieldFixed(FieldType detail) const
{
	return !isFieldFree(detail);
}

bool DateTime::isFieldFree(FieldType detail) const
{
	return (freeFields_m & (1 << detail)) != 0;
}

bool DateTime::tryIncreaseClosestGreaterFreeField(FieldType current)
{
	// Search the closest field that is not constrained going towards the
	// year.
	while (!isFieldFree(current) && current > Invalid)
	{
		current = (FieldType)(current - 1);
	}

	if (current != Invalid)
	{
		increaseField(current, false);
		return true;
	}
	else
	{
		return false;
	}
}

int DateTime::increaseField(FieldType fieldId, bool fixes)
{
	int newValue = fields_m[fieldId] + 1;
	setField(fieldId, newValue, fixes);

	return newValue;
}

time_t DateTime::getTime(tm *outBrokenDownTime) const
{
	tm *t = outBrokenDownTime;
	if (t == NULL) t = new tm();

	t->tm_year = fields_m[Year];
	t->tm_mon = fields_m[Month];
	t->tm_mday = fields_m[Day];
	t->tm_hour = fields_m[Hour];
	t->tm_min = fields_m[Minute];
	t->tm_sec = 0;
	t->tm_isdst = -1;

	time_t time = mktime(t);
	if (outBrokenDownTime == NULL)
	{
		delete t;
		t = NULL;
	}
	return time;
}

DateTime::ProjectionResult DateTime::projectOnActualCalendar()
{
	tm time;
	getTime(&time);
	
	DateTime projected(&time);
	bool isChanged = false;
	bool hasFreeField = false;
	for (FieldType fieldId = Year; fieldId <= Minute; fieldId = (FieldType)(fieldId + 1))
	{
		if (getField(fieldId) != projected.getField(fieldId))
		{
			if (isFieldFree(fieldId))
			{
				fields_m[fieldId] = projected.getField(fieldId);
				isChanged = true;
			}
			else
			{
				if (isChanged)
				{
					return Projection_Changed;
				}
				else
				{
					return hasFreeField ? Projection_Failed : Projection_Impossible;
				}
			}
		}
		hasFreeField |= isFieldFree(fieldId);
	}

	return isChanged ? Projection_Changed : Projection_Succeeded;
}

DateTime::ResolutionResult DateTime::tryResolve(const DateTime &current, FieldType from, FieldType to)
{
	if (!tryResolveUnprojected(current, from, to)) return Resolution_Impossible;

	switch (projectOnActualCalendar())
	{
		case Projection_Changed:
			return tryResolve(current, from, to);

		case Projection_Succeeded:
			return Resolution_Resolved;

		case Projection_Failed:
			return Resolution_Unresolved;

		case Projection_Impossible:
			return Resolution_Impossible;

		default:
			throw ticpp::Exception("Unsupported projection result.");
	}	
}

bool DateTime::tryResolveUnprojected(const DateTime &current, FieldType from, FieldType to)
{
	while (tryResolveWithoutWeekdays(current, from, to))
	{
		if (isCompatibleWithWeekDays())
		{
			return true;
		}
		else
		{
			if (!this->tryIncreaseClosestGreaterFreeField(Day)) return false;
		}
	}

	return false;
}

/** Attempts to ensure constraints are met and adjusts free fields if
 * required so that the date/time represented by this object satisfies
 * the various constraints and comes after current date/time. */
bool DateTime::tryResolveWithoutWeekdays(const DateTime &current, FieldType from, FieldType to)
{
	// Nothing to do if target is already after current.
	for (FieldType fieldId = Year; fieldId < from; fieldId = (FieldType)(fieldId + 1))
	{
		int targetField = this->getField(fieldId);
		int currentField = current.getField(fieldId);
		if (targetField > currentField) return true;
		if (targetField < currentField) return false; // Target is after current and those fields are not adjustable.
	}

	// So far, target's fields are equal to current. Let's see how it goes
	// with the remaining fields.
	for (FieldType fieldId = from; fieldId <= to; fieldId = (FieldType)(fieldId + 1))
	{
		int currentField = current.getField(fieldId);
		int targetField = this->getField(fieldId);

		if (targetField < currentField)
		{
			if (this->isFieldFree(fieldId))
			{
				// Increase current field.
				this->setField(fieldId, currentField, false);
			}
			else
			{
				return this->tryIncreaseClosestGreaterFreeField(fieldId);
			}
		}
		else if (targetField > currentField)
		{
			// No need to inspect remaining fields: target is already after
			// current.
			return true;
		}
	}
	return true;
}

bool DateTime::isCompatibleWithWeekDays() const
{
	if (weekdays_m != 0)
	{
		struct tm time;
		this->getTime(&time);
		int wd = (time.tm_wday + 6) % 7;
		return (weekdays_m & (1 << wd)) != 0;
	}
	else
	{
		return true;
	}
}

bool DateTime::operator>(const DateTime &other) const
{
	return getTime() > other.getTime();
}

bool DateTime::operator<(const DateTime &other) const
{
	return getTime() < other.getTime();
}

void DateTime::resetFieldIfFree(FieldType field, bool recurses)
{
	if (isFieldFree(field))
	{
		int value = field == Day ? 1 : 0;
		setField(field, value, false);
	}
	if (recurses && field != Minute)
	{
		resetFieldIfFree((FieldType)(field + 1), true);
	}
}

TimerManager::TimerManager()
{}

TimerManager::~TimerManager()
{
    StopDelete ();
}

TimerManager::TimerCheck TimerManager::checkTaskList(time_t now)
{
    if (taskList_m.empty())
        return Long;

    TimerTask* first = taskList_m.front();
    time_t nextExec = first->getExecTime();
    if (nextExec > now)
        return Short;
    
    if (nextExec > now-60)
    {
        logger_m.infoStream() << "TimerTask execution. " << nextExec << endlog;
        first->onTimer(now);
    }
    else
        logger_m.warnStream() << "TimerTask skipped due to clock skew or heavy load. " << nextExec << endlog;
    
    if (first == taskList_m.front())
    {
        // If the taskList was modified, do not remove the first item
        // because it's not the one we just called onTimer for.
        taskList_m.pop_front();
        first->reschedule(now);
    }
    return Immediate;
}

void TimerManager::Run (pth_sem_t * stop1)
{
    pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
    logger_m.debugStream() << "Starting TimerManager loop." << endlog;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
        TimerCheck interval = checkTaskList(time(0));
        if (interval == Immediate)
            tv.tv_sec = 0;
        else if (interval == Short)
            tv.tv_sec = 1;
        else
            tv.tv_sec = 10;
        pth_select_ev(0,0,0,0,&tv,stop);
    }
    logger_m.debugStream() << "Out of TimerManager loop." << endlog;
    pth_event_free (stop, PTH_FREE_THIS);
}

void TimerManager::addTask(TimerTask* task)
{
    TaskList_t::iterator it;
    time_t execTime = task->getExecTime();
    for (it = taskList_m.begin(); it != taskList_m.end(); it++)
    {
        if (execTime < (*it)->getExecTime())
            break;
    }
    taskList_m.insert(it, task);
}

void TimerManager::removeTask(TimerTask* task)
{
    taskList_m.remove(task);
}

void TimerManager::statusXml(ticpp::Element* pStatus)
{
    TaskList_t::iterator it;
    for (it = taskList_m.begin(); it != taskList_m.end(); it++)
    {
        ticpp::Element pElem("task");
        (*it)->statusXml(&pElem);
        pStatus->LinkEndChild(&pElem);
    }
}

TimeSpec* TimeSpec::create(const std::string& type, ChangeListener* cl)
{
    if (type == "variable")
        return new VariableTimeSpec(cl);
    if (type == "sunrise")
        return new SunriseTimeSpec();
    if (type == "sunset")
        return new SunsetTimeSpec();
    if (type == "noon")
        return new SolarNoonTimeSpec();
    else
        return new TimeSpec();
}

TimeSpec* TimeSpec::create(ticpp::Element* pConfig, ChangeListener* cl)
{
    std::string type = pConfig->GetAttribute("type");
    TimeSpec* timeSpec = TimeSpec::create(type, cl);
    if (timeSpec == 0)
    {
        std::stringstream msg;
        msg << "TimeSpec type not supported: '" << type << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    timeSpec->importXml(pConfig);
    return timeSpec;
}

TimeSpec::TimeSpec()
    : min_m(-1), hour_m(-1), mday_m(-1), mon_m(-1), year_m(-1), wdays_m(All), offset_m(0), exception_m(DontCare)
{}

TimeSpec::TimeSpec(int min, int hour, int mday, int mon, int year, int offset)
    : min_m(min), hour_m(hour), mday_m(mday), mon_m(mon), year_m(year), wdays_m(All), offset_m(offset), exception_m(DontCare)
{
    if (year_m >= 1900)
        year_m -= 1900;
    if (mon_m > 0)
        mon_m --;
}

TimeSpec::TimeSpec(int min, int hour, int wdays, ExceptionDays exception)
    : min_m(min), hour_m(hour), mday_m(-1), mon_m(-1), year_m(-1), wdays_m(wdays), offset_m(0), exception_m(exception)
{}

bool TimeSpec::isValid() const
{
	bool hasDay = mday_m != -1;
	bool hasMonth = mon_m != -1;
	bool hasYear = year_m != -1;

	if (hasDay && hasMonth && hasYear)
	{
		tm timeinfo;
		timeinfo.tm_year = year_m;
		timeinfo.tm_mon = mon_m;
		timeinfo.tm_mday = mday_m;
		timeinfo.tm_hour = 0;
		timeinfo.tm_min = 0;
		timeinfo.tm_sec = 0;
		timeinfo.tm_isdst = -1;

		// Use C Library to resolve the time spec.
		mktime(&timeinfo);

		// Make sure that did not change the time spec or it is not deemed
		// valid.
		return timeinfo.tm_year == year_m && timeinfo.tm_mon == mon_m && timeinfo.tm_mday == mday_m;
	}

	if (hasMonth)
	{
		if (mon_m < 0 || mon_m > 11) return false;
	}
	if (hasDay)
	{
		if (mday_m < 1) return false;
		int maxMonthDay = 31;
		if (hasMonth) // But year cannot be defined here.
		{
			if (mon_m == 1)
			{
				maxMonthDay = 29;	
			}
			else if (mon_m == 3 || mon_m == 5 || mon_m == 8 || mon_m == 10)
			{
				maxMonthDay = 30;
			}
		}
		if (mday_m > maxMonthDay) return false;
	}

	if (hour_m != -1 && (hour_m < 0 || hour_m > 23)) return false;
	if (min_m != -1 && (min_m < 0 || min_m > 59)) return false;

	return true;
}

void TimeSpec::importXml(ticpp::Element* pConfig)
{
    pConfig->GetAttributeOrDefault("year", &(year_m), -1);
    pConfig->GetAttributeOrDefault("month", &(mon_m), -1);
    pConfig->GetAttributeOrDefault("day", &(mday_m), -1);
    pConfig->GetAttributeOrDefault("hour", &(hour_m), -1);
    pConfig->GetAttributeOrDefault("min", &(min_m), -1);
    if (year_m >= 1900)
        year_m -= 1900;
    if (mon_m >= 0)
        mon_m--;
    std::string wdays;
    pConfig->GetAttribute("wdays", &wdays, false);
    wdays_m = All;
    if (wdays.find('1') != wdays.npos)
        wdays_m |= Mon;
    if (wdays.find('2') != wdays.npos)
        wdays_m |= Tue;
    if (wdays.find('3') != wdays.npos)
        wdays_m |= Wed;
    if (wdays.find('4') != wdays.npos)
        wdays_m |= Thu;
    if (wdays.find('5') != wdays.npos)
        wdays_m |= Fri;
    if (wdays.find('6') != wdays.npos)
        wdays_m |= Sat;
    if (wdays.find('7') != wdays.npos)
        wdays_m |= Sun;

    std::string exception;
    pConfig->GetAttribute("exception", &exception, false);
    if (exception == "yes" || exception == "true")
        exception_m = Yes;
    else if (exception == "no" || exception == "false")
        exception_m = No;
    else
        exception_m = DontCare;

    offset_m = RuleServer::parseDuration(pConfig->GetAttribute("offset"), true);

    infoStream("TimeSpec")
    << year_m+1900 << "-"
    << mon_m+1 << "-"
    << std::setfill('0') << std::setw(2)
    << mday_m << " "
    << std::setfill('0') << std::setw(2)
    << hour_m << ":"
    << std::setfill('0') << std::setw(2)
    << min_m << ":0 (wdays="
    << wdays_m << "; exception=" << exception_m << ")" << endlog;
}

void TimeSpec::exportXml(ticpp::Element* pConfig)
{
    if (hour_m != -1)
        pConfig->SetAttribute("hour", hour_m);
    if (min_m != -1)
        pConfig->SetAttribute("min", min_m);
    if (mday_m != -1)
        pConfig->SetAttribute("day", mday_m);
    if (mon_m != -1)
        pConfig->SetAttribute("month", mon_m+1);
    if (year_m != -1)
        pConfig->SetAttribute("year", year_m+1900);
    if (exception_m == Yes)
        pConfig->SetAttribute("exception", "yes");
    else if (exception_m == No)
        pConfig->SetAttribute("exception", "no");
    if (wdays_m != All)
    {
        std::stringstream wdays;
        if (wdays_m & Mon)
            wdays << '1';
        if (wdays_m & Tue)
            wdays << '2';
        if (wdays_m & Wed)
            wdays << '3';
        if (wdays_m & Thu)
            wdays << '4';
        if (wdays_m & Fri)
            wdays << '5';
        if (wdays_m & Sat)
            wdays << '6';
        if (wdays_m & Sun)
            wdays << '7';
        pConfig->SetAttribute("wdays", wdays.str());
    }

    if (offset_m != 0)
        pConfig->SetAttribute("offset", RuleServer::formatDuration(offset_m));
}

void TimeSpec::getDay(const tm &current, int &mday, int &mon, int &year, int &wdays) const
{
    mday = mday_m;
    mon = mon_m;
    year = year_m;
	wdays = wdays_m;
}

void TimeSpec::getTime(int mday, int mon, int year, int &min, int &hour) const
{
	min = min_m;
	hour = hour_m;
}

VariableTimeSpec::VariableTimeSpec(ChangeListener* cl)
   	: time_m(0), date_m(0), cl_m(cl)
{
}

VariableTimeSpec::VariableTimeSpec(ChangeListener* cl, int min, int hour, int mday, int mon, int year, int offset)
   	: TimeSpec(min, hour, mday, mon, year, offset), time_m(0), date_m(0), cl_m(cl)
{
}

VariableTimeSpec::~VariableTimeSpec()
{
    if (cl_m && time_m)
        time_m->removeChangeListener(cl_m);
    if (cl_m && date_m)
        date_m->removeChangeListener(cl_m);
    if (time_m)
        time_m->decRefCount();
    if (date_m)
        date_m->decRefCount();
}


void VariableTimeSpec::importXml(ticpp::Element* pConfig)
{
    TimeSpec::importXml(pConfig);
    std::string time = pConfig->GetAttribute("time");
    if (time != "")
    {
        Object* obj = ObjectController::instance()->getObject(time); 
        time_m = dynamic_cast<TimeObject*>(obj); 
        if (!time_m)
        {
            obj->decRefCount();
            std::stringstream msg;
            msg << "Wrong Object type for time in VariableTimeSpec: '" << time << "'" << std::endl;
            throw ticpp::Exception(msg.str());
        }
        if (cl_m)
            time_m->addChangeListener(cl_m);
    }
    std::string date = pConfig->GetAttribute("date");
    if (date != "")
    {
        Object* obj = ObjectController::instance()->getObject(date); 
        date_m = dynamic_cast<DateObject*>(obj); 
        if (!date_m)
        {
            obj->decRefCount();
            std::stringstream msg;
            msg << "Wrong Object type for date in VariableTimeSpec: '" << date << "'" << std::endl;
            throw ticpp::Exception(msg.str());
        }
        if (cl_m)
            date_m->addChangeListener(cl_m);
    }

}

void VariableTimeSpec::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "variable");
    TimeSpec::exportXml(pConfig);
    if (time_m)
        pConfig->SetAttribute("time", time_m->getID());
    if (date_m)
        pConfig->SetAttribute("date", date_m->getID());
}

void VariableTimeSpec::getDay(const tm &current, int &mday, int &mon, int &year, int &wdays) const
{
	TimeSpec::getDay(current, mday, mon, year, wdays);

	int min, hour = -1;
	getDataFromObject(min, hour, mday, mon, year, wdays);
}

void VariableTimeSpec::getTime(int mday, int mon, int year, int &min, int &hour) const
{
	TimeSpec::getTime(mday, mon, year, min, hour);

	int dummyMday = -1;
	int dummyMon = -1;
	int dummyYear = -1;
	int dummyWdays = -1;
	getDataFromObject(min, hour, dummyMday, dummyMon, dummyYear, dummyWdays);
}

void VariableTimeSpec::getDataFromObject(int &min, int &hour, int &mday, int &mon, int &year, int &wdays) const
{
    if (time_m)
    {
        int sec_l, min_l, hour_l, wday_l;
        time_m->getTime(&wday_l, &hour_l, &min_l, &sec_l);
        if (min == -1)
            min = min_l;
        if (hour == -1)
            hour = hour_l;
        if (wdays == All && wday_l > 0)
            wdays = 1 << (wday_l - 1);
    }
    if (date_m)
    {
        int day_l, month_l, year_l;
        date_m->getDate(&day_l, &month_l, &year_l);
        if (mday == -1)
            mday = day_l;    
        if (mon == -1)
            mon = month_l-1;    
        if (year == -1)
            year = year_l-1900;    
    }
}

Logger& PeriodicTask::logger_m(Logger::getInstance("PeriodicTask"));

PeriodicTask::PeriodicTask(ChangeListener* cl)
        : at_m(0), until_m(0), during_m(0), after_m(-1), nextExecTime_m(0), cl_m(cl), value_m(false)
{}

PeriodicTask::~PeriodicTask()
{
    Services::instance()->getTimerManager()->removeTask(this);
    if (at_m)
        delete at_m;
    if (until_m)
        delete until_m;
}

void PeriodicTask::onTimer(time_t time)
{
    value_m = !value_m;
    if (cl_m)
        cl_m->onChange(0);
    if (during_m == 0 && value_m)
    {
        value_m = false;
        if (cl_m)
            cl_m->onChange(0);
    }
}

void PeriodicTask::onChange(Object* object)
{
    Services::instance()->getTimerManager()->removeTask(this);
    reschedule(0);
}

void PeriodicTask::reschedule(time_t now)
{
    if (now == 0)
        now = time(0);
    if (nextExecTime_m == 0 && during_m != 0)
    {
        // first schedule. check if value must be on or off (except if timer is instantaneous)
        time_t start, stop;
        if (during_m != -1)
        {
            if (after_m == -1)
                stop = findNext(now-during_m, at_m)+during_m;
            else
                stop = now + during_m;
        }
        else
            stop = findNext(now, until_m);

        if (after_m != -1)
            start = now + after_m;
        else
            start = findNext(now, at_m);

        if (stop < start)
        {
            value_m = true;
            nextExecTime_m = stop;
        }
        else
        {
            value_m = false;
            nextExecTime_m = start;
        }
    }
    else if (value_m)
    {
        if (during_m != -1)
            nextExecTime_m = now + during_m;
        else
            nextExecTime_m = findNext(now, until_m);
    }
    else
    {
        if (after_m != -1)
            nextExecTime_m = now + after_m;
        else
            nextExecTime_m = findNext(now, at_m);

    }
    if (nextExecTime_m != 0)
    {
        struct tm timeinfo;
        memcpy(&timeinfo, localtime(&nextExecTime_m), sizeof(struct tm));
        logger_m.infoStream() << "Rescheduled at "
        << timeinfo.tm_year + 1900 << "-"
        << timeinfo.tm_mon + 1 << "-"
        << timeinfo.tm_mday << " "
        << std::setfill('0') << std::setw(2)
        << timeinfo.tm_hour << ":"
        << std::setfill('0') << std::setw(2)
        << timeinfo.tm_min << ":"
        << std::setfill('0') << std::setw(2)
        << timeinfo.tm_sec << " ("
        << nextExecTime_m << ")" << endlog;
        Services::instance()->getTimerManager()->addTask(this);
    }
    else
        logger_m.infoStream() << "Not rescheduled" << endlog;

}

time_t PeriodicTask::mktimeNoDst(struct tm * timeinfo)
{
    time_t ret;
    int dst = timeinfo->tm_isdst;
    ret = mktime(timeinfo);
    if (dst != timeinfo->tm_isdst)
    {
        logger_m.infoStream() << "PeriodicTask: DST change detected" << endlog;
        if (dst == 1) // If day changed due to DST adjustment, we revert the change.
            timeinfo->tm_hour++;
        else if (dst == 0 && timeinfo->tm_hour == 3)
        {
            // If between 3am and 4am, do not go back before 3am
            // because we would fall inside the non-existing hour
            timeinfo->tm_hour = 3;
            timeinfo->tm_min = 0;
            timeinfo->tm_sec = 0;
        }
        else
            timeinfo->tm_hour--;
        ret = mktime(timeinfo);
    }
    return ret;
}

time_t PeriodicTask::findNext(time_t start, TimeSpec* next)
{
    struct tm timeinfostruct;
    struct tm * timeinfo;
    if (!next)
    {
        logger_m.infoStream() << "PeriodicTask: no more schedule available" << endlog;
        return 0;
    }
    // make a copy of value returned by localtime to avoid interference
    // with other calls to localtime or gmtime
    memcpy(&timeinfostruct, localtime(&start), sizeof(struct tm));
    timeinfo = &timeinfostruct;

	// Move forward 1 minute.
	timeinfo->tm_min++;
	mktime(timeinfo);
    
    int dayOfMonth, month, year, weekdays;
    next->getDay(*timeinfo, dayOfMonth, month, year, weekdays);

	// Weekdays and {day, month, year} are mutually exclusive. Give priority
	// to weekdays.
	if (weekdays != 0)
	{
		year = -1;
		month = -1;
		dayOfMonth = -1;
	}

	// Start at current time.
	DateTime current(timeinfo);
	DateTime target(timeinfo);

	// Fix all constrained fields.
	target.setYear(year);
	target.setMonth(month);
	target.setDay(dayOfMonth);
	target.setWeekdays(weekdays);
    
	// Find day.
	switch (target.tryResolve(current, DateTime::Year, DateTime::Day))
	{	
		case DateTime::Resolution_Resolved:
			break;
		case DateTime::Resolution_Unresolved:
			return goToNextDayAndFindNext(target, next);
		case DateTime::Resolution_Impossible:
			// No schedule available.
			logger_m.infoStream() << "No more schedule available" << endlog;
			return 0;
	}

	// Find time.
	int min, hour = -1;
	next->getTime(target.getDay(), target.getMonth(), target.getYear(), min, hour);
	target.setHour(hour);
	target.setMinute(min);
	switch (target.tryResolve(current, DateTime::Hour, DateTime::Minute))
	{	
		case DateTime::Resolution_Resolved:
			break;
		case DateTime::Resolution_Unresolved:
			return goToNextDayAndFindNext(target, next);
		case DateTime::Resolution_Impossible:
			// No schedule available.
			logger_m.infoStream() << "No more schedule available" << endlog;
			return 0;
	}

    time_t nextExecTime = target.getTime(timeinfo);
    if (nextExecTime <= start)
    {
        logger_m.errorStream() << "Timer error, nextExecTime(" << nextExecTime << ") is before startTime(" << start << ")" << endlog;
        return 0;
    }

    TimeSpec::ExceptionDays exception = next->getExceptions();
    if (exception != TimeSpec::DontCare)
    {
        bool isException = Services::instance()->getExceptionDays()->isException(nextExecTime);
        if (isException && exception == TimeSpec::No || !isException && exception == TimeSpec::Yes)
        {
            logger_m.debugStream() << "Calling findNext recursively! (" << nextExecTime << ")" << endlog;

			return goToNextDayAndFindNext(target, next);
        }
    }

	// Apply offset.
    return nextExecTime + next->getOffsetInSeconds();
}

time_t PeriodicTask::goToNextDayAndFindNext(const DateTime &current, TimeSpec* next)
{
	tm timeinfo;
    current.getTime(&timeinfo);

	// Fast forward to 23:59 the same day, so that the next call
	// switches to the next day.
	timeinfo.tm_hour = 23;
	timeinfo.tm_min = 59;
	time_t nextExecTime = mktime(&timeinfo);

	return findNext(nextExecTime, next);
}

void PeriodicTask::statusXml(ticpp::Element* pStatus)
{
    struct tm timeinfo;
    std::stringstream execTime;
    memcpy(&timeinfo, localtime(&nextExecTime_m), sizeof(struct tm));
    execTime << timeinfo.tm_year + 1900 << "-"
    << timeinfo.tm_mon + 1 << "-"
    << timeinfo.tm_mday << " "
    << std::setfill('0') << std::setw(2)
    << timeinfo.tm_hour << ":"
    << std::setfill('0') << std::setw(2)
    << timeinfo.tm_min << ":"
    << std::setfill('0') << std::setw(2)
    << timeinfo.tm_sec;
    pStatus->SetAttribute("next-exec", execTime.str());
    if (cl_m)
        pStatus->SetAttribute("owner", cl_m->getID());
}

Logger& FixedTimeTask::logger_m(Logger::getInstance("FixedTimeTask"));

FixedTimeTask::FixedTimeTask() : execTime_m(0)
{}

FixedTimeTask::~FixedTimeTask()
{
    Services::instance()->getTimerManager()->removeTask(this);
}

void FixedTimeTask::reschedule(time_t now)
{
    if (now == 0)
        now = time(0);
    if (execTime_m > now)
    {
        struct tm timeinfo;
        memcpy(&timeinfo, localtime(&execTime_m), sizeof(struct tm));
        logger_m.infoStream() << "Rescheduled at "
        << timeinfo.tm_year + 1900 << "-"
        << timeinfo.tm_mon + 1 << "-"
        << timeinfo.tm_mday << " "
        << std::setfill('0') << std::setw(2)
        << timeinfo.tm_hour << ":"
        << std::setfill('0') << std::setw(2)
        << timeinfo.tm_min << ":"
        << std::setfill('0') << std::setw(2)
        << timeinfo.tm_sec << " ("
        << execTime_m << ")" << endlog;
        Services::instance()->getTimerManager()->addTask(this);
    }
    else
        logger_m.infoStream() << "Not rescheduled" << endlog;
}

void FixedTimeTask::statusXml(ticpp::Element* pStatus)
{
    struct tm timeinfo;
    std::stringstream execTime;
    memcpy(&timeinfo, localtime(&execTime_m), sizeof(struct tm));
    execTime << timeinfo.tm_year + 1900 << "-"
    << timeinfo.tm_mon + 1 << "-"
    << timeinfo.tm_mday << " "
    << std::setfill('0') << std::setw(2)
    << timeinfo.tm_hour << ":"
    << std::setfill('0') << std::setw(2)
    << timeinfo.tm_min << ":"
    << std::setfill('0') << std::setw(2)
    << timeinfo.tm_sec;
    pStatus->SetAttribute("next-exec", execTime.str());
}

void DaySpec::importXml(ticpp::Element* pConfig)
{
    pConfig->GetAttributeOrDefault("year", &(year_m), -1);
    pConfig->GetAttributeOrDefault("month", &(mon_m), -1);
    pConfig->GetAttributeOrDefault("day", &(mday_m), -1);
    if (year_m >= 1900)
        year_m -= 1900;
    if (mon_m >= 0)
        mon_m--;

    debugStream("DaySpec")
    << year_m+1900 << "-"
    << mon_m+1 << "-"
    << mday_m << endlog;
}

void DaySpec::exportXml(ticpp::Element* pConfig)
{
    if (mday_m != -1)
        pConfig->SetAttribute("day", mday_m);
    if (mon_m != -1)
        pConfig->SetAttribute("month", mon_m+1);
    if (year_m != -1)
        pConfig->SetAttribute("year", year_m+1900);
}


ExceptionDays::ExceptionDays()
{}

ExceptionDays::~ExceptionDays()
{
    clear();
}

void ExceptionDays::clear()
{
    DaysList_t::iterator it;
    for (it = daysList_m.begin(); it != daysList_m.end(); it++)
        delete (*it);
    daysList_m.clear();
}

void ExceptionDays::importXml(ticpp::Element* pConfig)
{
    ticpp::Iterator< ticpp::Element > child;
    if (pConfig->GetAttribute("clear") == "true")
        clear();
    for ( child = pConfig->FirstChildElement(false); child != child.end(); child++ )
    {
        if (child->Value() == "date")
        {
            DaySpec* day = new DaySpec();
            day->importXml(&(*child));
            daysList_m.push_back(day);
        }
        else
        {
            throw ticpp::Exception("Invalid element inside 'exceptiondays' section");
        }
    }
}

void ExceptionDays::exportXml(ticpp::Element* pConfig)
{
    DaysList_t::iterator it;
    for (it = daysList_m.begin(); it != daysList_m.end(); it++)
    {
        ticpp::Element pElem("date");
        (*it)->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

bool ExceptionDays::isException(time_t time)
{
    struct tm timeinfo;
    memcpy(&timeinfo, localtime(&time), sizeof(struct tm));

    DaysList_t::iterator it;
    for (it = daysList_m.begin(); it != daysList_m.end(); it++)
    {
        if (((*it)->year_m == -1 || (*it)->year_m == timeinfo.tm_year) &&
                ((*it)->mon_m == -1 || (*it)->mon_m == timeinfo.tm_mon) &&
                ((*it)->mday_m == -1 || (*it)->mday_m == timeinfo.tm_mday))
        {
            infoStream("ExceptionDays")
            << timeinfo.tm_year+1900 << "-"
            << timeinfo.tm_mon+1 << "-"
            << timeinfo.tm_mday << " is an exception day!" << endlog;
            return true;
        }
    }
    return false;
}

void ExceptionDays::addDay(DaySpec* day)
{
    DaysList_t::iterator it;
    for (it = daysList_m.begin(); it != daysList_m.end(); it++)
    {}
    daysList_m.insert(it, day);
}

void ExceptionDays::removeDay(DaySpec* day)
{
    daysList_m.remove(day);
}
