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

#include "ruleserver.h"
#include "services.h"
#include "smsgateway.h"


RuleServer* RuleServer::instance_m;

RuleServer::RuleServer()
{}

RuleServer::~RuleServer()
{
    RuleIdMap_t::iterator it;
    for (it = rulesMap_m.begin(); it != rulesMap_m.end(); it++)
        delete (*it).second;
}

RuleServer* RuleServer::instance()
{
    if (instance_m == 0)
        instance_m = new RuleServer();
    return instance_m;
}

void RuleServer::importXml(ticpp::Element* pConfig)
{
    ticpp::Iterator< ticpp::Element > child("rule");
    for ( child = pConfig->FirstChildElement("rule", false); child != child.end(); child++ )
    {
        std::string id = child->GetAttribute("id");
        bool del = child->GetAttribute("delete") == "true";
        RuleIdMap_t::iterator it = rulesMap_m.find(id);
        if (it == rulesMap_m.end())
        {
            if (del)
                throw ticpp::Exception("Rule not found");
            Rule* rule = new Rule();
            rule->importXml(&(*child));
            rulesMap_m.insert(RuleIdPair_t(id, rule));
        }
        else if (del)
        {
            delete it->second;
            rulesMap_m.erase(it);
        }
        else
            it->second->updateXml(&(*child));
    }
}

