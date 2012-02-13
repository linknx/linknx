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

Logger& TimerManager::logger_m(Logger::getInstance("TimerManager"));

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
    : min_m(-1), hour_m(-1), mday_m(-1), mon_m(-1), year_m(-1), wdays_m(All), exception_m(DontCare)
{}

TimeSpec::TimeSpec(int min, int hour, int mday, int mon, int year)
    : min_m(min), hour_m(hour), mday_m(mday), mon_m(mon), year_m(year), wdays_m(All), exception_m(DontCare)
{
    if (year_m >= 1900)
        year_m -= 1900;
    if (mon_m > 0)
        mon_m --;
}

TimeSpec::TimeSpec(int min, int hour, int wdays, ExceptionDays exception)
    : min_m(min), hour_m(hour), mday_m(-1), mon_m(-1), year_m(-1), wdays_m(wdays), exception_m(exception)
{}

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

    infoStream("TimeSpec")
    << year_m+1900 << "-"
    << mon_m+1 << "-"
    << mday_m << " "
    << hour_m << ":"
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
}

void TimeSpec::getData(int *min, int *hour, int *mday, int *mon, int *year, int *wdays, ExceptionDays *exception, const struct tm * timeinfo)
{
    *min = min_m;
    *hour = hour_m;
    *mday = mday_m;
    *mon = mon_m;
    *year = year_m;
    *wdays = wdays_m;
    *exception = exception_m;
}

VariableTimeSpec::VariableTimeSpec(ChangeListener* cl) : time_m(0), date_m(0), cl_m(cl)
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

void VariableTimeSpec::getData(int *min, int *hour, int *mday, int *mon, int *year, int *wdays, ExceptionDays *exception, const struct tm * timeinfo)
{
    *min = min_m;
    *hour = hour_m;
    *mday = mday_m;
    *mon = mon_m;
    *year = year_m;
    *wdays = wdays_m;
    *exception = exception_m;

    if (time_m)
    {
        int sec_l, min_l, hour_l, wday_l;
        time_m->getTime(&wday_l, &hour_l, &min_l, &sec_l);
        if (*min == -1)
            *min = min_l;    
        if (*hour == -1)
            *hour = hour_l;    
        if (*wdays == All && wday_l > 0)
            *wdays = 1 << (wday_l - 1);    
    }
    if (date_m)
    {
        int day_l, month_l, year_l;
        date_m->getDate(&day_l, &month_l, &year_l);
        if (*mday == -1)
            *mday = day_l;    
        if (*mon == -1)
            *mon = month_l-1;    
        if (*year == -1)
            *year = year_l-1900;    
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
        << timeinfo.tm_hour << ":"
        << timeinfo.tm_min << ":"
        << timeinfo.tm_sec << " ("
        << nextExecTime_m << ")" << endlog;
        Services::instance()->getTimerManager()->addTask(this);
    }
    else
        logger_m.infoStream() << "Not rescheduled" << endlog;

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
    
    timeinfo->tm_min++;
    if (timeinfo->tm_min > 59)
    {
        timeinfo->tm_hour++;
        timeinfo->tm_min = 0;
    }
    
    int min, hour, mday, mon, year, wdays;
    TimeSpec::ExceptionDays exception;
    next->getData(&min, &hour, &mday, &mon, &year, &wdays, &exception, timeinfo);
    
    if (min != -1)
    {
        if  (timeinfo->tm_min > min)
            timeinfo->tm_hour++;
        timeinfo->tm_min = min;
    }
    if (timeinfo->tm_hour > 23)
    {
        timeinfo->tm_mday++;
        timeinfo->tm_hour = 0;
    }
        
    if (hour != -1)
    {
        if (timeinfo->tm_hour > hour)
            timeinfo->tm_mday++;
        if (timeinfo->tm_hour != hour)
        {
            if (min == -1)
                timeinfo->tm_min = 0;
            timeinfo->tm_hour = hour;
        }
    }

    mktime(timeinfo);

    if (wdays == 0)
    {
        if (mday != -1)
        {
            if (timeinfo->tm_mday > mday)
                timeinfo->tm_mon++;
            if (timeinfo->tm_mday != mday)
            {
                if (min == -1)
                    timeinfo->tm_min = 0;
                if (hour == -1)
                    timeinfo->tm_hour = 0;
                timeinfo->tm_mday = mday;
                mktime(timeinfo);
                timeinfo->tm_mday = mday;
            }
        }
        if (timeinfo->tm_mon > 11)
        {
            timeinfo->tm_year++;
            timeinfo->tm_mon = 0;
        }
        if (mon != -1)
        {
            if (timeinfo->tm_mon > mon)
                timeinfo->tm_year++;
            if (timeinfo->tm_mon != mon)
            {
                if (min == -1)
                    timeinfo->tm_min = 0;
                if (hour == -1)
                    timeinfo->tm_hour = 0;
                if (mday == -1)
                    timeinfo->tm_mday = 1;
            }
            timeinfo->tm_mon = mon;
        }
        if (year != -1)
        {
            if (timeinfo->tm_year > year)
            {
                logger_m.infoStream() << "No more schedule available" << endlog;
                return 0;
            }
        }
    }
    else
    {
        int wd = (timeinfo->tm_wday+6) % 7;

        while ((wdays & (1 << wd)) == 0)
        {
            timeinfo->tm_mday++;
            if (timeinfo->tm_mday > 40)
            {
                logger_m.infoStream() << "Wrong weekday specification" << endlog;
                return 0;
            }
            wd = (wd+1) % 7;
        }
    }

    timeinfo->tm_sec = 0;
    time_t nextExecTime = mktime(timeinfo);
    
    if (hour != -1 && timeinfo->tm_hour != hour)
    {
        // deal with clock shift due to DST
        timeinfo->tm_hour = hour;
        nextExecTime = mktime(timeinfo);
    }
    
    if (nextExecTime < 0)
    {
        logger_m.infoStream() << "No more schedule available" << endlog;
        return 0;
    }
    if (exception != TimeSpec::DontCare)
    {
        bool isException = Services::instance()->getExceptionDays()->isException(nextExecTime);
        if (isException && exception == TimeSpec::No || !isException && exception == TimeSpec::Yes)
        {
            logger_m.debugStream() << "Calling findNext recursively! (" << nextExecTime << ")" << endlog;
            return findNext(nextExecTime, next);
        }
    }
    // now that we selected a day, make time adjustments for that day if needed (e.g. for sunrise or sunset)
    if (next->adjustTime(timeinfo))
        nextExecTime = mktime(timeinfo);
    return nextExecTime;
}

void PeriodicTask::statusXml(ticpp::Element* pStatus)
{
    struct tm timeinfo;
    std::stringstream execTime;
    memcpy(&timeinfo, localtime(&nextExecTime_m), sizeof(struct tm));
    execTime << timeinfo.tm_year + 1900 << "-"
    << timeinfo.tm_mon + 1 << "-"
    << timeinfo.tm_mday << " "
    << timeinfo.tm_hour << ":"
    << timeinfo.tm_min << ":"
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
        << timeinfo.tm_hour << ":"
        << timeinfo.tm_min << ":"
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
    << timeinfo.tm_hour << ":"
    << timeinfo.tm_min << ":"
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
