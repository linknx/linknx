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
#include "luacondition.h"
#include "ioport.h"
#include <cmath>

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

void RuleServer::statusXml(ticpp::Element* pStatus)
{
    RuleIdMap_t::iterator it;
    for (it = rulesMap_m.begin(); it != rulesMap_m.end(); it++)
    {
        ticpp::Element pElem("rule");
        (*it).second->statusXml(&pElem);
        pStatus->LinkEndChild(&pElem);
    }
}

Rule *RuleServer::getRule(const char *id)
{
    RuleIdMap_t::iterator it = rulesMap_m.find(id);
    if (it == rulesMap_m.end())
        return 0;
    return it->second;
}

int RuleServer::parseDuration(const std::string& duration, bool allowNegative, bool useMilliseconds)
{
    if (duration == "")
        return 0;
    std::istringstream val(duration);
    std::string unit;
    int num;
    val >> num;

    if (val.fail() || (num < 0 && !allowNegative))
    {
        std::stringstream msg;
        msg << "RuleServer::parseDuration: Bad value: '" << duration << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    val >> unit;
    if (unit == "d")
        num = num * 3600 * 24;
    else if (unit == "h")
        num = num * 3600;
    else if (unit == "m")
        num = num * 60;
    else if (unit != "" && unit != "s" && unit != "ms")
    {
        std::stringstream msg;
        msg << "RuleServer::parseDuration: Bad unit: '" << unit << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    if (unit == "ms")
    {
        if (!useMilliseconds)
        {
            std::stringstream msg;
            msg << "RuleServer::parseDuration: Milliseconds not supported" << std::endl;
            throw ticpp::Exception(msg.str());
        }
    }
    else if (useMilliseconds)
        num = num * 1000;

    return num;
}

std::string RuleServer::formatDuration(int duration, bool useMilliseconds)
{
    if (duration == 0)
        return "";
    std::stringstream output;
    if (useMilliseconds)
    {
        if (duration % (1000) != 0)
        {
            output << duration << "ms";
            return output.str();
        }
        duration = (duration / 1000);
    }
    if (duration % (3600*24) == 0)
        output << (duration / (3600*24)) << 'd';
    else if (duration % 3600 == 0)
        output << (duration / 3600) << 'h';
    else if (duration % 60 == 0)
        output << (duration / 60) << 'm';
    else
        output << duration;
    return output.str();
}

Logger& Rule::logger_m(Logger::getInstance("Rule"));

Rule::Rule() : condition_m(0), prevValue_m(false), flags_m(None)
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
    setActive(value != "off" && value != "false" && value != "no");

    pConfig->GetAttribute("description", &descr_m, false);

    logger_m.infoStream() << "Rule: Configuring " << getID() << " (active=" << ((flags_m & Active) != 0) << ")" << endlog;

    ticpp::Element* pCondition = pConfig->FirstChildElement("condition");
    condition_m = Condition::create(pCondition, this);

    ticpp::Iterator<ticpp::Element> actionListIt("actionlist");
    for ( actionListIt = pConfig->FirstChildElement("actionlist"); actionListIt != actionListIt.end(); actionListIt++ )
    {
        std::string type = (*actionListIt).GetAttribute("type");
        bool isFalse = ((type == "if-false") || (type == "on-false"));
        if (type == "if-false")
            flags_m |= StatelessIfFalse;
        else if (type == "if-true")
            flags_m |= StatelessIfTrue;
        else if (!isFalse)
            type = "on-true"; // this is just for log display below.
        
        logger_m.infoStream() << "ActionList: Configuring '" << type << "' action list" << endlog;
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
    logger_m.infoStream() << "Rule: Configuration done" << endlog;
}

void Rule::updateXml(ticpp::Element* pConfig)
{
    std::string value = pConfig->GetAttribute("active");
    if (value != "")
        setActive(value != "off" && value != "false" && value != "no");

    pConfig->GetAttribute("description", &descr_m, false);

    logger_m.infoStream() << "Rule: Reconfiguring " << getID() << " (active=" << ((flags_m & Active) != 0) << ")" << endlog;

    ticpp::Element* pCondition = pConfig->FirstChildElement("condition", false);
    if (pCondition != NULL)
    {
        if (condition_m != 0)
            delete condition_m;
        logger_m.infoStream() << "Rule: Reconfiguring condition " << getID() << endlog;
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
            std::string type = (*actionListIt).GetAttribute("type");
            bool isFalse = ((type == "if-false") || (type == "on-false"));
            if (type == "if-false")
                flags_m |= StatelessIfFalse;
            else if (type == "if-true")
                flags_m |= StatelessIfTrue;
            else if (!isFalse)
                type = "on-true"; // this is just for log display below.
            
            logger_m.infoStream() << "ActionList: Reconfiguring '" << type << "' action list" << endlog;
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
    logger_m.infoStream() << "Rule: Reconfiguration done" << endlog;
}

void Rule::exportXml(ticpp::Element* pConfig)
{
    if (id_m != "")
        pConfig->SetAttribute("id", id_m);
    if (!(flags_m & Active))
        pConfig->SetAttribute("active", "no");
    if (descr_m != "")
        pConfig->SetAttribute("description", descr_m);
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
        if (flags_m & StatelessIfTrue)
            pList.SetAttribute("type", "if-true");
        
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
        if (flags_m & StatelessIfFalse)
            pList.SetAttribute("type", "if-false");
        else
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

void Rule::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("id", id_m);
    pStatus->SetAttribute("active", (flags_m & Active ? "true" : "false"));
    pStatus->SetAttribute("state", (prevValue_m ? "true" : "false"));
    if (condition_m)
    {
        ticpp::Element pElem("condition");
        condition_m->statusXml(&pElem);
        pStatus->LinkEndChild(&pElem);
    }
}

void Rule::onChange(Object* object)
{
    evaluate();
}

void Rule::evaluate()
{
    if (flags_m & Active)
    {
        logger_m.infoStream() << "Evaluate rule " << id_m << endlog;
        bool curValue = condition_m->evaluate();
        logger_m.infoStream() << "Rule " << id_m << " evaluated as " << curValue << ", prev value was " << prevValue_m << endlog;
        if (curValue && ((flags_m & StatelessIfTrue) || !prevValue_m))
            executeActionsTrue();
        else if (!curValue && ((flags_m & StatelessIfFalse) || prevValue_m))
            executeActionsFalse();
        prevValue_m = curValue;
    }
}

void Rule::executeActionsTrue()
{
    ActionsList_t::iterator it;
    for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
        (*it)->execute();
    logger_m.debugStream() << "Action list 'true' executed for rule " << id_m << endlog;
}

void Rule::executeActionsFalse()
{
    ActionsList_t::iterator it;
    for(it=actionsListFalse_m.begin(); it != actionsListFalse_m.end(); ++it)
        (*it)->execute();
    logger_m.debugStream() << "Action list 'false' executed for rule " << id_m << endlog;
}

void Rule::setActive(bool active)
{
    if (active)
        flags_m |= Active;
    else
        flags_m &= (~Active);
}

void Rule::cancel()
{
    if (flags_m & Active)
    {
        logger_m.infoStream() << "Cancel all actions for rule " << id_m << endlog;
        ActionsList_t::iterator it;
        for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
            (*it)->cancel();
        for(it=actionsListFalse_m.begin(); it != actionsListFalse_m.end(); ++it)
            (*it)->cancel();
    }
}

Logger& Action::logger_m(Logger::getInstance("Action"));

Action* Action::create(const std::string& type)
{
    if (type == "dim-up")
        return new DimUpAction();
    else if (type == "set-value")
        return new SetValueAction();
    else if (type == "copy-value")
        return new CopyValueAction();
    else if (type == "toggle-value")
        return new ToggleValueAction();
    else if (type == "set-string")
        return new SetStringAction();
    else if (type == "send-read-request")
        return new SendReadRequestAction();
    else if (type == "cycle-on-off")
        return new CycleOnOffAction();
    else if (type == "repeat")
        return new RepeatListAction();
    else if (type == "conditional")
        return new ConditionalAction();
    else if (type == "send-sms")
        return new SendSmsAction();
    else if (type == "send-email")
        return new SendEmailAction();
    else if (type == "shell-cmd")
        return new ShellCommandAction();
    else if (type == "ioport-tx")
        return new TxAction();
#ifdef HAVE_LUA
    else if (type == "script")
        return new LuaScriptAction();
#endif
    else if (type == "start-actionlist")
        return new StartActionlistAction();
    else if (type == "cancel")
        return new CancelAction();
    else if (type == "set-rule-active")
        return new SetRuleActiveAction();
    else if (type == "formula")
        return new FormulaAction();
    else
        return 0;
}

Action* Action::create(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttribute("type");
    int delay;
    delay = RuleServer::parseDuration(pConfig->GetAttribute("delay"), false, true);
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
        pConfig->SetAttribute("delay", RuleServer::formatDuration(delay_m, true));
}

bool Action::sleep(int delay, pth_sem_t * stop)
{
    struct timeval timeout;
    timeout.tv_sec = delay / 1000;
    timeout.tv_usec = (delay % 1000) * 1000;
    pth_event_t stop_ev = pth_event (PTH_EVENT_SEM, stop);
    pth_select_ev(0, NULL, NULL, NULL, &timeout, stop_ev);
    return (pth_event_status (stop_ev) == PTH_STATUS_OCCURRED);
}

bool Action::usleep(int delay, pth_sem_t * stop)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = delay;
    pth_event_t stop_ev = pth_event (PTH_EVENT_SEM, stop);
    pth_select_ev(0, NULL, NULL, NULL, &timeout, stop_ev);
    return (pth_event_status (stop_ev) == PTH_STATUS_OCCURRED);
}

bool Action::parseVarString(std::string &str, bool checkOnly)
{
    bool modified = false;
    size_t idx = 0;
    while ((idx = str.find('$', idx)) != std::string::npos)
    {
        if (str.length() <= ++idx)
            break;
        char c = str[idx];
        if (c == '{')
        {
            size_t idx2 = str.find('}', ++idx);
            if (idx2 == std::string::npos)
                break;
            Object* obj = ObjectController::instance()->getObject(str.substr(idx, idx2-idx));
            if (!checkOnly) {
                std::string val = obj->getValue();
                logger_m.debugStream() << "Action: insert value '"<< val <<"' of object " << obj->getID() << endlog;
                str.replace(idx-2, 3+idx2-idx, val);
                idx += val.length()-2;
            }
            obj->decRefCount();
            modified = true;
        }
        else if (c == '$')
        {
            str.erase(idx, 1); // skip double $
            modified = true;
        }
    }
    return modified;
}

DimUpAction::DimUpAction() : object_m(0), start_m(0), stop_m(255), duration_m(60)
{}

DimUpAction::~DimUpAction()
{
    if (object_m)
        object_m->decRefCount();
}

void DimUpAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    Object* obj = ObjectController::instance()->getObject(id); 
    if (object_m)
        object_m->decRefCount();
    object_m = dynamic_cast<UIntObject*>(obj);
    if (!object_m)
    {
        obj->decRefCount();
        std::stringstream msg;
        msg << "Wrong Object type for DimUpAction: '" << id << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    pConfig->GetAttribute("start", &start_m);
    pConfig->GetAttribute("stop", &stop_m);
    duration_m = RuleServer::parseDuration(pConfig->GetAttribute("duration"), false, true);
    logger_m.infoStream() << "DimUpAction: Configured for object " << object_m->getID()
    << " with start=" << start_m
    << "; stop=" << stop_m
    << "; duration=" << duration_m << endlog;
}

void DimUpAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "dim-up");
    pConfig->SetAttribute("id", object_m->getID());
    pConfig->SetAttribute("start", start_m);
    pConfig->SetAttribute("stop", stop_m);
    pConfig->SetAttribute("duration", RuleServer::formatDuration(duration_m, true));

    Action::exportXml(pConfig);
}

void DimUpAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    if (stop_m > start_m)
    {
        logger_m.infoStream() << "Execute DimUpAction" << endlog;

        unsigned long step = (duration_m / (stop_m - start_m));
        for (unsigned int idx=start_m; idx <= stop_m; idx++)
        {
            object_m->setIntValue(idx);
            if (sleep(step, stop))
                return;
            if (object_m->getIntValue() < idx)
            {
                logger_m.infoStream() << "Abort DimUpAction" << endlog;
                return;
            }
        }
    }
    else
    {
        logger_m.infoStream() << "Execute DimUpAction (decrease)" << endlog;

        unsigned long step = (duration_m / (start_m - stop_m));
        for (unsigned int idx=start_m; idx >= stop_m; idx--)
        {
            object_m->setIntValue(idx);
            if (sleep(step, stop))
                return;
            if (object_m->getIntValue() > idx)
            {
                logger_m.infoStream() << "Abort DimUpAction" << endlog;
                return;
            }
        }
    }
}

SetValueAction::SetValueAction() : object_m(0), value_m(0)
{}

SetValueAction::~SetValueAction()
{
    if (object_m)
        object_m->decRefCount();
    if (value_m)
        delete value_m;
}

void SetValueAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    if (object_m)
        object_m->decRefCount();
    object_m = ObjectController::instance()->getObject(id);

    std::string value;
    value = pConfig->GetAttribute("value");

    value_m = object_m->createObjectValue(value);
    logger_m.infoStream() << "SetValueAction: Configured for object " << object_m->getID() << " with value " << value_m->toString() << endlog;
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
    if (sleep(delay_m, stop))
        return;
    if (object_m)
    {
        logger_m.infoStream() << "Execute SetValueAction: set " << object_m->getID() << " with value " << value_m->toString() << endlog;
        object_m->setValue(value_m);
    }
}