void RuleServer::exportXml(ticpp::Element* pConfig)
{
    RuleIdMap_t::iterator it;
    for (it = rulesMap_m.begin(); it != rulesMap_m.end(); it++)
    {
        ticpp::Element pElem("rule");
        (*it).second->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

Rule::Rule() : condition_m(0), prevValue_m(false), isActive_m(false)
{}

Rule::~Rule()
{
    if (condition_m != 0)
        delete condition_m;

    ActionsList_t::iterator it;
    for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
        delete (*it);
    for(it=actionsListFalse_m.begin(); it != actionsListFalse_m.end(); ++it)
        delete (*it);
}

void Rule::importXml(ticpp::Element* pConfig)
{
    pConfig->GetAttribute("id", &id_m, false);

    std::string value = pConfig->GetAttribute("active");
    isActive_m = !(value == "off" || value == "false" || value == "no");

    std::cout << "Rule: Configuring " << getID() << " (active=" << isActive_m << ")" << std::endl;

    ticpp::Element* pCondition = pConfig->FirstChildElement("condition");
    condition_m = Condition::create(pCondition, this);

    ticpp::Iterator<ticpp::Element> actionListIt("actionlist");
    for ( actionListIt = pConfig->FirstChildElement("actionlist"); actionListIt != actionListIt.end(); actionListIt++ )
    {
        bool isFalse = (*actionListIt).GetAttribute("type") == "on-false";
        std::cout << "ActionList: Configuring " << (isFalse ? "'on-false'" : "" ) << std::endl;
        ticpp::Iterator<ticpp::Element> actionIt("action");
        for (actionIt = (*actionListIt).FirstChildElement("action", false); actionIt != actionIt.end(); actionIt++ )
        {
            Action* action = Action::create(&(*actionIt));
            if (isFalse)
                actionsListFalse_m.push_back(action);
            else
                actionsList_m.push_back(action);
        }
    }
    std::cout << "Rule: Configuration done" << std::endl;
}

void Rule::updateXml(ticpp::Element* pConfig)
{
    std::string value = pConfig->GetAttribute("active");
    if (value != "")
        isActive_m = !(value == "off" || value == "false" || value == "no");

    std::cout << "Rule: Reconfiguring " << getID() << " (active=" << isActive_m << ")" << std::endl;

    ticpp::Element* pCondition = pConfig->FirstChildElement("condition", false);
    if (pCondition != NULL)
    {
        if (condition_m != 0)
            delete condition_m;
        std::cout << "Rule: Reconfiguring condition " << getID() << std::endl;
        condition_m = Condition::create(pCondition, this);
    }

    ticpp::Element* pActionList = pConfig->FirstChildElement("actionlist", false);

    if (pActionList != NULL)
    {
        ActionsList_t::iterator it;
        for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
            delete (*it);
        actionsList_m.clear();
        for(it=actionsListFalse_m.begin(); it != actionsListFalse_m.end(); ++it)
            delete (*it);
        actionsListFalse_m.clear();

        ticpp::Iterator<ticpp::Element> actionListIt("actionlist");
        for ( actionListIt = pActionList; actionListIt != actionListIt.end(); actionListIt++ )
        {
            bool isFalse = (*actionListIt).GetAttribute("type") == "on-false";
            std::cout << "ActionList: Reconfiguring " << (isFalse ? "'on-false'" : "" ) << std::endl;
            ticpp::Iterator<ticpp::Element> actionIt("action");
            for (actionIt = (*actionListIt).FirstChildElement("action"); actionIt != actionIt.end(); actionIt++ )
            {
                Action* action = Action::create(&(*actionIt));
                if (isFalse)
                    actionsListFalse_m.push_back(action);
                else
                    actionsList_m.push_back(action);
            }
        }
    }
    std::cout << "Rule: Reconfiguration done" << std::endl;
}

void Rule::exportXml(ticpp::Element* pConfig)
{
    if (id_m != "")
        pConfig->SetAttribute("id", id_m);
    if (isActive_m == false)
        pConfig->SetAttribute("active", "no");
    if (condition_m)
    {
        ticpp::Element pCond("condition");
        condition_m->exportXml(&pCond);
        pConfig->LinkEndChild(&pCond);
    }

    ActionsList_t::iterator it;
    if (actionsList_m.begin() != actionsList_m.end())
    {
        ticpp::Element pList("actionlist");
        pConfig->LinkEndChild(&pList);

        for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
        {
            ticpp::Element pElem("action");
            (*it)->exportXml(&pElem);
            pList.LinkEndChild(&pElem);
        }
    }
    if (actionsListFalse_m.begin() != actionsListFalse_m.end())
    {
        ticpp::Element pList("actionlist");
        pList.SetAttribute("type", "on-false");
        pConfig->LinkEndChild(&pList);

        for(it=actionsListFalse_m.begin(); it != actionsListFalse_m.end(); ++it)
        {
            ticpp::Element pElem("action");
            (*it)->exportXml(&pElem);
            pList.LinkEndChild(&pElem);
        }
    }
}

void Rule::onChange(Object* object)
{
    evaluate();
}

void Rule::evaluate()
{
    if (isActive_m)
    {
        ActionsList_t::iterator it;
        bool curValue = condition_m->evaluate();
        if (curValue && !prevValue_m)
        {
            for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
                (*it)->execute();
        }
        else if (!curValue && prevValue_m)
        {
            for(it=actionsListFalse_m.begin(); it != actionsListFalse_m.end(); ++it)
                (*it)->execute();
        }
        prevValue_m = curValue;
    }
}

Action* Action::create(const std::string& type)
{
    if (type == "dim-up")
        return new DimUpAction();
    else if (type == "set-value")
        return new SetValueAction();
    else if (type == "cycle-on-off")
        return new CycleOnOffAction();
    else if (type == "send-sms")
        return new SendSmsAction();
    else if (type == "send-email")
        return new SendEmailAction();
    else if (type == "shell-cmd")
        return new ShellCommandAction();
    else
        return 0;
}

Action* Action::create(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttribute("type");
    int delay;
    pConfig->GetAttributeOrDefault("delay", &delay, 0);
    Action* action = Action::create(type);
    if (action == 0)
    {
        std::stringstream msg;
        msg << "Action type not supported: '" << type << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    action->delay_m = delay;
    action->importXml(pConfig);
    return action;
}

void Action::exportXml(ticpp::Element* pConfig)
{
    if (delay_m != 0)
        pConfig->SetAttribute("delay", delay_m);
}

DimUpAction::DimUpAction() : start_m(0), stop_m(255), duration_m(60), object_m(0)
{}

DimUpAction::~DimUpAction()
{}

void DimUpAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    Object* obj = ObjectController::instance()->getObject(id); 
    object_m = dynamic_cast<ScalingObject*>(obj); 
    if (!object_m)
    {
        std::stringstream msg;
        msg << "Wrong Object type for DimUpAction: '" << id << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    pConfig->GetAttribute("start", &start_m);
    pConfig->GetAttribute("stop", &stop_m);
    pConfig->GetAttribute("duration", &duration_m);
    std::cout << "DimUpAction: Configured for object " << object_m->getID()
    << " with start=" << start_m
    << "; stop=" << stop_m
    << "; duration=" << duration_m << std::endl;
}

void DimUpAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "dim-up");
    pConfig->SetAttribute("id", object_m->getID());
    pConfig->SetAttribute("start", start_m);
    pConfig->SetAttribute("stop", stop_m);
    pConfig->SetAttribute("duration", duration_m);

    Action::exportXml(pConfig);
}

void DimUpAction::Run (pth_sem_t * stop)
{
    pth_sleep(delay_m);
    if (stop_m > start_m)
    {
        std::cout << "Execute DimUpAction" << std::endl;

        unsigned long step = ((duration_m * 1000) / (stop_m - start_m)) * 1000;
        for (int idx=start_m; idx < stop_m; idx++)
        {
            object_m->setIntValue(idx);
            pth_usleep(step);
            if (object_m->getIntValue() < idx)
            {
                std::cout << "Abort DimUpAction" << std::endl;
                return;
            }
        }
    }
    else
    {
        std::cout << "Execute DimUpAction (decrease)" << std::endl;

        unsigned long step = ((duration_m * 1000) / (start_m - stop_m)) * 1000;
        for (int idx=start_m; idx > stop_m; idx--)
        {
            object_m->setIntValue(idx);
            pth_usleep(step);
            if (object_m->getIntValue() > idx)
            {
                std::cout << "Abort DimUpAction" << std::endl;
                return;
            }
        }
    }
}

SetValueAction::SetValueAction() : value_m(0), object_m(0)
{}

SetValueAction::~SetValueAction()
{
    if (value_m)
        delete value_m;
}

void SetValueAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    object_m = ObjectController::instance()->getObject(id);

    std::string value;
    value = pConfig->GetAttribute("value");

    value_m = object_m->createObjectValue(value);
    std::cout << "SetValueAction: Configured for object " << object_m->getID() << " with value " << value_m->toString() << std::endl;
}

void SetValueAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "set-value");
    pConfig->SetAttribute("id", object_m->getID());
    pConfig->SetAttribute("value", value_m->toString());

    Action::exportXml(pConfig);
}

void SetValueAction::Run (pth_sem_t * stop)
{
    pth_sleep(delay_m);
    std::cout << "Execute SetValueAction with value " << value_m->toString() << std::endl;
    if (object_m)
        object_m->setValue(value_m);
}

CycleOnOffAction::CycleOnOffAction() : object_m(0), count_m(0), delayOn_m(0), delayOff_m(0)
{}

CycleOnOffAction::~CycleOnOffAction()
{}

void CycleOnOffAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    Object* obj = ObjectController::instance()->getObject(id); 
    object_m = dynamic_cast<SwitchingObject*>(obj); 
    if (!object_m)
    {
        std::stringstream msg;
        msg << "Wrong Object type for CycleOnOffAction: '" << id << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }

    pConfig->GetAttribute("on", &delayOn_m);
    pConfig->GetAttribute("off", &delayOff_m);
    pConfig->GetAttribute("count", &count_m);
    std::cout << "CycleOnOffAction: Configured for object " << object_m->getID()
    << " with delay_on=" << delayOn_m
    << "; delay_off=" << delayOff_m
    << "; count=" << count_m << std::endl;
}

void CycleOnOffAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "cycle-on-off");
    pConfig->SetAttribute("id", object_m->getID());
    pConfig->SetAttribute("on", delayOn_m);
    pConfig->SetAttribute("off", delayOff_m);
    pConfig->SetAttribute("count", count_m);

    Action::exportXml(pConfig);
}

void CycleOnOffAction::Run (pth_sem_t * stop)
{
    if (!object_m)
        return;
    pth_sleep(delay_m);
    std::cout << "Execute CycleOnOffAction" << std::endl;
    for (int i=0; i<count_m; i++)
    {
        object_m->setBoolValue(true);
        pth_sleep(delayOn_m);
        object_m->setBoolValue(false);
        pth_sleep(delayOff_m);
    }
}

SendSmsAction::SendSmsAction()
{}

SendSmsAction::~SendSmsAction()
{}

void SendSmsAction::importXml(ticpp::Element* pConfig)
{
    id_m = pConfig->GetAttribute("id");
    value_m = pConfig->GetAttribute("value");

    std::cout << "SendSmsAction: Configured for id " << id_m << " with value " << value_m << std::endl;
}

void SendSmsAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "send-sms");
    pConfig->SetAttribute("id", id_m);
    pConfig->SetAttribute("value", value_m);

    Action::exportXml(pConfig);
}

void SendSmsAction::Run (pth_sem_t * stop)
{
    pth_sleep(delay_m);
    std::cout << "Execute SendSmsAction with value " << value_m << std::endl;

    Services::instance()->getSmsGateway()->sendSms(id_m, value_m);
}

SendEmailAction::SendEmailAction()
{}

SendEmailAction::~SendEmailAction()
{}

