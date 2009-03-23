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

#ifndef RULESERVER_H
#define RULESERVER_H

#include <list>
#include <string>
#include "config.h"
#include "logger.h"
#include "objectcontroller.h"
#include "timermanager.h"
#include "ticpp.h"

class Condition
{
public:
    virtual ~Condition() {};

    static Condition* create(const std::string& type, ChangeListener* cl);
    static Condition* create(ticpp::Element* pConfig, ChangeListener* cl);

    virtual bool evaluate() = 0;
    virtual void importXml(ticpp::Element* pConfig) = 0;
    virtual void exportXml(ticpp::Element* pConfig) = 0;

    typedef std::list<Condition*> ConditionsList_t;
protected:
    static Logger& logger_m;
};

class AndCondition : public Condition
{
public:
    AndCondition(ChangeListener* cl);
    virtual ~AndCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    ConditionsList_t conditionsList_m;
    ChangeListener* cl_m;
};

class OrCondition : public Condition
{
public:
    OrCondition(ChangeListener* cl);
    virtual ~OrCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    ConditionsList_t conditionsList_m;
    ChangeListener* cl_m;
};

class NotCondition : public Condition
{
public:
    NotCondition(ChangeListener* cl);
    virtual ~NotCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    Condition* condition_m;
    ChangeListener* cl_m;
};

class ObjectCondition : public Condition
{
public:
    ObjectCondition(ChangeListener* cl);
    virtual ~ObjectCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

protected:
    Object* object_m;
private:
    ObjectValue* value_m;
    ChangeListener* cl_m;
    bool trigger_m;
    int op_m;
    enum Operation
    {
        eq = 0x01,
        gt = 0x02,
        lt = 0x04
    };
};

class ObjectSourceCondition : public ObjectCondition
{
public:
    ObjectSourceCondition(ChangeListener* cl);
    virtual ~ObjectSourceCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    eibaddr_t src_m;
};

class TimerCondition : public Condition, public PeriodicTask
{
public:
    TimerCondition(ChangeListener* cl);
    virtual ~TimerCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
private:
    bool trigger_m;
};

class TimeCounterCondition : public Condition, public FixedTimeTask
{
public:
    TimeCounterCondition(ChangeListener* cl);
    virtual ~TimeCounterCondition();

    virtual void onTimer(time_t time);
    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    Condition* condition_m;
    ChangeListener* cl_m;
    time_t lastTime_m;
    bool lastVal_m;
    int counter_m;
    int threshold_m;
    int resetDelay_m;
};

class Action : protected Thread
{
public:
    Action() : delay_m(0) {};
    virtual ~Action() {};

    static Action* create(ticpp::Element* pConfig);
    static Action* create(const std::string& type);

    virtual void importXml(ticpp::Element* pConfig) = 0;
    virtual void exportXml(ticpp::Element* pConfig);

    virtual void execute() { Start(true); };
private:
    virtual void Run (pth_sem_t * stop) = 0;
protected:
    int delay_m;
    static Logger& logger_m;
};

class DimUpAction : public Action
{
public:
    DimUpAction();
    virtual ~DimUpAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    U8Object* object_m;
    unsigned int start_m, stop_m, duration_m;
};

class SetValueAction : public Action
{
public:
    SetValueAction();
    virtual ~SetValueAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    Object* object_m;
    ObjectValue* value_m;
};

class CopyValueAction : public Action
{
public:
    CopyValueAction();
    virtual ~CopyValueAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    Object* from_m;
    Object* to_m;
};

class CycleOnOffAction : public Action, public ChangeListener
{
public:
    CycleOnOffAction();
    virtual ~CycleOnOffAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    virtual void onChange(Object* object);

private:
    virtual void Run (pth_sem_t * stop);

    SwitchingObject* object_m;
    int delayOn_m, delayOff_m, count_m;
    Condition* stopCondition_m;
    bool running_m;
};

class SendSmsAction : public Action
{
public:
    SendSmsAction();
    virtual ~SendSmsAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    std::string id_m;
    std::string value_m;
};

class SendEmailAction : public Action
{
public:
    SendEmailAction();
    virtual ~SendEmailAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    std::string to_m;
    std::string subject_m;
    std::string text_m;
};

class ShellCommandAction : public Action
{
public:
    ShellCommandAction();
    virtual ~ShellCommandAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    std::string cmd_m;
};

class Rule : public ChangeListener
{
public:
    Rule();
    virtual ~Rule();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void updateXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    const char* getID() { return id_m.c_str(); };
    virtual void onChange(Object* object);

    void evaluate();

private:
    std::string id_m;
    Condition* condition_m;
    typedef std::list<Action*> ActionsList_t;
    ActionsList_t actionsList_m;
    ActionsList_t actionsListFalse_m;
    bool prevValue_m;
    bool isActive_m;
protected:
    static Logger& logger_m;
};

class RuleServer
{
public:
    static RuleServer* instance();
    static void reset()
    {
        if (instance_m)
            delete instance_m;
        instance_m = 0;
    };

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    
    static int parseDuration(const std::string& duration, bool allowNegative = false);
    static std::string formatDuration(int duration);

private:
    RuleServer();
    ~RuleServer();
    typedef std::pair<std::string ,Rule*> RuleIdPair_t;
    typedef std::map<std::string ,Rule*> RuleIdMap_t;
    RuleIdMap_t rulesMap_m;
    static RuleServer* instance_m;
};

#endif
