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
    virtual void statusXml(ticpp::Element* pStatus) = 0;

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
    virtual void statusXml(ticpp::Element* pStatus);

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
    virtual void statusXml(ticpp::Element* pStatus);

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
    virtual void statusXml(ticpp::Element* pStatus);

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
    virtual void statusXml(ticpp::Element* pStatus);

protected:
    Object* object_m;
    ChangeListener* cl_m;
    bool trigger_m;
    int op_m;
    enum Operation
    {
        eq = 0x01,
        gt = 0x02,
        lt = 0x04
    };
private:
    ObjectValue* value_m;
};

class ObjectComparisonCondition : public ObjectCondition
{
public:
    ObjectComparisonCondition(ChangeListener* cl);
    virtual ~ObjectComparisonCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    virtual void statusXml(ticpp::Element* pStatus);

protected:
    Object* object2_m;
};

class ObjectSourceCondition : public ObjectCondition
{
public:
    ObjectSourceCondition(ChangeListener* cl);
    virtual ~ObjectSourceCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    virtual void statusXml(ticpp::Element* pStatus);

private:
    eibaddr_t src_m;
};

class ObjectThresholdCondition : public ObjectCondition
{
public:
    ObjectThresholdCondition(ChangeListener* cl);
    virtual ~ObjectThresholdCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    virtual void statusXml(ticpp::Element* pStatus);

protected:
    double refValue_m;
    double deltaUp_m, deltaLow_m;
    Condition* condition_m;
};

class TimerCondition : public Condition, public PeriodicTask
{
public:
    TimerCondition(ChangeListener* cl);
    virtual ~TimerCondition();

    virtual bool evaluate();
    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    virtual void statusXml(ticpp::Element* pStatus);
private:
    bool trigger_m;
    char initVal_m;
    enum InitialValue
    {
        initValTrue = 1,
        initValFalse = 0,
        initValGuess = -1
    };
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
    virtual void statusXml(ticpp::Element* pStatus);

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
    virtual void cancel() { Stop(); };
    virtual bool isFinished() { return Thread::isFinished(); };
private:
    virtual void Run (pth_sem_t * stop) = 0;
protected:
    static bool sleep(int delay, pth_sem_t * stop);
    static bool usleep(int delay, pth_sem_t * stop);
    bool parseVarString(std::string &str, bool checkOnly = false);
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

    UIntObject* object_m;
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

class ToggleValueAction : public Action
{
public:
    ToggleValueAction();
    virtual ~ToggleValueAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    SwitchingObject* object_m;
};

class FormulaAction : public Action
{
public:
    FormulaAction();
    virtual ~FormulaAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    Object *object_m, *x_m, *y_m;
    float a_m, b_m, c_m, m_m, n_m;
};

class SetStringAction : public Action
{
public:
    SetStringAction();
    virtual ~SetStringAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    Object* object_m;
    std::string value_m;
};

class SendReadRequestAction : public Action
{
public:
    SendReadRequestAction();
    virtual ~SendReadRequestAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    Object* object_m;
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

class RepeatListAction : public Action
{
public:
    RepeatListAction();
    virtual ~RepeatListAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    int period_m, count_m;
    typedef std::list<Action*> ActionsList_t;
    ActionsList_t actionsList_m;
};

class ConditionalAction : public Action
{
public:
    ConditionalAction();
    virtual ~ConditionalAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    Condition *condition_m;
    typedef std::list<Action*> ActionsList_t;
    ActionsList_t actionsList_m;
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

    int varFlags_m;
    enum replaceVarFlags
    {
        VarEnabled = 1,
        VarId = 2,
        VarValue = 4,
    };
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

    int varFlags_m;
    enum replaceVarFlags
    {
        VarEnabled = 1,
        VarTo = 2,
        VarSubject = 4,
        VarText = 8,
    };
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

    int varFlags_m;
    enum replaceVarFlags
    {
        VarEnabled = 1,
        VarCmd = 2,
    };
    std::string cmd_m;
};

class StartActionlistAction : public Action
{
public:
    StartActionlistAction();
    virtual ~StartActionlistAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    std::string ruleId_m;
    bool list_m;
};

class CancelAction : public Action
{
public:
    CancelAction();
    virtual ~CancelAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    std::string ruleId_m;
};

class SetRuleActiveAction : public Action
{
public:
    SetRuleActiveAction();
    virtual ~SetRuleActiveAction();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

private:
    virtual void Run (pth_sem_t * stop);

    std::string ruleId_m;
    bool active_m;
};

class Rule : public ChangeListener
{
public:
    Rule();
    virtual ~Rule();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void updateXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);
    virtual void statusXml(ticpp::Element* pStatus);

    virtual const char* getID() { return id_m.c_str(); };
    virtual void onChange(Object* object);

    void evaluate();
    void executeActionsTrue();
    void executeActionsFalse();
    void setActive(bool active);
    void cancel();

private:
    std::string id_m;
    std::string descr_m;
    Condition* condition_m;
    typedef std::list<Action*> ActionsList_t;
    ActionsList_t actionsList_m;
    ActionsList_t actionsListFalse_m;
    bool prevValue_m;
//    bool isActive_m;
    enum Flags
    {
        None = 0x00,
        Active = 0x01,
        StatelessIfTrue = 0x02,
        StatelessIfFalse = 0x04,
        Stateless = StatelessIfFalse | StatelessIfTrue
    };
    int flags_m;
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
    virtual void statusXml(ticpp::Element* pStatus);
    
    Rule *getRule(const char *id);

    static int parseDuration(const std::string& duration, bool allowNegative = false, bool useMilliseconds = false);
    static std::string formatDuration(int duration, bool useMilliseconds = false);

private:
    RuleServer();
    ~RuleServer();
    typedef std::pair<std::string ,Rule*> RuleIdPair_t;
    typedef std::map<std::string ,Rule*> RuleIdMap_t;
    RuleIdMap_t rulesMap_m;
    static RuleServer* instance_m;
};

#endif