void SendEmailAction::importXml(ticpp::Element* pConfig)
{
    to_m = pConfig->GetAttribute("to");
    subject_m = pConfig->GetAttribute("subject");
    text_m = pConfig->GetText();

    std::cout << "SendEmailAction: Configured to=" << to_m << " subject=" << subject_m << std::endl;
}

void SendEmailAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "send-email");
    pConfig->SetAttribute("to", to_m);
    pConfig->SetAttribute("subject", subject_m);
    pConfig->SetAttribute("text", text_m);

    Action::exportXml(pConfig);
}

void SendEmailAction::Run (pth_sem_t * stop)
{
    pth_sleep(delay_m);
    std::cout << "Execute SendEmailAction: to=" << to_m << " subject=" << subject_m << std::endl;

    Services::instance()->getEmailGateway()->sendEmail(to_m, subject_m, text_m);
}

ShellCommandAction::ShellCommandAction()
{}

ShellCommandAction::~ShellCommandAction()
{}

void ShellCommandAction::importXml(ticpp::Element* pConfig)
{
    cmd_m = pConfig->GetAttribute("cmd");

    std::cout << "ShellCommandAction: Configured" << std::endl;
}

void ShellCommandAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "shell-cmd");
    pConfig->SetAttribute("cmd", cmd_m);

    Action::exportXml(pConfig);
}

void ShellCommandAction::Run (pth_sem_t * stop)
{
    pth_sleep(delay_m);
    std::cout << "Execute ShellCommandAction: " << cmd_m << std::endl;

    int ret = pth_system(cmd_m.c_str());
    if (ret != 0)
        std::cout << "Execute ShellCommandAction: returned " << ret << std::endl;
}


Condition* Condition::create(const std::string& type, ChangeListener* cl)
{
    if (type == "and")
        return new AndCondition(cl);
    else if (type == "or")
        return new OrCondition(cl);
    else if (type == "not")
        return new NotCondition(cl);
    else if (type == "object")
        return new ObjectCondition(cl);
    else if (type == "timer")
        return new TimerCondition(cl);
    else
        return 0;
}

Condition* Condition::create(ticpp::Element* pConfig, ChangeListener* cl)
{
    std::string type;
    type = pConfig->GetAttribute("type");
    Condition* condition = Condition::create(type, cl);
    if (condition == 0)
    {
        std::stringstream msg;
        msg << "Condition type not supported: '" << type << "'";
        throw ticpp::Exception(msg.str());
    }
    condition->importXml(pConfig);
    return condition;
}

AndCondition::AndCondition(ChangeListener* cl) : cl_m(cl)
{}

AndCondition::~AndCondition()
{
    ConditionsList_t::iterator it;
    for(it=conditionsList_m.begin(); it != conditionsList_m.end(); ++it)
        delete (*it);
}

bool AndCondition::evaluate()
{
    ConditionsList_t::iterator it;
    for(it=conditionsList_m.begin(); it != conditionsList_m.end(); ++it)
        if (!(*it)->evaluate())
            return false;
    return true;
}

void AndCondition::importXml(ticpp::Element* pConfig)
{
    ticpp::Iterator< ticpp::Element > child("condition");
    for ( child = pConfig->FirstChildElement("condition"); child != child.end(); child++ )
    {
        Condition* condition = Condition::create(&(*child), cl_m);
        conditionsList_m.push_back(condition);
    }
}

void AndCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "and");
    ConditionsList_t::iterator it;
    for (it = conditionsList_m.begin(); it != conditionsList_m.end(); it++)
    {
        ticpp::Element pElem("condition");
        (*it)->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

OrCondition::OrCondition(ChangeListener* cl) : cl_m(cl)
{}

OrCondition::~OrCondition()
{
    ConditionsList_t::iterator it;
    for(it=conditionsList_m.begin(); it != conditionsList_m.end(); ++it)
        delete (*it);
}

bool OrCondition::evaluate()
{
    ConditionsList_t::iterator it;
    for(it=conditionsList_m.begin(); it != conditionsList_m.end(); ++it)
        if ((*it)->evaluate())
            return true;
    return false;
}

void OrCondition::importXml(ticpp::Element* pConfig)
{
    ticpp::Iterator< ticpp::Element > child("condition");
    for ( child = pConfig->FirstChildElement("condition"); child != child.end(); child++ )
    {
        Condition* condition = Condition::create(&(*child), cl_m);
        conditionsList_m.push_back(condition);
    }
}

void OrCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "or");
    ConditionsList_t::iterator it;
    for (it = conditionsList_m.begin(); it != conditionsList_m.end(); it++)
    {
        ticpp::Element pElem("condition");
        (*it)->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

NotCondition::NotCondition(ChangeListener* cl) : cl_m(cl), condition_m(0)
{}

NotCondition::~NotCondition()
{
    if (condition_m)
        delete condition_m;
}

bool NotCondition::evaluate()
{
    return !condition_m->evaluate();
}

void NotCondition::importXml(ticpp::Element* pConfig)
{
    condition_m = Condition::create(pConfig->FirstChildElement("condition"), cl_m);
}

void NotCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "not");
    if (condition_m)
    {
        ticpp::Element pElem("condition");
        condition_m->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

ObjectCondition::ObjectCondition(ChangeListener* cl) : value_m(0), cl_m(cl), trigger_m(false)
{}

ObjectCondition::~ObjectCondition()
{
    if (value_m)
        delete value_m;
}

bool ObjectCondition::evaluate()
{
    bool val = object_m->equals(value_m);
    std::cout << "ObjectCondition (id='" << object_m->getID()
    << "') evaluated as '" << val
    << "'" << std::endl;
    return val;
}

void ObjectCondition::importXml(ticpp::Element* pConfig)
{
    std::string trigger;
    trigger = pConfig->GetAttribute("trigger");
    std::string id;
    id = pConfig->GetAttribute("id");
    object_m = ObjectController::instance()->getObject(id);

    if (trigger == "true")
    {
        trigger_m = true;
        object_m->addChangeListener(cl_m);
    }

    std::string value;
    value = pConfig->GetAttribute("value");

    value_m = object_m->createObjectValue(value);
    std::cout << "ObjectCondition: configured value_m='" << value_m->toString() << "'" << std::endl;
}

void ObjectCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "object");
    pConfig->SetAttribute("id", object_m->getID());
    pConfig->SetAttribute("value", value_m->toString());
    if (trigger_m)
        pConfig->SetAttribute("trigger", "true");
}


TimerCondition::TimerCondition(ChangeListener* cl)
        : PeriodicTask(cl), trigger_m(false)
{}

TimerCondition::~TimerCondition()
{}

bool TimerCondition::evaluate()
{
    std::cout << "TimerCondition evaluated as '" << value_m << "'" << std::endl;
    return value_m;
}

void TimerCondition::importXml(ticpp::Element* pConfig)
{
    std::string trigger;
    trigger = pConfig->GetAttribute("trigger");

    if (trigger == "true")
        trigger_m = true;
    else
        cl_m = 0;

    ticpp::Element* at = pConfig->FirstChildElement("at", false);
    ticpp::Element* every = pConfig->FirstChildElement("every", false);
    if (at && every)
        throw ticpp::Exception("Timer can't define <at> and <every> elements simultaneously");
    if (at)
    {
        if (at_m)
            delete at_m;
        at_m = TimeSpec::create(at, this);
    }
    else if (every)
        every->GetText(&after_m);
    else
        throw ticpp::Exception("Timer must define <at> or <every> elements");

    ticpp::Element* during = pConfig->FirstChildElement("during", false);
    ticpp::Element* until = pConfig->FirstChildElement("until", false);
    if (during && until)
        throw ticpp::Exception("Timer can't define <until> and <during> elements simultaneously");
    if (during)
    {
        during->GetText(&during_m);
        if (every && after_m > during_m)
            after_m -= during_m;
        else if (every)
            throw ticpp::Exception("Parameter <every> must be greater than <during>");
    }
    else if (until)
    {
        if (until_m)
            delete until_m;
        until_m = TimeSpec::create(until, this);
//        until_m.importXml(until);
        during_m = -1;
    }
    else
        during_m = 0;

    reschedule(0);
}

void TimerCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "timer");
    if (trigger_m)
        pConfig->SetAttribute("trigger", "true");

    if (after_m == -1)
    {
        ticpp::Element pAt("at");
        at_m->exportXml(&pAt);
        pConfig->LinkEndChild(&pAt);
    }
    else
    {
        ticpp::Element pEvery("every");
        int every = after_m;
        if (during_m > 0)
            every += during_m;
        pEvery.SetText(every);
        pConfig->LinkEndChild(&pEvery);
    }

    if (during_m == -1)
    {
        ticpp::Element pUntil("until");
        until_m->exportXml(&pUntil);
        pConfig->LinkEndChild(&pUntil);
    }
    else if (during_m != 0)
    {
        ticpp::Element pDuring("during");
        pDuring.SetText(during_m);
        pConfig->LinkEndChild(&pDuring);
    }
}