CopyValueAction::CopyValueAction() : from_m(0), to_m(0)
{}

CopyValueAction::~CopyValueAction()
{}

void CopyValueAction::importXml(ticpp::Element* pConfig)
{
    std::string obj;
    obj = pConfig->GetAttribute("to");
    if (to_m)
        to_m->decRefCount();
    to_m = ObjectController::instance()->getObject(obj);
    obj = pConfig->GetAttribute("from");
    if (from_m)
        from_m->decRefCount();
    from_m = ObjectController::instance()->getObject(obj);
    if (from_m->getType() != to_m->getType())
    {
        std::stringstream msg;
        msg << "Incompatible object types for CopyValueAction: from='" << from_m->getID() << "' to='" << to_m->getID() << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }

    logger_m.infoStream() << "CopyValueAction: Configured to copy value from " << from_m->getID() << " to " << to_m->getID() << endlog;
}

void CopyValueAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "copy-value");
    pConfig->SetAttribute("from", from_m->getID());
    pConfig->SetAttribute("to", to_m->getID());

    Action::exportXml(pConfig);
}

void CopyValueAction::Run (pth_sem_t * stop)
{
    if (from_m && to_m)
    {
        if (sleep(delay_m, stop))
            return;
        try
        {
            std::string value = from_m->getValue();
            logger_m.infoStream() << "Execute CopyValueAction set " << to_m->getID() << " with value " << value << endlog;
            to_m->setValue(value);
        }
        catch( ticpp::Exception& ex )
        {
            logger_m.warnStream() << "Error in CopyValueAction: " << ex.m_details << endlog;
        }
    }
}

ToggleValueAction::ToggleValueAction() : object_m(0)
{}

ToggleValueAction::~ToggleValueAction()
{
    if (object_m)
        object_m->decRefCount();
}

void ToggleValueAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    Object* obj = ObjectController::instance()->getObject(id);
    if (object_m)
        object_m->decRefCount();
    object_m = dynamic_cast<SwitchingObject*>(obj);
    if (!object_m)
    {
        obj->decRefCount();
        std::stringstream msg;
        msg << "Wrong Object type for ToggleValueAction: '" << id << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }

    logger_m.infoStream() << "ToggleValueAction: Configured for object " << object_m->getID() << endlog;
}

void ToggleValueAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "toggle-value");
    pConfig->SetAttribute("id", object_m->getID());

    Action::exportXml(pConfig);
}

void ToggleValueAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    if (object_m)
    {
        logger_m.infoStream() << "Execute ToggleValueAction on object " << object_m->getID() << endlog;
        object_m->setBoolValue(!object_m->getBoolValue());
    }
}

FormulaAction::FormulaAction() : object_m(0), x_m(0), y_m(0), a_m(1), b_m(1), c_m(0), m_m(1), n_m(1)
{}

FormulaAction::~FormulaAction()
{
    if (object_m)
        object_m->decRefCount();
    if (x_m)
        x_m->decRefCount();
    if (y_m)
        y_m->decRefCount();
}

void FormulaAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    if (object_m)
        object_m->decRefCount();
    object_m = ObjectController::instance()->getObject(id);

//    float a, b, c;
    if (x_m)
        x_m->decRefCount();
    id = pConfig->GetAttribute("x");
    if (id.empty())
        x_m = 0;
    else
        x_m = ObjectController::instance()->getObject(id);
    if (y_m)
        y_m->decRefCount();
    id = pConfig->GetAttribute("y");
    if (id.empty())
        y_m = 0;
    else
        y_m = ObjectController::instance()->getObject(id);
    pConfig->GetAttributeOrDefault("a", &a_m, 1.0);
    pConfig->GetAttributeOrDefault("b", &b_m, 1.0);
    pConfig->GetAttributeOrDefault("c", &c_m, 0.0);
    pConfig->GetAttributeOrDefault("m", &m_m, 1.0);
    pConfig->GetAttributeOrDefault("n", &n_m, 1.0);

    logger_m.infoStream() << "FormulaAction: Configured for object " << object_m->getID() << endlog;
}

void FormulaAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "formula");
    pConfig->SetAttribute("id", object_m->getID());
    if (x_m)
        pConfig->SetAttribute("x", x_m->getID());
    if (y_m)
        pConfig->SetAttribute("y", y_m->getID());
    if (a_m != 1.0)
        pConfig->SetAttribute("a", a_m);
    if (b_m != 1.0)
        pConfig->SetAttribute("b", b_m);
    if (c_m != 0.0)
        pConfig->SetAttribute("c", c_m);
    if (m_m != 1.0)
        pConfig->SetAttribute("m", m_m);
    if (n_m != 1.0)
        pConfig->SetAttribute("n", n_m);

    Action::exportXml(pConfig);
}

void FormulaAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    if (object_m)
    {
        logger_m.infoStream() << "Execute FormulaAction: set " << object_m->getID() << endlog;
        float res = c_m;
        if (x_m)
            res += a_m * pow(x_m->getFloatValue(), m_m);
        if (y_m)
            res += b_m * pow(y_m->getFloatValue(), n_m);
        object_m->setFloatValue(res);
    }
}

SetStringAction::SetStringAction() : object_m(0)
{}

SetStringAction::~SetStringAction()
{
    if (object_m)
        object_m->decRefCount();
}

void SetStringAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    if (object_m)
        object_m->decRefCount();
    object_m = ObjectController::instance()->getObject(id);

    std::string value =pConfig->GetAttribute("value");
    value_m = value;
    parseVarString(value, true); // Just to check string parsing and that referenced object are present

    logger_m.infoStream() << "SetStringAction: Configured for object " << object_m->getID() << " with string " << value_m << endlog;
}

void SetStringAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "set-string");
    pConfig->SetAttribute("id", object_m->getID());
    pConfig->SetAttribute("value", value_m);

    Action::exportXml(pConfig);
}

void SetStringAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    std::string value = value_m;
    parseVarString(value);
    if (object_m)
    {
        logger_m.infoStream() << "Execute SetStringAction for object " << object_m->getID() << " with value " << value << endlog;
        object_m->setValue(value);
    }
}

SendReadRequestAction::SendReadRequestAction() : object_m(0)
{}

SendReadRequestAction::~SendReadRequestAction()
{
    if (object_m)
        object_m->decRefCount();
}

void SendReadRequestAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    if (object_m)
        object_m->decRefCount();
    object_m = ObjectController::instance()->getObject(id);

    logger_m.infoStream() << "SendReadRequestAction: Configured for object " << object_m->getID() << endlog;
}

void SendReadRequestAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "send-read-request");
    pConfig->SetAttribute("id", object_m->getID());

    Action::exportXml(pConfig);
}

void SendReadRequestAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    if (object_m)
    {
        logger_m.infoStream() << "Execute SendReadRequestAction for object " << object_m->getID() << endlog;
        object_m->read();
    }
}

CycleOnOffAction::CycleOnOffAction()
    : object_m(0), delayOn_m(0), delayOff_m(0), count_m(0), stopCondition_m(0), running_m(false)
{}

CycleOnOffAction::~CycleOnOffAction()
{
    if (object_m)
        object_m->decRefCount();
    if (stopCondition_m)
        delete stopCondition_m;
}

void CycleOnOffAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    if (object_m)
        object_m->decRefCount();
    Object* obj = ObjectController::instance()->getObject(id); 
    object_m = dynamic_cast<SwitchingObject*>(obj); 
    if (!object_m)
    {
        obj->decRefCount();
        std::stringstream msg;
        msg << "Wrong Object type for CycleOnOffAction: '" << id << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }

    delayOn_m = RuleServer::parseDuration(pConfig->GetAttribute("on"), false, true);
    delayOff_m = RuleServer::parseDuration(pConfig->GetAttribute("off"), false, true);
    pConfig->GetAttribute("count", &count_m);

    ticpp::Iterator< ticpp::Element > child;
    ticpp::Element* pStopCondition = pConfig->FirstChildElement("stopcondition", false);
    if (pStopCondition)
    {
        if (stopCondition_m)
            delete stopCondition_m;
        stopCondition_m = Condition::create(pStopCondition, this);
    }

    logger_m.infoStream() << "CycleOnOffAction: Configured for object " << object_m->getID()
    << " with delay_on=" << delayOn_m
    << "; delay_off=" << delayOff_m
    << "; count=" << count_m << endlog;
}

void CycleOnOffAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "cycle-on-off");
    pConfig->SetAttribute("id", object_m->getID());
    pConfig->SetAttribute("on", RuleServer::formatDuration(delayOn_m, true));
    pConfig->SetAttribute("off", RuleServer::formatDuration(delayOff_m, true));
    pConfig->SetAttribute("count", count_m);

    Action::exportXml(pConfig);

    if (stopCondition_m)
    {
        ticpp::Element pCond("stopcondition");
        stopCondition_m->exportXml(&pCond);
        pConfig->LinkEndChild(&pCond);
    }
}

void CycleOnOffAction::onChange(Object* object)
{
    if (stopCondition_m && running_m && stopCondition_m->evaluate())
        running_m = false;
}

void CycleOnOffAction::Run (pth_sem_t * stop)
{
    if (!object_m)
        return;
    running_m = true;
    if (sleep(delay_m, stop))
        return;
    logger_m.infoStream() << "Execute CycleOnOffAction" << endlog;
    for (int i=0; i<count_m; i++)
    {
        if (!running_m)
            break;
        object_m->setBoolValue(true);
        if (sleep(delayOn_m, stop))
            return;
        if (!running_m)
            break;
        object_m->setBoolValue(false);
        if (sleep(delayOff_m, stop))
            return;
    }
    if (running_m)
        running_m = false;
    else
        logger_m.infoStream() << "CycleOnOffAction stopped by condition" << endlog;
}

RepeatListAction::RepeatListAction()
    : period_m(0), count_m(0)
{}

RepeatListAction::~RepeatListAction()
{
    Stop();
    while (!actionsList_m.empty())
    {
        delete actionsList_m.front();
        actionsList_m.pop_front();
    }
}

void RepeatListAction::importXml(ticpp::Element* pConfig)
{
    std::string id;
    id = pConfig->GetAttribute("id");
    period_m = RuleServer::parseDuration(pConfig->GetAttribute("period"), false, true);
    pConfig->GetAttribute("count", &count_m);

    ticpp::Iterator<ticpp::Element> actionIt("action");
    for (actionIt = pConfig->FirstChildElement("action", false); actionIt != actionIt.end(); actionIt++ )
    {
        Action* action = Action::create(&(*actionIt));
        actionsList_m.push_back(action);
    }

    logger_m.infoStream() << "RepeatListAction: Configured with period=" << period_m
    << "; count=" << count_m << endlog;
}

void RepeatListAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "repeat");
    pConfig->SetAttribute("period", RuleServer::formatDuration(period_m, true));
    pConfig->SetAttribute("count", count_m);

    Action::exportXml(pConfig);

    ActionsList_t::iterator it;
    for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
    {
        ticpp::Element pElem("action");
        (*it)->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

void RepeatListAction::Run (pth_sem_t * stop)
{
    bool running = true;
    if (sleep(delay_m, stop))
        return;
    logger_m.infoStream() << "Execute RepeatListAction" << endlog;
    for (int i=0; i<count_m; i++)
    {
        ActionsList_t::iterator it;
        for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
            (*it)->execute();
        if (sleep(period_m, stop))
        {
            for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
                (*it)->cancel();
            return;
        }
    }
    // Wait until all actions are finished to be able to cancel them if main action is canceled
    while (running)
    {
        ActionsList_t::iterator it;
        running = false;
        for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
        {
            if (!(*it)->isFinished())
            {
                running = true;
                if (sleep(1000, stop))
                {
                    logger_m.infoStream() << "RepeatListAction canceled." << endlog;
                    for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
                        (*it)->cancel();
                    return;
                }
                break;
            }
        }
    }
}

ConditionalAction::ConditionalAction()
    : condition_m(0)
{}

ConditionalAction::~ConditionalAction()
{
    Stop();
    if (condition_m)
        delete condition_m;
    while (!actionsList_m.empty())
    {
        delete actionsList_m.front();
        actionsList_m.pop_front();
    }
}

void ConditionalAction::importXml(ticpp::Element* pConfig)
{
    ticpp::Element* pCondition = pConfig->FirstChildElement("condition");
    condition_m = Condition::create(pCondition, 0);

    ticpp::Iterator<ticpp::Element> actionIt("action");
    for (actionIt = pConfig->FirstChildElement("action", false); actionIt != actionIt.end(); actionIt++ )
    {
        Action* action = Action::create(&(*actionIt));
        actionsList_m.push_back(action);
    }

    logger_m.infoStream() << "ConditionalAction: Configured" << endlog;
}

void ConditionalAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "conditional");
    if (condition_m)
    {
        ticpp::Element pCond("condition");
        condition_m->exportXml(&pCond);
        pConfig->LinkEndChild(&pCond);
    }

    Action::exportXml(pConfig);

    ActionsList_t::iterator it;
    for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
    {
        ticpp::Element pElem("action");
        (*it)->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

void ConditionalAction::Run (pth_sem_t * stop)
{
    bool running = true;
    if (sleep(delay_m, stop))
        return;
    logger_m.infoStream() << "Execute ConditionalAction" << endlog;
    bool curValue = condition_m->evaluate();
    logger_m.infoStream() << "ConditionalAction evaluated as " << curValue << endlog;
    if (curValue)
    {
        ActionsList_t::iterator it;
        for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
            (*it)->execute();
    }
    // Wait until all actions are finished to be able to cancel them if main action is canceled
    while (running)
    {
        ActionsList_t::iterator it;
        running = false;
        for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
        {
            if (!(*it)->isFinished())
            {
                running = true;
                if (sleep(1000, stop))
                {
                    logger_m.infoStream() << "ConditionalAction canceled." << endlog;
                    for(it=actionsList_m.begin(); it != actionsList_m.end(); ++it)
                        (*it)->cancel();
                    return;
                }
                break;
            }
        }
    }
}

SendSmsAction::SendSmsAction() : varFlags_m(0)
{}

SendSmsAction::~SendSmsAction()
{}

void SendSmsAction::importXml(ticpp::Element* pConfig)
{
    id_m = pConfig->GetAttribute("id");
    value_m = pConfig->GetAttribute("value");
    if (pConfig->GetAttribute("var") == "true")
    {
        std::string tmp = id_m;
        varFlags_m = VarEnabled;
        if (parseVarString(tmp, true))
            varFlags_m |= VarId;
        tmp = value_m;
        if (parseVarString(tmp, true))
            varFlags_m |= VarValue;
    }
    else
        varFlags_m = 0;

    logger_m.infoStream() << "SendSmsAction: Configured for id " << id_m << " with value " << value_m << endlog;
}

void SendSmsAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "send-sms");
    pConfig->SetAttribute("id", id_m);
    pConfig->SetAttribute("value", value_m);
    if (varFlags_m & VarEnabled)
        pConfig->SetAttribute("var", "true");

    Action::exportXml(pConfig);
}

void SendSmsAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;

    std::string id = id_m;
    if (varFlags_m & VarId)
        parseVarString(id);
    std::string value = value_m;
    if (varFlags_m & VarValue)
        parseVarString(value);

    logger_m.infoStream() << "Execute SendSmsAction to id '" << id << "' with value '" << value << "'"<< endlog;

    Services::instance()->getSmsGateway()->sendSms(id, value);
}

SendEmailAction::SendEmailAction() : varFlags_m(0)
{}

SendEmailAction::~SendEmailAction()
{}

void SendEmailAction::importXml(ticpp::Element* pConfig)
{
    to_m = pConfig->GetAttribute("to");
    subject_m = pConfig->GetAttribute("subject");
    text_m = pConfig->GetText();
    if (pConfig->GetAttribute("var") == "true")
    {
        std::string tmp = to_m;
        varFlags_m = VarEnabled;
        if (parseVarString(tmp, true))
            varFlags_m |= VarTo;
        tmp = subject_m;
        if (parseVarString(tmp, true))
            varFlags_m |= VarSubject;
        tmp = text_m;
        if (parseVarString(tmp, true))
            varFlags_m |= VarText;
    }
    else
        varFlags_m = 0;

    logger_m.infoStream() << "SendEmailAction: Configured to=" << to_m << " subject=" << subject_m << endlog;
}

void SendEmailAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "send-email");
    pConfig->SetAttribute("to", to_m);
    pConfig->SetAttribute("subject", subject_m);
    if (varFlags_m & VarEnabled)
        pConfig->SetAttribute("var", "true");
    if (text_m != "")
    {
        ticpp::Text pText(text_m);
        pText.SetCDATA(true);
        pConfig->LinkEndChild(&pText);
    }

    Action::exportXml(pConfig);
}

void SendEmailAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;

    std::string to = to_m;
    if (varFlags_m & VarTo)
        parseVarString(to);
    std::string subject = subject_m;
    if (varFlags_m & VarSubject)
        parseVarString(subject);
    std::string text = text_m;
    if (varFlags_m & VarText)
        parseVarString(text);

    logger_m.infoStream() << "Execute SendEmailAction: to=" << to << " subject=" << subject << endlog;

    Services::instance()->getEmailGateway()->sendEmail(to, subject, text);
}

ShellCommandAction::ShellCommandAction() : varFlags_m(0)
{}

ShellCommandAction::~ShellCommandAction()
{}

void ShellCommandAction::importXml(ticpp::Element* pConfig)
{
    cmd_m = pConfig->GetAttribute("cmd");
    if (pConfig->GetAttribute("var") == "true")
    {
        std::string tmp = cmd_m;
        varFlags_m = VarEnabled;
        if (parseVarString(tmp, true))
            varFlags_m |= VarCmd;
    }
    else
        varFlags_m = 0;

    logger_m.infoStream() << "ShellCommandAction: Configured" << endlog;
}

void ShellCommandAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "shell-cmd");
    pConfig->SetAttribute("cmd", cmd_m);
    if (varFlags_m & VarEnabled)
        pConfig->SetAttribute("var", "true");

    Action::exportXml(pConfig);
}

void ShellCommandAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    std::string cmd = cmd_m;
    if (varFlags_m & VarCmd)
        parseVarString(cmd);
    logger_m.infoStream() << "Execute ShellCommandAction: " << cmd << endlog;

    int ret = pth_system(cmd.c_str());
    if (ret != 0)
        logger_m.infoStream() << "Execute ShellCommandAction: returned " << ret << endlog;
}

StartActionlistAction::StartActionlistAction() : list_m(true)
{}

StartActionlistAction::~StartActionlistAction()
{}

