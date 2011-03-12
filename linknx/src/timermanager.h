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

#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <list>
#include <string>
#include <map>
#include "config.h"
#include "logger.h"
#include "threads.h"
#include "ticpp.h"
#include "objectcontroller.h"

class TimerTask
{
public:
    virtual ~TimerTask() {};
    virtual void onTimer(time_t time) = 0;
    virtual void reschedule(time_t from = 0) = 0;
    virtual time_t getExecTime() = 0;
    virtual void statusXml(ticpp::Element* pStatus) = 0;
};

class TimeSpec
{
public:
    enum ExceptionDays
    {
        No,
        Yes,
        DontCare
    };

    enum WeekDays
    {
        Mon = 0x01,
        Tue = 0x02,
        Wed = 0x04,
        Thu = 0x08,
        Fri = 0x10,
        Sat = 0x20,
        Sun = 0x40,
        All = 0x00
    };

    TimeSpec();
    TimeSpec(int min, int hour, int mday, int mon, int year);
    TimeSpec(int min, int hour, int wdays=All, ExceptionDays exception=DontCare);
    virtual ~TimeSpec() {};

    static TimeSpec* create(ticpp::Element* pConfig, ChangeListener* cl);
    static TimeSpec* create(const std::string& type, ChangeListener* cl);

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    virtual void getData(int *min, int *hour, int *mday, int *mon, int *year, int *wdays, ExceptionDays *exception, const struct tm * timeinfo);
    virtual bool adjustTime(struct tm * timeinfo) { return false; };
protected:
    //		int sec_m;
    int min_m;
    int hour_m;
    int mday_m;
    int mon_m;
    int year_m;
    int wdays_m;
    ExceptionDays exception_m;

};

class VariableTimeSpec : public TimeSpec
{
public:
    VariableTimeSpec(ChangeListener* cl);
    virtual ~VariableTimeSpec();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    virtual void getData(int *min, int *hour, int *mday, int *mon, int *year, int *wdays, ExceptionDays *exception, const struct tm * timeinfo);
protected:
    TimeObject* time_m;
    DateObject* date_m;
    ChangeListener* cl_m;
};

class PeriodicTask : public TimerTask, public ChangeListener
{
public:
    PeriodicTask(ChangeListener* cl);
    virtual ~PeriodicTask();

    virtual void onTimer(time_t time);
    virtual void reschedule(time_t from);
    virtual time_t getExecTime() { return nextExecTime_m; };
    virtual void statusXml(ticpp::Element* pStatus);

    void setAt(TimeSpec* at) { at_m = at; };
    void setUntil(TimeSpec* until) { until_m = until; };
    void setDuring(int during) { during_m = during; };
    virtual void onChange(Object* object);

protected:
    TimeSpec *at_m, *until_m;
    int during_m, after_m;
    time_t nextExecTime_m;
    ChangeListener* cl_m;
    bool value_m;

    time_t findNext(time_t start, TimeSpec* next);
    static Logger& logger_m;
};

class FixedTimeTask : public TimerTask
{
public:
    FixedTimeTask();
    virtual ~FixedTimeTask();

    virtual void onTimer(time_t time) = 0;
    virtual void reschedule(time_t from);
    virtual time_t getExecTime() { return execTime_m; };
    virtual void statusXml(ticpp::Element* pStatus);

protected:
    time_t execTime_m;
    static Logger& logger_m;
};

class TimerManager : protected Thread
{
public:
    enum TimerCheck
    {
        Immediate,
        Short,
        Long
    };

    TimerManager();
    virtual ~TimerManager();

    TimerCheck checkTaskList(time_t now);

    void addTask(TimerTask* task);
    void removeTask(TimerTask* task);

    void startManager() { Start(); };
    void stopManager() { Stop(); };

    virtual void statusXml(ticpp::Element* pStatus);

private:
    void Run (pth_sem_t * stop);

    typedef std::list<TimerTask*> TaskList_t;
    TaskList_t taskList_m;
    static Logger& logger_m;
};

class DaySpec
{
public:
    DaySpec() : mday_m(-1), mon_m(-1), year_m(-1) {};

    void importXml(ticpp::Element* pConfig);
    void exportXml(ticpp::Element* pConfig);

    int mday_m;
    int mon_m;
    int year_m;
};

class ExceptionDays
{
public:
    ExceptionDays();
    virtual ~ExceptionDays();

    void clear();
    void addDay(DaySpec* date);
    void removeDay(DaySpec* date);

    void importXml(ticpp::Element* pConfig);
    void exportXml(ticpp::Element* pConfig);

    bool isException(time_t time);

private:
    typedef std::list<DaySpec*> DaysList_t;
    DaysList_t daysList_m;
    static ExceptionDays* instance_m;
};

#endif
