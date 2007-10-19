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
#include "services.h"
#include <iostream>
#include <ctime>

TimerManager::TimerManager()
{}

TimerManager::~TimerManager()
{
    StopDelete ();
}

void TimerManager::Run (pth_sem_t * stop1)
{
    pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
    std::cout << "Starting TimerManager loop." << std::endl;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
        if (taskList_m.empty())
        {
            tv.tv_sec = 10;
        }
        else
        {
            TimerTask* first = taskList_m.front();
            time_t now = time(0);
            time_t nextExec = first->getExecTime();
            if (nextExec <= now)
            {
                if (nextExec > now-60)
                {
                    std::cout << "TimerTask execution. " << nextExec << std::endl;
                    first->onTimer(now);
                }
                else
                    std::cout << "TimerTask skipped due to clock skew or heavy load. " << nextExec << std::endl;
                taskList_m.pop_front();
                first->reschedule(now);
            }
            else
                tv.tv_sec = 1;
        }
        pth_select_ev(0,0,0,0,&tv,stop);
    }
    std::cout << "Out of TimerManager loop." << std::endl;
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

TimeSpec* TimeSpec::create(const std::string& type, ChangeListener* cl)
{
    if (type == "variable")
        return new VariableTimeSpec(cl);
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

    std::cout << "TimeSpec "
    << year_m+1900 << "-"
    << mon_m+1 << "-"
    << mday_m << " "
    << hour_m << ":"
    << min_m << ":0 (wdays="
    << wdays_m << "; exception=" << exception_m << ")" << std::endl;
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

void TimeSpec::getData(int *min, int *hour, int *mday, int *mon, int *year, int *wdays, ExceptionDays *exception)
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

void VariableTimeSpec::getData(int *min, int *hour, int *mday, int *mon, int *year, int *wdays, ExceptionDays *exception)
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

PeriodicTask::PeriodicTask(ChangeListener* cl)
        : cl_m(cl), nextExecTime_m(0), value_m(false), during_m(0), after_m(-1), at_m(0), until_m(0)
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

void PeriodicTask::reschedule(time_t now)
{
    if (now == 0)
        now = time(0);
    if (value_m)
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
        struct tm * timeinfo = localtime(&nextExecTime_m);
        std::cout << "PeriodicTask: rescheduled at "
        << timeinfo->tm_year + 1900 << "-"
        << timeinfo->tm_mon + 1 << "-"
        << timeinfo->tm_mday << " "
        << timeinfo->tm_hour << ":"
        << timeinfo->tm_min << ":"
        << timeinfo->tm_sec << " ("
        << nextExecTime_m << ")" << std::endl;
        Services::instance()->getTimerManager()->addTask(this);
    }
    else
        std::cout << "PeriodicTask: not rescheduled" << std::endl;

}

time_t PeriodicTask::findNext(time_t start, TimeSpec* next)
{
    struct tm * timeinfo;
    if (!next)
    {
        std::cout << "PeriodicTask: no more schedule available" << std::endl;
        return 0;
    }
    timeinfo = localtime(&start);
    timeinfo->tm_min++;
    
    int min, hour, mday, mon, year, wdays;
    TimeSpec::ExceptionDays exception;
    next->getData(&min, &hour, &mday, &mon, &year, &wdays, &exception);
    
    if (min != -1)
    {
        if  (timeinfo->tm_min > min)
            timeinfo->tm_hour++;
        timeinfo->tm_min = min;
    }
    if (hour != -1)
    {
        if (timeinfo->tm_hour > hour)
        {
            timeinfo->tm_mday++;
            timeinfo->tm_wday++;
        }
        timeinfo->tm_hour = hour;
    }
    if (wdays == 0)
    {
        if (mday != -1)
        {
            if (timeinfo->tm_mday > mday)
                timeinfo->tm_mon++;
            timeinfo->tm_mday = mday;
        }
        if (mon != -1)
        {
            if (timeinfo->tm_mon > mon)
                timeinfo->tm_year++;
            timeinfo->tm_mon = mon;
        }
        if (year != -1)
        {
            if (timeinfo->tm_year > year)
            {
                std::cout << "PeriodicTask: no more schedule available" << std::endl;
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
                std::cout << "PeriodicTask: wrong weekday specification" << std::endl;
                return 0;
            }
            wd = (wd+1) % 7;
        }
    }

    timeinfo->tm_sec = 0;
    time_t nextExecTime = mktime(timeinfo);
    if (nextExecTime < 0)
    {
        std::cout << "PeriodicTask: no more schedule available" << std::endl;
        return 0;
    }
    if (exception != TimeSpec::DontCare)
    {
        bool isException = Services::instance()->getExceptionDays()->isException(nextExecTime);
        if (isException && exception == TimeSpec::No || !isException && exception == TimeSpec::Yes)
        {
            std::cout << "PeriodicTask: calling findNext recursively! (" << nextExecTime << ")" << std::endl;
            return findNext(nextExecTime, next);
        }
    }
    return nextExecTime;
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

    std::cout << "DaySpec "
    << year_m+1900 << "-"
    << mon_m+1 << "-"
    << mday_m << std::endl;
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
    std::cout << "ExceptionDays: DELETE" << std::endl;
    DaysList_t::iterator it;
    for (it = daysList_m.begin(); it != daysList_m.end(); it++)
        delete (*it);
}

void ExceptionDays::importXml(ticpp::Element* pConfig)
{
    ticpp::Iterator< ticpp::Element > child;
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
    struct tm * timeinfo = localtime(&time);

    DaysList_t::iterator it;
    for (it = daysList_m.begin(); it != daysList_m.end(); it++)
    {
        if (((*it)->year_m == -1 || (*it)->year_m == timeinfo->tm_year) &&
                ((*it)->mon_m == -1 || (*it)->mon_m == timeinfo->tm_mon) &&
                ((*it)->mday_m == -1 || (*it)->mday_m == timeinfo->tm_mday))
        {
            std::cout << "ExceptionDays: "
            << timeinfo->tm_year+1900 << "-"
            << timeinfo->tm_mon+1 << "-"
            << timeinfo->tm_mday << " is an exception day!" << std::endl;
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