void StartActionlistAction::importXml(ticpp::Element* pConfig)
{
    pConfig->GetAttribute("rule-id", &ruleId_m);
    list_m = (pConfig->GetAttribute("list") != "false");
    logger_m.infoStream() << "StartActionlistAction: Configured" << endlog;
}

void StartActionlistAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "start-actionlist");
    pConfig->SetAttribute("rule-id", ruleId_m);
    pConfig->SetAttribute("list", list_m ? "true" : "false");

    Action::exportXml(pConfig);
}

void StartActionlistAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    logger_m.infoStream() << "Execute StartActionlistAction for rule ID: " << ruleId_m << endlog;

    Rule* rule = RuleServer::instance()->getRule(ruleId_m.c_str());
    if (rule) {
        if (list_m)
            rule->executeActionsTrue();
        else
            rule->executeActionsFalse();
    }
    else
        logger_m.errorStream() << "CancelAction: Rule not found '" << ruleId_m << "'" << endlog;
}

CancelAction::CancelAction()
{}

CancelAction::~CancelAction()
{}

void CancelAction::importXml(ticpp::Element* pConfig)
{
    pConfig->GetAttribute("rule-id", &ruleId_m);

    logger_m.infoStream() << "CancelAction: Configured" << endlog;
}

void CancelAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "cancel");
    pConfig->SetAttribute("rule-id", ruleId_m);

    Action::exportXml(pConfig);
}

void CancelAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    logger_m.infoStream() << "Execute CancelAction for rule ID: " << ruleId_m << endlog;

    Rule* rule = RuleServer::instance()->getRule(ruleId_m.c_str());
    if (rule)
        rule->cancel();
    else
        logger_m.errorStream() << "CancelAction: Rule not found '" << ruleId_m << "'" << endlog;
}

SetRuleActiveAction::SetRuleActiveAction() : active_m(true)
{}

SetRuleActiveAction::~SetRuleActiveAction()
{}

void SetRuleActiveAction::importXml(ticpp::Element* pConfig)
{
    pConfig->GetAttribute("rule-id", &ruleId_m);
    std::string value = pConfig->GetAttribute("active");
    active_m = (value != "off" && value != "false" && value != "no");

    logger_m.infoStream() << "SetRuleActiveAction: Configured (" << (active_m ? "yes" : "no" ) << ") for rule " << ruleId_m << endlog;
}

void SetRuleActiveAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "set-rule-active");
    pConfig->SetAttribute("rule-id", ruleId_m);
    pConfig->SetAttribute("active", (active_m ? "yes" : "no" ));

    Action::exportXml(pConfig);
}

void SetRuleActiveAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    logger_m.infoStream() << "Execute SetRuleActiveAction for rule ID: " << ruleId_m << endlog;

    Rule* rule = RuleServer::instance()->getRule(ruleId_m.c_str());
    if (rule)
        rule->setActive(active_m);
    else
        logger_m.errorStream() << "SetRuleActiveAction: Rule not found '" << ruleId_m << "'" << endlog;
}

Logger& Condition::logger_m(Logger::getInstance("Condition"));

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
    else if (type == "object-compare")
        return new ObjectComparisonCondition(cl);
    else if (type == "object-src")
        return new ObjectSourceCondition(cl);
    else if (type == "threshold")
        return new ObjectThresholdCondition(cl);
    else if (type == "time-counter")
        return new TimeCounterCondition(cl);
    else if (type == "ioport-rx")
        return new RxCondition(cl);
#ifdef HAVE_LUA
    else if (type == "script")
        return new LuaCondition(cl);
#endif
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

void AndCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "and");
    ConditionsList_t::iterator it;
    for (it = conditionsList_m.begin(); it != conditionsList_m.end(); it++)
    {
        ticpp::Element pElem("condition");
        (*it)->statusXml(&pElem);
        pStatus->LinkEndChild(&pElem);
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

void OrCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "or");
    ConditionsList_t::iterator it;
    for (it = conditionsList_m.begin(); it != conditionsList_m.end(); it++)
    {
        ticpp::Element pElem("condition");
        (*it)->statusXml(&pElem);
        pStatus->LinkEndChild(&pElem);
    }
}

NotCondition::NotCondition(ChangeListener* cl) : condition_m(0), cl_m(cl)
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

void NotCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "not");
    if (condition_m)
    {
        ticpp::Element pElem("condition");
        condition_m->statusXml(&pElem);
        pStatus->LinkEndChild(&pElem);
    }
}

ObjectCondition::ObjectCondition(ChangeListener* cl) : object_m(0), value_m(0), cl_m(cl), trigger_m(false), op_m(eq)
{}

ObjectCondition::~ObjectCondition()
{
    if (value_m)
        delete value_m;
    if (object_m && cl_m)
        object_m->removeChangeListener(cl_m);
    if (object_m)
        object_m->decRefCount();
}

bool ObjectCondition::evaluate()
{
    // if no value is defined, condition is always true
    bool val = (value_m == 0);
    if (!val)
    {
        int res = object_m->get()->compare(value_m);
        val = ((op_m & eq) && (res == 0)) || ((op_m & lt) && (res == -1)) || ((op_m & gt) && (res == 1));
    }
    logger_m.infoStream() << "ObjectCondition (id='" << object_m->getID()
    << "') evaluated as '" << val
    << "'" << endlog;
    return val;
}

void ObjectCondition::importXml(ticpp::Element* pConfig)
{
    std::string trigger;
    trigger = pConfig->GetAttribute("trigger");
    std::string id;
    id = pConfig->GetAttribute("id");
    if (object_m)
        object_m->decRefCount();
    object_m = ObjectController::instance()->getObject(id);

    if (trigger == "true")
    {
        if (!cl_m)
            throw ticpp::Exception("Trigger not supported in this context");
        trigger_m = true;
        object_m->addChangeListener(cl_m);
    }

    std::string value;
    value = pConfig->GetAttribute("value");
    if (value != "")
    {
        value_m = object_m->createObjectValue(value);
        logger_m.infoStream() << "ObjectCondition: configured value_m='" << value_m->toString() << "'" << endlog;
    }
    else
    {
        logger_m.infoStream() << "ObjectCondition: configured, no value specified" << endlog;
    }

    std::string op;
    op = pConfig->GetAttribute("op");
    if (op == "" || op == "eq")
        op_m = eq;
    else if (op == "lt")
        op_m = lt;
    else if (op == "gt")
        op_m = gt;
    else if (op == "ne")
        op_m = lt | gt;
    else if (op == "lte")
        op_m = lt | eq;
    else if (op == "gte")
        op_m = gt | eq;
    else
    {
        std::stringstream msg;
        msg << "ObjectCondition: operation not supported: '" << op << "'";
        throw ticpp::Exception(msg.str());
    }
}

void ObjectCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "object");
    pConfig->SetAttribute("id", object_m->getID());
    if (op_m != eq)
    {
        std::string op;
        if (op_m == lt)
            op = "lt";
        else if (op_m == gt)
            op = "gt";
        else if (op_m == (lt | eq))
            op = "lte";
        else if (op_m == (gt | eq))
            op = "gte";
        else
            op = "ne";
        pConfig->SetAttribute("op", op);
    }
    if (value_m)
        pConfig->SetAttribute("value", value_m->toString());
    if (trigger_m)
        pConfig->SetAttribute("trigger", "true");
}

void ObjectCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "object");
    pStatus->SetAttribute("id", object_m->getID());
    pStatus->SetAttribute("value", object_m->getValue());
    if (trigger_m)
        pStatus->SetAttribute("trigger", "true");
}

ObjectComparisonCondition::ObjectComparisonCondition(ChangeListener* cl) : ObjectCondition(cl), object2_m(0)
{}

ObjectComparisonCondition::~ObjectComparisonCondition()
{
    if (object2_m && cl_m)
        object2_m->removeChangeListener(cl_m);
    if (object2_m)
        object2_m->decRefCount();
}

bool ObjectComparisonCondition::evaluate()
{
    int res = object_m->get()->compare(object2_m->get());
    bool val = ((op_m & eq) && (res == 0)) || ((op_m & lt) && (res == -1)) || ((op_m & gt) && (res == 1));
    logger_m.infoStream() << "ObjectComparisonCondition (id='" << object_m->getID() << "'; id2='" << object2_m->getID()
    << "')" << endlog;
    return val;
}

void ObjectComparisonCondition::importXml(ticpp::Element* pConfig)
{
    std::string trigger;
    trigger = pConfig->GetAttribute("trigger");
    std::string id;
    id = pConfig->GetAttribute("id");
    if (object_m)
        object_m->decRefCount();
    object_m = ObjectController::instance()->getObject(id);
    id = pConfig->GetAttribute("id2");
    if (object2_m)
        object2_m->decRefCount();
    object2_m = ObjectController::instance()->getObject(id);

    if (trigger == "true")
    {
        if (!cl_m)
            throw ticpp::Exception("Trigger not supported in this context");
        trigger_m = true;
        object_m->addChangeListener(cl_m);
        object2_m->addChangeListener(cl_m);
    }

    std::string op;
    op = pConfig->GetAttribute("op");
    if (op == "" || op == "eq")
        op_m = eq;
    else if (op == "lt")
        op_m = lt;
    else if (op == "gt")
        op_m = gt;
    else if (op == "ne")
        op_m = lt | gt;
    else if (op == "lte")
        op_m = lt | eq;
    else if (op == "gte")
        op_m = gt | eq;
    else
    {
        std::stringstream msg;
        msg << "ObjectComparisonCondition: operation not supported: '" << op << "'";
        throw ticpp::Exception(msg.str());
    }
}

void ObjectComparisonCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "object-compare");
    pConfig->SetAttribute("id", object_m->getID());
    pConfig->SetAttribute("id2", object2_m->getID());
    if (op_m != eq)
    {
        std::string op;
        if (op_m == lt)
            op = "lt";
        else if (op_m == gt)
            op = "gt";
        else if (op_m == (lt | eq))
            op = "lte";
        else if (op_m == (gt | eq))
            op = "gte";
        else
            op = "ne";
        pConfig->SetAttribute("op", op);
    }
    if (trigger_m)
        pConfig->SetAttribute("trigger", "true");
}

void ObjectComparisonCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "object-compare");
    pStatus->SetAttribute("id", object_m->getID());
    pStatus->SetAttribute("id2", object2_m->getID());
    if (trigger_m)
        pStatus->SetAttribute("trigger", "true");
}

ObjectSourceCondition::ObjectSourceCondition(ChangeListener* cl) : ObjectCondition(cl), src_m(0)
{}

ObjectSourceCondition::~ObjectSourceCondition()
{}

bool ObjectSourceCondition::evaluate()
{
    bool val = (src_m == object_m->getLastTx()) && ObjectCondition::evaluate();
    logger_m.infoStream() << "ObjectSourceCondition (id='" << object_m->getID()
    << "') evaluated as '" << val
    << "'" << endlog;
    return val;
}

void ObjectSourceCondition::importXml(ticpp::Element* pConfig)
{
    std::string src;
    src = pConfig->GetAttribute("src");

    src_m = Object::ReadAddr(src);
    ObjectCondition::importXml(pConfig);
}

void ObjectSourceCondition::exportXml(ticpp::Element* pConfig)
{
    ObjectCondition::exportXml(pConfig);
    pConfig->SetAttribute("type", "object-src");
    pConfig->SetAttribute("src", Object::WriteAddr(src_m));
}

void ObjectSourceCondition::statusXml(ticpp::Element* pStatus)
{
    ObjectCondition::statusXml(pStatus);
    pStatus->SetAttribute("type", "object-src");
}

ObjectThresholdCondition::ObjectThresholdCondition(ChangeListener* cl) : ObjectCondition(cl), refValue_m(0), deltaUp_m(-1), deltaLow_m(-1), condition_m(0)
{}

ObjectThresholdCondition::~ObjectThresholdCondition()
{
    if (condition_m)
        delete condition_m;
}

bool ObjectThresholdCondition::evaluate()
{
    bool val = condition_m->evaluate();
    if (val)
        refValue_m = object_m->get()->toNumber();
    else
    {
        double delta = object_m->get()->toNumber() - refValue_m;
        if (deltaUp_m >= 0 && delta > deltaUp_m)
        {
            logger_m.infoStream() << "ObjectThresholdCondition (id='" << object_m->getID() << "') upper threshold reached" << endlog;
            return true;
        }
        if (deltaLow_m >= 0 && delta < -deltaLow_m)
        {
            logger_m.infoStream() << "ObjectThresholdCondition (id='" << object_m->getID() << "') lower threshold reached" << endlog;
            return true;
        }
    }
    return false;
}

void ObjectThresholdCondition::importXml(ticpp::Element* pConfig)
{
    if (!cl_m)
        throw ticpp::Exception("Threshold condition not supported in this context");
    pConfig->GetAttributeOrDefault("delta-up", &deltaUp_m, -1);
    pConfig->GetAttributeOrDefault("delta-low", &deltaLow_m, -1);
    condition_m = Condition::create(pConfig->FirstChildElement("resetcondition"), cl_m);

    std::string trigger;
    trigger = pConfig->GetAttribute("trigger");
    std::string id;
    id = pConfig->GetAttribute("id");
    if (object_m)
        object_m->decRefCount();
    object_m = ObjectController::instance()->getObject(id);

    if (trigger == "true")
    {
        if (!cl_m)
            throw ticpp::Exception("Trigger not supported in this context");
        trigger_m = true;
        object_m->addChangeListener(cl_m);
    }
}

void ObjectThresholdCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "threshold");
    if (deltaUp_m >= 0)
        pConfig->SetAttribute("delta-up", deltaUp_m);
    if (deltaLow_m >= 0)
        pConfig->SetAttribute("delta-low", deltaLow_m);

    if (condition_m)
    {
        ticpp::Element pElem("resetcondition");
        condition_m->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }



    pConfig->SetAttribute("id", object_m->getID());
    if (op_m != eq)
    {
        std::string op;
        if (op_m == lt)
            op = "lt";
        else if (op_m == gt)
            op = "gt";
        else if (op_m == (lt | eq))
            op = "lte";
        else if (op_m == (gt | eq))
            op = "gte";
        else
            op = "ne";
        pConfig->SetAttribute("op", op);
    }
    if (trigger_m)
        pConfig->SetAttribute("trigger", "true");
}

void ObjectThresholdCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "threshold");
    if (condition_m)
    {
        ticpp::Element pElem("condition");
        condition_m->statusXml(&pElem);
        pStatus->LinkEndChild(&pElem);
    }

    pStatus->SetAttribute("id", object_m->getID());
    if (trigger_m)
        pStatus->SetAttribute("trigger", "true");
}


TimerCondition::TimerCondition(ChangeListener* cl)
        : PeriodicTask(cl), trigger_m(false), initVal_m(initValGuess)
{}

TimerCondition::~TimerCondition()
{}

bool TimerCondition::evaluate()
{
    Condition::logger_m.infoStream() << "TimerCondition evaluated as '" << value_m << "'" << endlog;
    return value_m;
}

void TimerCondition::importXml(ticpp::Element* pConfig)
{
    std::string trigger = pConfig->GetAttribute("trigger");

    if (trigger == "true")
    {
        if (!cl_m)
            throw ticpp::Exception("Trigger not supported in this context");
        trigger_m = true;
    }
    else
        cl_m = 0;
    std::string initVal = pConfig->GetAttribute("initval");

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
        after_m = RuleServer::parseDuration(every->GetText());
    else
        throw ticpp::Exception("Timer must define <at> or <every> elements");

    ticpp::Element* during = pConfig->FirstChildElement("during", false);
    ticpp::Element* until = pConfig->FirstChildElement("until", false);
    if (during && until)
        throw ticpp::Exception("Timer can't define <until> and <during> elements simultaneously");
    if (during)
    {
        during_m = RuleServer::parseDuration(during->GetText());
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
    if (initVal == "true")
    {
        value_m = true;
        initVal_m = initValTrue;
    }
    else if (initVal == "false")
    {
        value_m = false;
        initVal_m = initValFalse;
    }
    else
        // if init value is not explicitly configured, we keep
        // the value guessed during reschedule(0)
        initVal_m = initValGuess;
}

void TimerCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "timer");
    if (trigger_m)
        pConfig->SetAttribute("trigger", "true");
        
    if (initVal_m == initValTrue)
        pConfig->SetAttribute("initval", "true");
    else if (initVal_m == initValFalse)
        pConfig->SetAttribute("initval", "false");

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
        pEvery.SetText(RuleServer::formatDuration(every));
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
        pDuring.SetText(RuleServer::formatDuration(during_m));
        pConfig->LinkEndChild(&pDuring);
    }
}

void TimerCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "timer");
    if (trigger_m)
        pStatus->SetAttribute("trigger", "true");
    PeriodicTask::statusXml(pStatus);
}

TimeCounterCondition::TimeCounterCondition(ChangeListener* cl) : condition_m(0), cl_m(cl), lastTime_m(0), lastVal_m(false), counter_m(0), threshold_m(0), resetDelay_m(0)
{}

TimeCounterCondition::~TimeCounterCondition()
{
    if (condition_m)
        delete condition_m;
}

bool TimeCounterCondition::evaluate()
{
    time_t now = time(0);
    bool val = condition_m->evaluate(); 
    if (lastVal_m && (counter_m < threshold_m))
    {
        counter_m += now - lastTime_m;
        Condition::logger_m.infoStream() << "TimeCounterCondition: counter is now  '" << counter_m << "'" << endlog;
    }
    if (val)
    {
        lastTime_m = now;
        lastVal_m = true;
        if (counter_m < threshold_m)
        {
            execTime_m = now + (threshold_m - counter_m) + 1;
            Services::instance()->getTimerManager()->removeTask(this);
            reschedule(0);
        }
    }
    else if (lastVal_m)
    {
        lastTime_m = now;
        lastVal_m = false;
        execTime_m = now + resetDelay_m + 1;
        Services::instance()->getTimerManager()->removeTask(this);
        reschedule(0);
    }

    if ((lastVal_m == false && lastTime_m > 0 && (now-lastTime_m) > resetDelay_m || lastTime_m == 0))
    {
        counter_m = 0;
        lastTime_m = 0;
        return false;
    }
    else
    {
        return (counter_m >= threshold_m);
    }
}

void TimeCounterCondition::onTimer(time_t time)
{
    if (cl_m)
        cl_m->onChange(0);
}

void TimeCounterCondition::importXml(ticpp::Element* pConfig)
{
    if (!cl_m)
        throw ticpp::Exception("TimeCounter condition not supported in this context");
    threshold_m = RuleServer::parseDuration(pConfig->GetAttribute("threshold"));
    resetDelay_m = RuleServer::parseDuration(pConfig->GetAttribute("reset-delay"));
    condition_m = Condition::create(pConfig->FirstChildElement("condition"), cl_m);
}

void TimeCounterCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "time-counter");
    if (threshold_m != 0)
        pConfig->SetAttribute("threshold", RuleServer::formatDuration(threshold_m));
    if (resetDelay_m != 0)
        pConfig->SetAttribute("reset-delay", RuleServer::formatDuration(resetDelay_m));

    if (condition_m)
    {
        ticpp::Element pElem("condition");
        condition_m->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

void TimeCounterCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "time-counter");
    FixedTimeTask::statusXml(pStatus);
    pStatus->SetAttribute("counter", counter_m);
    if (condition_m)
    {
        ticpp::Element pElem("condition");
        condition_m->statusXml(&pElem);
        pStatus->LinkEndChild(&pElem);
    }
}
