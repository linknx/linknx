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

#include "objectcontroller.h"
#include "persistentstorage.h"
#include "services.h"
#include <cmath>
#include <cassert>
#include <iomanip>
#include <iconv.h>

ObjectController* ObjectController::instance_m;

Logger& Object::logger_m(Logger::getInstance("Object"));

Object::Object() : init_m(false), flags_m(Default), refCount_m(0), gad_m(0), readRequestGad_m(0), persist_m(false), writeLog_m(false), readPending_m(false)
{}

Object::~Object()
{
    if (refCount_m > 0)
        logger_m.errorStream() << "Object (id=" << getID() << "): deleted object still has " << refCount_m << " references" << endlog;
}

Object* Object::create(const std::string& type)
{
    if (type == "" || type == "EIS1" || type == "1.001")
        return new SwitchingSwitchObject();
    else if (type.compare(0, 2, "1.") == 0)
    {    
        if (type == "1.002")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<2> >();
        else if (type == "1.003")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<3> >();
        else if (type == "1.004")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<4> >();
        else if (type == "1.005")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<5> >();
        else if (type == "1.006")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<6> >();
        else if (type == "1.007")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<7> >();
        else if (type == "1.008")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<8> >();
        else if (type == "1.009")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<9> >();
        else if (type == "1.010")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<10> >();
        else if (type == "1.011")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<11> >();
        else if (type == "1.012")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<12> >();
        else if (type == "1.013")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<13> >();
        else if (type == "1.014")
            return new SwitchingObjectImpl<SwitchingImplObjectValue<14> >();
        else return 0;
    }
    else if (type.compare(0, 2, "2.") == 0)
    {
        if (type == "2.xxx")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<0> >();
        else if (type == "2.001")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<1> >();
        else if (type == "2.002")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<2> >();
        else if (type == "2.003")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<3> >();
        else if (type == "2.004")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<4> >();
        else if (type == "2.005")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<5> >();
        else if (type == "2.006")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<6> >();
        else if (type == "2.007")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<7> >();
        else if (type == "2.008")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<8> >();
        else if (type == "2.009")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<9> >();
        else if (type == "2.010")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<10> >();
        else if (type == "2.011")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<11> >();
        else if (type == "2.012")
            return new SwitchingControlObject<SwitchingControlImplObjectValue<12> >();
        else return 0;
    }
    else if (type == "EIS2" || type == "3.007")
        return new DimmingObject();
    else if (type == "3.008")
        return new BlindsObject();
    else if (type == "4.001")
        return new AsciiCharObject();
    else if (type == "4.002")
        return new Latin1CharObject();
    else if (type == "EIS3" || type == "10.001")
        return new TimeObject();
    else if (type == "EIS4" || type == "11.001")
        return new DateObject();
    else if (type == "EIS5" || type == "9.xxx")
        return new ValueObjectImpl<ValueImplObjectValue<0> >();
    else if (type.compare(0, 2, "9.") == 0)
    {    
        if (type == "9.001")
            return new ValueObjectImpl<ValueImplObjectValue<1> >();
        else if (type == "9.002")
            return new ValueObjectImpl<ValueImplObjectValue<2> >();
        else if (type == "9.003")
            return new ValueObjectImpl<ValueImplObjectValue<3> >();
        else if (type == "9.004")
            return new ValueObjectImpl<ValueImplObjectValue<4> >();
        else if (type == "9.005")
            return new ValueObjectImpl<ValueImplObjectValue<5> >();
        else if (type == "9.006")
            return new ValueObjectImpl<ValueImplObjectValue<6> >();
        else if (type == "9.007")
            return new ValueObjectImpl<ValueImplObjectValue<7> >();
        else if (type == "9.008")
            return new ValueObjectImpl<ValueImplObjectValue<8> >();
        else if (type == "9.010")
            return new ValueObjectImpl<ValueImplObjectValue<10> >();
        else if (type == "9.011")
            return new ValueObjectImpl<ValueImplObjectValue<11> >();
        else if (type == "9.020")
            return new ValueObjectImpl<ValueImplObjectValue<20> >();
        else if (type == "9.021")
            return new ValueObjectImpl<ValueImplObjectValue<21> >();
        else if (type == "9.022")
            return new ValueObjectImpl<ValueImplObjectValue<22> >();
        else if (type == "9.023")
            return new ValueObjectImpl<ValueImplObjectValue<23> >();
        else if (type == "9.024")
            return new ValueObjectImpl<ValueImplObjectValue<24> >();
        else if (type == "9.025")
            return new ValueObjectImpl<ValueImplObjectValue<25> >();
        else if (type == "9.026")
            return new ValueObjectImpl<ValueImplObjectValue<26> >();
        else if (type == "9.027")
            return new ValueObjectImpl<ValueImplObjectValue<27> >();
        else if (type == "9.028")
            return new ValueObjectImpl<ValueImplObjectValue<28> >();
        else return 0;
    }
    else if (type == "14.xxx")
        return new ValueObject32();
    else if (type == "EIS6" || type == "5.xxx")
        return new U8Object();
    else if (type == "5.001")
        return new ScalingObject();
    else if (type == "5.003")
        return new AngleObject();
    else if (type == "5.010")
        return new U8CountObject();
    else if (type == "heat-mode" || type == "20.102")
        return new HeatingModeObject();
    else if (type == "EIS10" || type == "7.xxx")
        return new U16Object();
    else if (type == "EIS11" || type == "12.xxx")
        return new U32Object();
    else if (type == "EIS14" || type == "6.xxx")
        return new S8Object();
    else if (type == "8.xxx")
        return new S16Object();
    else if (type == "13.xxx")
        return new S32Object();
#ifdef STL_STREAM_SUPPORT_INT64
    else if (type == "29.xxx")
        return new S64Object();
#endif
    else if (type == "16.001")
        return new String14Object();
    else if (type == "EIS15" || type == "16.000")
        return new String14AsciiObject();
    else if (type == "28.001")
        return new StringObject();
    else if (type == "232.600")
        return new RGBObject();
    else if (type == "251.600")
        return new RGBWObject();
    else
        return 0;
}

Object* Object::create(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttribute("type");
    Object* obj = Object::create(type);
    if (obj == 0)
    {
        std::stringstream msg;
        msg << "Object type not supported: '" << type << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    obj->importXml(pConfig);
    return obj;
}

void Object::setValue(ObjectValue* value)
{
    if (set(value) || forceUpdate())
        onInternalUpdate();
}

void Object::setFloatValue(double value)
{
    if (set(value) || forceUpdate())
        onInternalUpdate();
}

ObjectValue* Object::get()
{
    if (!init_m)
        read();
    logger_m.debugStream() << "Object (id=" << getID() << "): get" << endlog;
    return getObjectValue();
}

void Object::importXml(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttribute("type");
    if (type != getType())
    {
        // sometimes, different type strings refer to the same type
        // in that case, create a temporary object just to get the type string.
        Object* obj = Object::create(type);
        type = obj->getType();
        delete obj;
        if (type != getType())
            throw ticpp::Exception("Changing type of existing object is not allowed");
    }
    std::string id = pConfig->GetAttribute("id");
    if (id == "")
        throw ticpp::Exception("Missing or empty object ID");
    if(id.find("/", 0) != std::string::npos)
    {
        std::stringstream msg;
        msg << "Slash character '/' not allowed in Object ID: " << id <<std::endl;
        throw ticpp::Exception(msg.str());
    }

    if (id_m == "")
        id_m = id;

    std::string gad = pConfig->GetAttributeOrDefault("gad", "nochange");
    // set default value to "nochange" just to see if the attribute was present or not in xml
    if (gad == "")
        gad_m = 0;
    else if (gad != "nochange")
        gad_m = Object::ReadGroupAddr(gad);
    readRequestGad_m = gad_m;

    bool has_descr = false;
    bool has_listener = false;
    ticpp::Iterator< ticpp::Node > child;
    for ( child = pConfig->FirstChild(false); child != child.end(); child++ )
    {
        std::string val = child->Value();
        if (child->Type() == TiXmlNode::TEXT && val.length())
        {
            if (!has_descr)
            {
                descr_m = "";
                has_descr = true;
            }
            descr_m.append(val);
        }
        else if (child->Type() == TiXmlNode::ELEMENT && val == "listener")
        {
            if (!has_listener)
            {
                listenerGadList_m.clear();
                has_listener = true;
            }
            std::string listener_gad = child->ToElement()->GetAttribute("gad");
            eibaddr_t gad = Object::ReadGroupAddr(listener_gad);
            listenerGadList_m.push_back(gad);
            if (child->ToElement()->GetAttribute("read") == "true")
                readRequestGad_m = gad;
        }
        //        else
        //        {
        //            std::stringstream msg;
        //            msg << "Invalid element '" << val << "' inside object definition" << std::endl;
        //            throw ticpp::Exception(msg.str());
        //        }
    }

    try
    {
        std::string flags;
        pConfig->GetAttribute("flags", &flags);
        flags_m = 0;
        if (flags.find('c') != flags.npos)
            flags_m |= Comm;
        if (flags.find('r') != flags.npos)
            flags_m |= Read;
        if (flags.find('w') != flags.npos)
            flags_m |= Write;
        if (flags.find('t') != flags.npos)
            flags_m |= Transmit;
        if (flags.find('u') != flags.npos)
            flags_m |= Update;
        if (flags.find('i') != flags.npos)
            flags_m |= Init;
        if (flags.find('s') != flags.npos || flags.find('f') != flags.npos)
            flags_m |= Stateless;
    }
    catch( ticpp::Exception& ex )
    {
        flags_m = Default;
    }

    writeLog_m = (pConfig->GetAttribute("log") == "true");

    std::string precision = pConfig->GetAttribute("precision");
    if (!precision.empty())
        getObjectValue()->setPrecision(precision);

    // TODO: do we need to use the 'i' flag instead of init="request" attribute
    persist_m = false;
    initValue_m = pConfig->GetAttribute("init");
    if (initValue_m == "persist")
    {
        PersistentStorage *persistence = Services::instance()->getPersistentStorage();
        if (persistence)
        {
            std::string val = persistence->read(id_m);
            if (val != "")
            {
                ObjectValue *objval = createObjectValue(val);
                set(objval); // Here, we use set() instead of setValue() to avoid call to onInternalUpdate()
                delete objval;
            }
            persist_m = true;
        }
        else
        {
            std::stringstream msg;
            msg << "Unable to persist object '" << id_m << "'; PersistentStorage not configured" << std::endl;
            throw ticpp::Exception(msg.str());
        }
    }
    else if (initValue_m != "" && initValue_m != "request")
    {
        ObjectValue *objval = createObjectValue(initValue_m);
        set(objval); // Here, we use set() instead of setValue() to avoid call to onInternalUpdate()
        delete objval;
    }

    logger_m.infoStream() << "Configured object '" << id_m << "': gad=" << WriteGroupAddr(gad_m) << endlog;
}

void Object::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", getType());
    pConfig->SetAttribute("id", id_m);

    if (gad_m != 0)
        pConfig->SetAttribute("gad", WriteGroupAddr(gad_m));

    if (initValue_m != "")
        pConfig->SetAttribute("init", initValue_m);

    if (writeLog_m)
        pConfig->SetAttribute("log", "true");
        
    std::string precision = getObjectValue()->getPrecision();
    if (!precision.empty())
        pConfig->SetAttribute("precision", precision);


    if (flags_m != Default)
    {
        std::stringstream flags;
        if (flags_m & Comm)
            flags << 'c';
        if (flags_m & Read)
            flags << 'r';
        if (flags_m & Write)
            flags << 'w';
        if (flags_m & Transmit)
            flags << 't';
        if (flags_m & Update)
            flags << 'u';
        if (flags_m & Init)
            flags << 'i';
        if (flags_m & Stateless)
            flags << 's';
        pConfig->SetAttribute("flags", flags.str());
    }

    if (descr_m != "")
        pConfig->SetText(descr_m);

    ListenerGadList_t::iterator it;
    for (it = listenerGadList_m.begin(); it != listenerGadList_m.end(); it++)
    {
        ticpp::Element pElem("listener");
        pElem.SetAttribute("gad", WriteGroupAddr(*it));
        if (readRequestGad_m == (*it))
            pElem.SetAttribute("read", "true");
        pConfig->LinkEndChild(&pElem);
    }
}

void Object::read()
{
    KnxConnection* con = Services::instance()->getKnxConnection();
	if (con->isVoid())
	{
		init_m = true;
		return;
	}

    if (!readPending_m)
    {
        uint8_t buf[2] = { 0, 0 };
        con->write(getReadRequestGad(), buf, 2);
    }
    readPending_m = true;

    pth_event_t tmout = pth_event (PTH_EVENT_TIME, pth_timeout(1,0));
    int cnt = 0;
    while (cnt < 100 && readPending_m)
    {
        if (con->isRunning())
        {
            if (con->checkInput(tmout) == -1)
                cnt = 100;
        }
        else
            pth_usleep(10000);
        ++cnt;
    }
    pth_event_free (tmout, PTH_FREE_THIS);
    // If the device didn't answer after 1 second, we consider the object's
    // default value as the current value to avoid waiting forever.
    init_m = true;
}

void Object::onInternalUpdate()
{
    if ((flags_m & Transmit) && (flags_m & Comm))
        doSend(true);
    onUpdate();
}

void Object::onUpdate()
{
    init_m = true;
    logger_m.infoStream() << "New value " << getValue() << " for object " << getID() << " (type: " << getType() << ")" << endlog;
    
    ListenerList_t::iterator it;
    for (it = listenerList_m.begin(); it != listenerList_m.end(); it++)
    {
        logger_m.debugStream() << "Calling onChange on listener for " << id_m << endlog;
        (*it)->onChange(this);
    }
    if (persist_m || writeLog_m)
    {
        PersistentStorage *persistence = Services::instance()->getPersistentStorage();
        if (persistence)
        {
            if (persist_m)
                persistence->write(id_m, getValue());
            if (writeLog_m)
                persistence->writelog(id_m, getValue());
        }
    }
}

void Object::onWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if ((flags_m & Write) && (flags_m & Comm))
    {
        lastTx_m = src;
        doWrite(buf, len, src);
    }
}

void Object::onRead(const uint8_t* buf, int len, eibaddr_t src)
{
    if ((flags_m & Read) && (flags_m & Comm))
        doSend(false);
}

void Object::onResponse(const uint8_t* buf, int len, eibaddr_t src)
{
    if ((flags_m & Update) && (flags_m & Comm))
    {
        readPending_m = false;
        lastTx_m = src;
        doWrite(buf, len, src);
    }
}

void Object::addChangeListener(ChangeListener* listener)
{
    logger_m.debugStream()  << "Adding listener to object '" << id_m << "'" << endlog;
    listenerList_m.push_back(listener);
}
void Object::removeChangeListener(ChangeListener* listener)
{
    listenerList_m.remove(listener);
}

eibaddr_t Object::ReadGroupAddr(const std::string& addr)
{
    int a, b, c;
    if (sscanf (addr.c_str(), "%d/%d/%d", &a, &b, &c) == 3)
        return ((a & 0x01f) << 11) | ((b & 0x07) << 8) | ((c & 0xff));
    if (sscanf (addr.c_str(), "%d/%d", &a, &b) == 2)
        return ((a & 0x01f) << 11) | ((b & 0x7FF));
    if (sscanf (addr.c_str(), "%x", &a) == 1)
        return a & 0xffff;
    std::stringstream msg;
    msg << "Object: Invalid group address format: '" << addr << "'" << std::endl;
    throw ticpp::Exception(msg.str());
}

eibaddr_t Object::ReadAddr(const std::string& addr)
{
    int a, b, c;
    if (sscanf (addr.c_str(), "%d.%d.%d", &a, &b, &c) == 3)
        return ((a & 0x0f) << 12) | ((b & 0x0f) << 8) | ((c & 0xff));
    if (sscanf (addr.c_str(), "%x", &a) == 1)
        return a & 0xffff;
    std::stringstream msg;
    msg << "Object: Invalid individual address format: '" << addr << "'" << std::endl;
    throw ticpp::Exception(msg.str());
}

std::string Object::WriteGroupAddr(eibaddr_t addr)
{
    char writegaddr_buf[16];
    sprintf (writegaddr_buf, "%d/%d/%d", (addr >> 11) & 0x1f, (addr >> 8) & 0x07, (addr) & 0xff);
    return std::string(writegaddr_buf);
}

std::string Object::WriteAddr(eibaddr_t addr)
{
    char writeaddr_buf[16];
    sprintf (writeaddr_buf, "%d.%d.%d", (addr >> 12) & 0x0f, (addr >> 8) & 0x0f, (addr) & 0xff);
    return std::string(writeaddr_buf);
}

KnxConnection* Object::getKnxConnection()
{
    return Services::instance()->getKnxConnection();
}

Logger& ObjectValue::logger_m(Logger::getInstance("ObjectValue"));

void SwitchingObjectValue::init(const std::string& value)
{
    if (value == "1" || value == "on" || value == getValueString(true))
        value_m = true;
    else if (value == "0" || value == "off" || value == getValueString(false))
        value_m = false;
    else
    {
        std::stringstream msg;
        msg << "SwitchingObjectValue: Bad value: '" << value << "' for object type " << getType() << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string SwitchingObjectValue::toString()
{
    return getValueString(value_m);
}

double SwitchingObjectValue::toNumber()
{
    return value_m ? 1.0 : 0.0;
}

bool SwitchingObjectValue::equals(ObjectValue* value)
{
    assert(value);
    SwitchingObjectValue* val = dynamic_cast<SwitchingObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "SwitchingObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "SwitchingObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int SwitchingObjectValue::compare(ObjectValue* value)
{
    assert(value);
    SwitchingObjectValue* val = dynamic_cast<SwitchingObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "SwitchingObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "SwitchingObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    if (value_m == val->value_m)
        return 0;
    else if (value_m)
        return 1;
    else
        return -1;
}

bool SwitchingObjectValue::set(bool value)
{
    if (value_m != value)
    {
        value_m = value;
        return true;
    }
    return false;
}

bool SwitchingObjectValue::set(ObjectValue* value)
{
    SwitchingObjectValue* val = dynamic_cast<SwitchingObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "SwitchingObjectValue: ERROR, set() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    return set(val->value_m);
}

bool SwitchingObjectValue::set(double value)
{
    return set(value != 0.0);
}

Logger& SwitchingObject::logger_m(Logger::getInstance("SwitchingObject"));

void SwitchingObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    bool newValue;
    if (len == 2)
        newValue = (buf[1] & 0x3F) != 0;
    else
        newValue = buf[2] != 0;

    if (set(newValue) || forceUpdate())
        onUpdate();
}

void SwitchingObject::doSend(bool isWrite)
{
    uint8_t buf[2] = { 0, (uint8_t)((isWrite ? 0x80 : 0x40) | (getBoolObjectValue() ? 1 : 0)) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 2);
}

void SwitchingControlObjectValue::init(const std::string& value)
{
    control_m = (value != "-1" && value != "no control");
    if (value == "1" || value == "on" || value == getValueString(true))
        value_m = true;
    else if (!control_m || value == "0" || value == "off" || value == getValueString(false))
        value_m = false;
    else
    {
        std::stringstream msg;
        msg << "SwitchingObjectValue: Bad value: '" << value << "' for object type " << getType() << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string SwitchingControlObjectValue::toString()
{
    if (!control_m)
        return "no control";
    return getValueString(value_m);
}

double SwitchingControlObjectValue::toNumber()
{
    return control_m ? (value_m ? 1.0 : 0.0) : -1.0;
}

bool SwitchingControlObjectValue::equals(ObjectValue* value)
{
    assert(value);
    SwitchingControlObjectValue* val = dynamic_cast<SwitchingControlObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "SwitchingControlObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "SwitchingControlObjectValue: Compare value_m='" << value_m << "' : control_m='" << control_m << "' to value='" << val->value_m << "' : control='" << val->control_m << "'" << endlog;
    return (!control_m && !val->control_m) || (control_m && val->control_m && value_m == val->value_m);
}

int SwitchingControlObjectValue::compare(ObjectValue* value)
{
    assert(value);
    SwitchingControlObjectValue* val = dynamic_cast<SwitchingControlObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "SwitchingControlObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "SwitchingControlObjectValue: Compare value_m='" << value_m << "' : control_m='" << control_m << "' with value='" << val->value_m << "' : control='" << val->control_m << "'" << endlog;
    if (!control_m && !val->control_m)
        return 0;
    else if (control_m && !val->control_m)
        return 1;
    else if (!control_m && val->control_m)
        return -1;
    else if (value_m == val->value_m)
        return 0;
    else if (value_m)
        return 1;
    else
        return -1;
}

bool SwitchingControlObjectValue::set(bool value, bool control)
{
    if (control_m != control || (control && (value_m != value)))
    {
        value_m = value;
        control_m = control;
        return true;
    }
    return false;
}

bool SwitchingControlObjectValue::set(ObjectValue* value)
{
    SwitchingControlObjectValue* val = dynamic_cast<SwitchingControlObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "SwitchingControlObjectValue: ERROR, set() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    return set(val->value_m, val->control_m);
}

bool SwitchingControlObjectValue::set(double value)
{
    if (value < 0)
        return set(false, false);
    return set(value != 0.0, true);
}

bool StepDirObjectValue::equals(ObjectValue* value)
{
    assert(value);
    StepDirObjectValue* val = dynamic_cast<StepDirObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "StepDirObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "StepDirObjectValue: Compare object='"
    << toString() << "' to value='"
    << val->toString() << "'" << endlog;
    return (direction_m == val->direction_m) && (stepcode_m == val->stepcode_m);
}

int StepDirObjectValue::compare(ObjectValue* value)
{
    assert(value);
    StepDirObjectValue* val = dynamic_cast<StepDirObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "StepDirObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "StepDirObjectValue: Compare object='"
    << toString() << "' to value='"
    << val->toString() << "'" << endlog;

    if (stepcode_m == 0 && val->stepcode_m == 0)
        return 0;
    if (direction_m == val->direction_m)
    {
        if (stepcode_m == val->stepcode_m)
            return 0;
        if (stepcode_m > val->stepcode_m) // bigger stepcode => smaller steps
            return direction_m ? -1 : 1;
        else
            return direction_m ? 1 : -1;
    }
    return direction_m ? 1 : -1;
}

bool StepDirObjectValue::set(ObjectValue* value)
{
    StepDirObjectValue* val = dynamic_cast<StepDirObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "StepDirObjectValue: ERROR, set() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (direction_m != val->direction_m || stepcode_m != val->stepcode_m)
    {
        direction_m = val->direction_m;
        stepcode_m = val->stepcode_m;
        return true;
    }
    return false;
}

bool StepDirObjectValue::set(double value)
{
    int direction = 1;
    int stepcode = 0;
    if (value < 0.0)
    {
        direction = 0;
        value = -value;
    }
    stepcode = static_cast<int>(value);
    if (direction_m != direction || stepcode_m != stepcode)
    {
        direction_m = direction;
        stepcode_m = stepcode;
        return true;
    }
    return false;
}

double StepDirObjectValue::toNumber()
{
    if (stepcode_m == 0)
        return 0.0;
    return (direction_m ? 1.0 : -1.0) * stepcode_m;
}

Logger& StepDirObject::logger_m(Logger::getInstance("StepDirObject"));

StepDirObject::StepDirObject()
{}

StepDirObject::~StepDirObject()
{}

void StepDirObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int newValue;
    if (len == 2)
        newValue = (buf[1] & 0x3F);
    else
        newValue = buf[2];
    int direction = (newValue & 0x08) >> 3;
    int stepcode = newValue & 0x07;
    if (stepcode == 0)
        direction = 0;

    if (setStep(direction, stepcode) || forceUpdate())
        onUpdate();
}

void StepDirObject::doSend(bool isWrite)
{
    uint8_t buf[2] = { 0, (uint8_t)((isWrite ? 0x80 : 0x40) | (getDirection() ? 8 : 0) | (getStepCode() & 0x07)) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 2);
}

DimmingObjectValue::DimmingObjectValue(const std::string& value)
{
    std::string dir;
    size_t pos = value.find(":");
    dir = value.substr(0, pos);
    stepcode_m = 1;
    if (pos != value.npos)
    {
        if (value.length() > pos+1)
        {
            char step = value[pos+1];
            if (step >= '1' && step <= '7')
                stepcode_m = step - '0';
            else
            {
                std::stringstream msg;
                msg << "DimmingObjectValue: Invalid stepcode (must be between 1 and 7): '" << step << "'" << std::endl;
                throw ticpp::Exception(msg.str());
            }
        }
    }
    if (dir == "stop")
    {
        direction_m = 0;
        stepcode_m = 0;
    }
    else if (dir == "up")
        direction_m = 1;
    else if (dir == "down")
        direction_m = 0;
    else
    {
        std::stringstream msg;
        msg << "DimmingObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string DimmingObjectValue::toString()
{
    if (stepcode_m == 0)
        return "stop";
    std::string ret(direction_m ? "up" : "down");
    if (stepcode_m != 1)
    {
        ret.push_back(':');
        ret.push_back('0' + stepcode_m);
    }
    return ret;
}

Logger& DimmingObject::logger_m(Logger::getInstance("DimmingObject"));

ObjectValue* DimmingObject::createObjectValue(const std::string& value)
{
    return new DimmingObjectValue(value);
}

void DimmingObject::setValue(const std::string& value)
{
    DimmingObjectValue val(value);
    Object::setValue(&val);
}

void DimmingObject::setStepValue(int direction, int stepcode)
{
    if (forceUpdate() || stepcode_m != stepcode  || direction_m != direction)
    {
        stepcode_m = stepcode;
        direction_m = direction;
        onInternalUpdate();
    }
}

BlindsObjectValue::BlindsObjectValue(const std::string& value)
{
    std::string dir;
    size_t pos = value.find(":");
    dir = value.substr(0, pos);
    stepcode_m = 1;
    if (pos != value.npos)
    {
        if (value.length() > pos+1)
        {
            char step = value[pos+1];
            if (step >= '1' && step <= '7')
                stepcode_m = step - '0';
            else
            {
                std::stringstream msg;
                msg << "BlindsObjectValue: Invalid stepcode (must be between 1 and 7): '" << step << "'" << std::endl;
                throw ticpp::Exception(msg.str());
            }
        }
    }
    if (dir == "stop")
    {
        direction_m = 0;
        stepcode_m = 0;
    }
    else if (dir == "close")
        direction_m = 1;
    else if (dir == "open")
        direction_m = 0;
    else
    {
        std::stringstream msg;
        msg << "BlindsObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string BlindsObjectValue::toString()
{
    if (stepcode_m == 0)
        return "stop";
    std::string ret(direction_m ? "close" : "open");
    if (stepcode_m != 1)
    {
        ret.push_back(':');
        ret.push_back('0' + stepcode_m);
    }
    return ret;
}

Logger& BlindsObject::logger_m(Logger::getInstance("BlindsObject"));

ObjectValue* BlindsObject::createObjectValue(const std::string& value)
{
    return new BlindsObjectValue(value);
}

void BlindsObject::setValue(const std::string& value)
{
    BlindsObjectValue val(value);
    Object::setValue(&val);
}

void BlindsObject::setStepValue(int direction, int stepcode)
{
    if (forceUpdate() || stepcode_m != stepcode  || direction_m != direction)
    {
        stepcode_m = stepcode;
        direction_m = direction;
        onInternalUpdate();
    }
}

TimeObjectValue::TimeObjectValue(const std::string& value) : wday_m(-1), hour_m(-1), min_m(-1), sec_m(-1)
{
    if (value == "now")
        return;
    std::istringstream val(value);
    char s1, s2;
    val >> hour_m >> s1 >> min_m >> s2 >> sec_m;
    wday_m = 0;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         s1 != ':' || s2 != ':' ||
         hour_m < 0 || hour_m > 23  || min_m < 0 || min_m > 59 || sec_m < 0 || sec_m > 59 )
    {
        std::stringstream msg;
        msg << "TimeObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string TimeObjectValue::toString()
{
    if (hour_m == -1)
        return "now";
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) 
    << hour_m << ":" << std::setfill('0') << std::setw(2) 
    << min_m << ":" << std::setfill('0') << std::setw(2)
    << sec_m;
    return out.str();
}

double TimeObjectValue::toNumber()
{
    if (hour_m == -1)
        return -1.0;
    return hour_m * 3600 + min_m * 60 + sec_m;
}

bool TimeObjectValue::equals(ObjectValue* value)
{
    int wday, hour, min, sec;
    assert(value);
    TimeObjectValue* val = dynamic_cast<TimeObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "TimeObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    // logger_m.debugStream() << "TimeObjectValue (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    val->getTimeValue(&wday, &hour, &min, &sec);
    return (sec_m == sec) && (min_m == min) && (hour_m == hour) && (wday_m == wday);
}

int TimeObjectValue::compare(ObjectValue* value)
{
    int wday, hour, min, sec;
    assert(value);
    TimeObjectValue* val = dynamic_cast<TimeObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "TimeObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    // logger_m.debugStream() << "TimeObjectValue (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    val->getTimeValue(&wday, &hour, &min, &sec);

    if (wday_m > wday)
        return 1;
    if (wday_m < wday)
        return -1;

    if (hour_m > hour)
        return 1;
    if (hour_m < hour)
        return -1;

    if (min_m > min)
        return 1;
    if (min_m < min)
        return -1;

    if (sec_m > sec)
        return 1;
    if (sec_m < sec)
        return -1;
    return 0;
}

bool TimeObjectValue::set(ObjectValue* value)
{
    int wday, hour, min, sec;
    assert(value);
    TimeObjectValue* val = dynamic_cast<TimeObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "TimeObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        val->getTimeValue(&wday, &hour, &min, &sec);
        if (wday_m != wday || hour_m != hour || min_m != min || sec_m != sec)
        {
            wday_m = wday;
            hour_m = hour;
            min_m = min;
            sec_m = sec;
            return true;
        }
    }
    return false;
}

bool TimeObjectValue::set(double value)
{
    int wday, hour, min, sec;
    if (value < 0)
    {
        wday = -1;
        hour = -1;
        min = -1;
        sec = -1;
    }
    else
    {
        wday = 0;
        hour = value / 3600;
        value -= hour * 3600;
        min = value / 60;
        sec = value - min * 60;
    }
    if (wday_m != wday || hour_m != hour || min_m != min || sec_m != sec)
    {
        wday_m = wday;
        hour_m = hour;
        min_m = min;
        sec_m = sec;
        return true;
    }
    return false;
}

void TimeObjectValue::getTimeValue(int *wday, int *hour, int *min, int *sec)
{
    if (hour_m == -1)
    {
        time_t t = time(0);
        struct tm * timeinfo = localtime(&t);
        *wday = timeinfo->tm_wday;
        if (*wday == 0)
            *wday = 7;
        *hour = timeinfo->tm_hour;
        *min = timeinfo->tm_min;
        *sec = timeinfo->tm_sec;
    }
    else
    {
        *wday = wday_m;
        *hour = hour_m;
        *min = min_m;
        *sec = sec_m;
    }
}

Logger& TimeObject::logger_m(Logger::getInstance("TimeObject"));

TimeObject::TimeObject() : TimeObjectValue(0, 0, 0, 0)
{}

TimeObject::~TimeObject()
{}

ObjectValue* TimeObject::createObjectValue(const std::string& value)
{
    return new TimeObjectValue(value);
}

void TimeObject::setValue(const std::string& value)
{
    TimeObjectValue val(value);
    Object::setValue(&val);
}

void TimeObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 5)
    {
        logger_m.errorStream() << "Invalid packet received for TimeObject (too short)" << endlog;
        return;
    }
    int wday, hour, min, sec;

    wday = (buf[2] & 0xE0) >> 5;
    hour = buf[2] & 0x1F;
    min = buf[3];
    sec = buf[4];
    if (forceUpdate() || wday != wday_m || hour != hour_m || min != min_m || sec != sec_m)
    {
        wday_m = wday;
        hour_m = hour;
        min_m = min;
        sec_m = sec;
        onUpdate();
    }
}

void TimeObject::setTime(time_t time)
{
    struct tm * timeinfo = localtime(&time);
    int wday = timeinfo->tm_wday;
    if (wday == 0)
        wday = 7;
    setTime(wday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void TimeObject::doSend(bool isWrite)
{
    uint8_t buf[5] = { 0, (uint8_t)(isWrite ? 0x80 : 0x40), (uint8_t)(((wday_m<<5) & 0xE0) | (hour_m & 0x1F)), (uint8_t)min_m, (uint8_t)sec_m };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 5);
}

void TimeObject::setTime(int wday, int hour, int min, int sec)
{
    TimeObjectValue val(wday, hour, min, sec);
    Object::setValue(&val);
}

void TimeObject::getTime(int *wday, int *hour, int *min, int *sec)
{
    *wday = wday_m;
    *hour = hour_m;
    *min = min_m;
    *sec = sec_m;
}

DateObjectValue::DateObjectValue(const std::string& value) : day_m(-1), month_m(-1), year_m(-1)
{
    if (value == "now")
        return;
    std::istringstream val(value);
    char s1, s2;
    val >> year_m >> s1 >> month_m >> s2 >> day_m;
    year_m -= 1900;
    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         s1 != '-' || s2 != '-' ||
         year_m < 0 || year_m > 255 || month_m < 1 || month_m > 12 || day_m < 1 || day_m > 31)
    {
        std::stringstream msg;
        msg << "DateObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string DateObjectValue::toString()
{
    if (day_m == -1)
        return "now";
    std::ostringstream out;
    out << year_m+1900 << "-" << month_m << "-" << day_m;
    return out.str();
}

double DateObjectValue::toNumber()
{
    if (day_m == -1)
        return -1.0;
    return year_m * 400 + month_m * 31 + day_m;
}

bool DateObjectValue::equals(ObjectValue* value)
{
    int day, month, year;
    assert(value);
    DateObjectValue* val = dynamic_cast<DateObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "DateObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    // logger_m.debugStream() << "DateObjectValue (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    val->getDateValue(&day, &month, &year);
    return (day_m == day) && (month_m == month) && (year_m == year);
}

int DateObjectValue::compare(ObjectValue* value)
{
    int day, month, year;
    assert(value);
    DateObjectValue* val = dynamic_cast<DateObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "DateObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    // logger_m.debugStream() << "DateObjectValue (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    val->getDateValue(&day, &month, &year);

    if (year_m > year)
        return 1;
    if (year_m < year)
        return -1;

    if (month_m > month)
        return 1;
    if (month_m < month)
        return -1;

    if (day_m > day)
        return 1;
    if (day_m < day)
        return -1;
    return 0;
}

bool DateObjectValue::set(ObjectValue* value)
{
    int day, month, year;
    assert(value);
    DateObjectValue* val = dynamic_cast<DateObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "DateObjectValue: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        val->getDateValue(&day, &month, &year);
        if ( day_m != day || month_m != month || year_m != year )
        {
            day_m = day;
            month_m = month;
            year_m = year;
            return true;
        }
    }
    return false;
}

bool DateObjectValue::set(double value)
{
    int day, month, year;
    if (value < 0)
    {
        day = -1;
        month = -1;
        year = -1;
    }
    else
    {
        year = value / 400;
        value -= year * 400;
        month = value / 31;
        day = value - month * 31;
    }
    if ( day_m != day || month_m != month || year_m != year )
    {
        day_m = day;
        month_m = month;
        year_m = year;
        return true;
    }
    return false;
}

void DateObjectValue::getDateValue(int *day, int *month, int *year)
{
    if (day_m == -1)
    {
        time_t t = time(0);
        struct tm * timeinfo = localtime(&t);
        *day = timeinfo->tm_mday;
        *month = timeinfo->tm_mon+1;
        *year = timeinfo->tm_year;
    }
    else
    {
        *day = day_m;
        *month = month_m;
        *year = year_m;
    }
}

Logger& DateObject::logger_m(Logger::getInstance("DateObject"));

DateObject::DateObject() : DateObjectValue(0, 0, 0)
{}

DateObject::~DateObject()
{}

ObjectValue* DateObject::createObjectValue(const std::string& value)
{
    return new DateObjectValue(value);
}

void DateObject::setValue(const std::string& value)
{
    DateObjectValue val(value);
    Object::setValue(&val);
}

void DateObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 5)
    {
        logger_m.errorStream() << "Invalid packet received for DateObject (too short)" << endlog;
        return;
    }
    int day, month, year;

    day = buf[2];
    month = buf[3];
    year = buf[4];
    if (year < 90)
        year += 100;
    if (forceUpdate() || day != day_m || month != month_m || year != year_m)
    {
        day_m = day;
        month_m = month;
        year_m = year;
        onUpdate();
    }
}

void DateObject::setDate(time_t time)
{
    struct tm * timeinfo = localtime(&time);
    setDate(timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year);
}

void DateObject::doSend(bool isWrite)
{
	BufferBuilder builder(5, DateObject::logger_m);
	builder << 0 << (isWrite ? 0x80 : 0x40) << day_m << month_m;
	builder << ((year_m >= 100 && year_m < 190) ? year_m-100 : year_m);

    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 5);
}

void DateObject::setDate(int day, int month, int year)
{
    if (year >= 1900)
        year -= 1900;
    DateObjectValue val(day, month, year);
    Object::setValue(&val);
}

void DateObject::getDate(int *day, int *month, int *year)
{
    *day = day_m;
    *month = month_m;
    if (year_m < 1900)
        *year = 1900 + year_m;
    else
        *year = 1900;
}

void ValueObjectValue::init(const std::string& value)
{
    precision_m = 0;
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         value_m > getBound(true) ||
         value_m < getBound(false))
    {
        std::stringstream msg;
        msg << "ValueObjectValue: Bad value: '" << value << "' for object type " << getType() << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

void ValueObjectValue::setPrecision(std::string precision)
{
    std::istringstream val(precision);
    val >> precision_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "ValueObjectValue: Bad precision: '" << precision << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string ValueObjectValue::getPrecision()
{
    if (precision_m == 0)
        return "";
    else
    {
        std::ostringstream out;
        out << precision_m;
        return out.str();
    }
}

std::string ValueObjectValue::toString()
{
    std::ostringstream out;
    out.precision(8);
    out << value_m;
    return out.str();
}

double ValueObjectValue::toNumber()
{
    return value_m;
}

bool ValueObjectValue::equals(ObjectValue* value)
{
    assert(value);
    ValueObjectValue* val = dynamic_cast<ValueObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "ValueObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "ValueObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int ValueObjectValue::compare(ObjectValue* value)
{
    assert(value);
    ValueObjectValue* val = dynamic_cast<ValueObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "ValueObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "ValueObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;

    if (value_m == val->value_m)
        return 0;
    if (value_m > val->value_m)
        return 1;
    else
        return -1;
}

bool ValueObjectValue::set(ObjectValue* value)
{
    assert(value);
    ValueObjectValue* val = dynamic_cast<ValueObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "ValueObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        return set(val->value_m);
    }
    return false;
}

double ValueObjectValue::roundToKnxPrecision(double value)
{
    int ex = 0;
    int m = (int)rint(value * 100);
    if (m < 0)
    {
        m = -m;
        while (m > 2048)
        {
            m = m >> 1;
            ex++;
        }
        m = -m;
    }
    else
    {
        while (m > 2047)
        {
            m = m >> 1;
            ex++;
        }
    }
    return ((double)m * (1 << ex) / 100);
}

bool ValueObjectValue::set(double value)
{
    if (precision_m != 0) {
        int div = (int) (value/precision_m + (value >= 0 ? 0.5 : -0.5));
        value = div*precision_m;
        logger_m.debugStream() << "ValueObject: rounded value "<< value << endlog;
    }
    else {
        value = roundToKnxPrecision(value);
    }
    if (value_m != value)
    {
        value_m = value;
        return true;
    }
    return false;
}

Logger& ValueObject::logger_m(Logger::getInstance("ValueObject"));

ValueObject::ValueObject()
{}

ValueObject::~ValueObject()
{}
/*
ObjectValue* ValueObject::createObjectValue(const std::string& value)
{
    return new ValueObjectValue(value);
}

void ValueObject::setValue(const std::string& value)
{
    ValueObjectValue val(value);
    Object::setValue(&val);
}
*/
void ValueObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 4)
    {
        logger_m.errorStream() << "Invalid packet received for ValueObject (too short)" << endlog;
        return;
    }
    double newValue;
    int d1 = ((unsigned char) buf[2]) * 256 + (unsigned char) buf[3];
    int m = d1 & 0x7ff;
    if (d1 & 0x8000)
        m |= ~0x7ff;
    int ex = (d1 & 0x7800) >> 11;
    newValue = ((double)m * (1 << ex) / 100);
    if (set(newValue) || forceUpdate())
    {
        onUpdate();
    }
}

void ValueObject::doSend(bool isWrite)
{
	BufferBuilder builder(4, ValueObject::logger_m);
	builder << 0 << (isWrite ? 0x80 : 0x40) << 0 << 0;
    int ex = 0;
    int m = (int)rint(getFloatValue() * 100);
    if (m < 0)
    {
        m = -m;
        while (m > 2048)
        {
            m = m >> 1;
            ex++;
        }
        m = -m;
		builder.setValue(2, ((m >> 8) & 0x07) | ((ex << 3) & 0x78) | (1 << 7));
    }
    else
    {
        while (m > 2047)
        {
            m = m >> 1;
            ex++;
        }
        builder.setValue(2, ((m >> 8) & 0x07) | ((ex << 3) & 0x78));
    }
    builder.setValue(3, m & 0xff);

    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 4);
}
/*
void ValueObjectImpl::setFloatValue(double value)
{
    ValueObjectValue val(value);
    Object::setValue(&val);
}*/

ValueObject32Value::ValueObject32Value(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "ValueObject32Value: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string ValueObject32Value::toString()
{
    std::ostringstream out;
    out.precision(8);
    out << value_m;
    return out.str();
}

Logger& ValueObject32::logger_m(Logger::getInstance("valueObject32"));

ObjectValue* ValueObject32::createObjectValue(const std::string& value)
{
    return new ValueObject32Value(value);
}

void ValueObject32::setValue(const std::string& value)
{
    ValueObject32Value val(value);
    Object::setValue(&val);
}

void ValueObject32::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 6)
    {
        logger_m.errorStream() << "Invalid packet received for ValueObject32 (too short)" << endlog;
        return;
    }

    convfloat tmp;
    tmp.u32 = buf[2]<<24 | buf[3]<<16 | buf[4]<<8 | buf[5];
//    logger_m.infoStream() << "New value int tmp " << tmp << " for ValueObject32 " << getID() << endlog;
//    const float* nv = reinterpret_cast<const float*>(&tmp);
    double newValue = tmp.fl;
    if (set(newValue) || forceUpdate())
    {
        onUpdate();
    }
}

void ValueObject32::doSend(bool isWrite)
{
	BufferBuilder builder(6, ValueObject32::logger_m);
	builder << 0 << (isWrite ? 0x80 : 0x40) << 0 << 0 << 0 << 0;
    convfloat tmp;
    tmp.fl = static_cast<float>(value_m);
    builder.setValue(5, (tmp.u32 & 0x000000FF));
    builder.setValue(4, ((tmp.u32 & 0x0000FF00) >> 8));
    builder.setValue(3, ((tmp.u32 & 0x00FF0000) >> 16));
    builder.setValue(2, ((tmp.u32 & 0xFF000000) >> 24));

    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 6);
}

UIntObjectValue::UIntObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "UIntObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string UIntObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

double UIntObjectValue::toNumber()
{
    return value_m;
}

bool UIntObjectValue::equals(ObjectValue* value)
{
    assert(value);
    UIntObjectValue* val = dynamic_cast<UIntObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "UIntObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "UIntObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int UIntObjectValue::compare(ObjectValue* value)
{
    assert(value);
    UIntObjectValue* val = dynamic_cast<UIntObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "UIntObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "UIntObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;

    if (value_m == val->value_m)
        return 0;
    if (value_m > val->value_m)
        return 1;
    else
        return -1;
}

bool UIntObjectValue::set(ObjectValue* value)
{
    assert(value);
    UIntObjectValue* val = dynamic_cast<UIntObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "UIntObjectValue: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        if (value_m != val->value_m)
        {
            value_m = val->value_m;
            return true;
        }
    }
    return false;
}

Logger& UIntObject::logger_m(Logger::getInstance("UIntObject"));

UIntObject::UIntObject()
{}

UIntObject::~UIntObject()
{}

void UIntObject::setIntValue(uint32_t value)
{
    if (setInt(value) || forceUpdate())
        onInternalUpdate();
}

uint32_t UIntObject::getIntValue()
{
    if (!init_m)
        read();
    return getInt();
}

Logger& U8ImplObject::logger_m(Logger::getInstance("U8ImplObject"));

U8ImplObject::U8ImplObject()
{}

U8ImplObject::~U8ImplObject()
{}

void U8ImplObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    uint32_t newValue;
    if (len == 2)
        newValue = (buf[1] & 0x3F);
    else
        newValue = buf[2];
    if (setInt(newValue) || forceUpdate())
        onUpdate();
}

void U8ImplObject::doSend(bool isWrite)
{
	BufferBuilder builder(3, U8ImplObject::logger_m);
	builder << 0 << (isWrite ? 0x80 : 0x40) << (getInt() & 0xff);
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 3);
}

U8ObjectValue::U8ObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         value_m > 255 ||
         value_m < 0)
    {
        std::stringstream msg;
        msg << "U8ObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string U8ObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

Logger& U8Object::logger_m(Logger::getInstance("U8Object"));

U8Object::U8Object()
{}

U8Object::~U8Object()
{}

ObjectValue* U8Object::createObjectValue(const std::string& value)
{
    return new U8ObjectValue(value);
}

void U8Object::setValue(const std::string& value)
{
    U8ObjectValue val(value);
    Object::setValue(&val);
}

ScalingObjectValue::ScalingObjectValue(const std::string& value)
{
    float fvalue;
    std::istringstream val(value);
    val >> fvalue;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         fvalue > 100 ||
         fvalue < 0)
    {
        std::stringstream msg;
        msg << "ScalingObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    value_m = (int)(fvalue * 255 / 100);
}

std::string ScalingObjectValue::toString()
{
    std::ostringstream out;
    out.precision(3);
    out << (float)value_m * 100 / 255;
    return out.str();
}

Logger& ScalingObject::logger_m(Logger::getInstance("ScalingObject"));

ScalingObject::ScalingObject(): ScalingObjectValue(0)
{}

ScalingObject::~ScalingObject()
{}

ObjectValue* ScalingObject::createObjectValue(const std::string& value)
{
    return new ScalingObjectValue(value);
}

void ScalingObject::setValue(const std::string& value)
{
    ScalingObjectValue val(value);
    Object::setValue(&val);
}

AngleObjectValue::AngleObjectValue(const std::string& value)
{
    float fvalue;
    std::istringstream val(value);
    val >> fvalue;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         fvalue > 360 ||
         fvalue < 0)
    {
        std::stringstream msg;
        msg << "AngleObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    value_m = ((int)(fvalue * 256 / 360)) % 256;
}

std::string AngleObjectValue::toString()
{
    std::ostringstream out;
    out.precision(4);
    out << (float)value_m * 360 / 256;
    return out.str();
}

Logger& AngleObject::logger_m(Logger::getInstance("AngleObject"));

AngleObject::AngleObject() : AngleObjectValue(0)
{}

AngleObject::~AngleObject()
{}

ObjectValue* AngleObject::createObjectValue(const std::string& value)
{
    return new AngleObjectValue(value);
}

void AngleObject::setValue(const std::string& value)
{
    AngleObjectValue val(value);
    Object::setValue(&val);
}

HeatingModeObjectValue::HeatingModeObjectValue(const std::string& value)
{
    if (value == "comfort")
        value_m = 1;
    else if (value == "standby")
        value_m = 2;
    else if (value == "night")
        value_m = 3;
    else if (value == "frost")
        value_m = 4;
    else if (value == "auto")
        value_m = 0;
    else
    {
        std::stringstream msg;
        msg << "HeatingModeObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string HeatingModeObjectValue::toString()
{
    switch (value_m)
    {
    case 0:
        return "auto";
    case 1:
        return "comfort";
    case 2:
        return "standby";
    case 3:
        return "night";
    case 4:
        return "frost";
    }
    return "frost";
}

Logger& HeatingModeObject::logger_m(Logger::getInstance("HeatingModeObject"));

ObjectValue* HeatingModeObject::createObjectValue(const std::string& value)
{
    return new HeatingModeObjectValue(value);
}

void HeatingModeObject::setValue(const std::string& value)
{
    HeatingModeObjectValue val(value);
    Object::setValue(&val);
}

Latin1CharObjectValue::Latin1CharObjectValue(const std::string& value)
{
	// Transcode passed value to latin1.
	std::string latin1Value = StringObjectValue::transcode(value, StringObjectValue::getUTF8Encoding(), StringObjectValue::getLatin1Encoding());
    unsigned char chvalue;
    std::istringstream val(latin1Value);
    val >> chvalue;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         chvalue > 255 ||
         chvalue < 0)
    {
        std::stringstream msg;
        msg << "Latin1CharObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    value_m = (int)chvalue;
}

std::string Latin1CharObjectValue::toString()
{
    std::ostringstream out;
    out << (unsigned char)value_m;
    return StringObjectValue::transcode(out.str(), StringObjectValue::getLatin1Encoding(), StringObjectValue::getUTF8Encoding());
}

Logger& Latin1CharObject::logger_m(Logger::getInstance("Latin1CharObject"));

Latin1CharObject::Latin1CharObject() : Latin1CharObjectValue(0)
{}

Latin1CharObject::~Latin1CharObject()
{}

ObjectValue* Latin1CharObject::createObjectValue(const std::string& value)
{
    return new Latin1CharObjectValue(value);
}

void Latin1CharObject::setValue(const std::string& value)
{
    Latin1CharObjectValue val(value);
    Object::setValue(&val);
}

AsciiCharObjectValue::AsciiCharObjectValue(const std::string& value)
{
    unsigned char chvalue;
    std::istringstream val(value);
    val >> chvalue;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         chvalue > 127 ||
         chvalue < 0)
    {
        std::stringstream msg;
        msg << "AsciiCharObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    value_m = (int)chvalue;
}

std::string AsciiCharObjectValue::toString()
{
    std::ostringstream out;
    out << (unsigned char)value_m;
    return out.str();
}

Logger& AsciiCharObject::logger_m(Logger::getInstance("AsciiCharObject"));

AsciiCharObject::AsciiCharObject() : AsciiCharObjectValue(0)
{}

AsciiCharObject::~AsciiCharObject()
{}

ObjectValue* AsciiCharObject::createObjectValue(const std::string& value)
{
    return new AsciiCharObjectValue(value);
}

void AsciiCharObject::setValue(const std::string& value)
{
    AsciiCharObjectValue val(value);
    Object::setValue(&val);
}

U16ObjectValue::U16ObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         value_m > 65535 ||
         value_m < 0)
    {
        std::stringstream msg;
        msg << "U16ObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string U16ObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

Logger& U16Object::logger_m(Logger::getInstance("U16Object"));

U16Object::U16Object()
{}

U16Object::~U16Object()
{}

ObjectValue* U16Object::createObjectValue(const std::string& value)
{
    return new U16ObjectValue(value);
}

void U16Object::setValue(const std::string& value)
{
    U16ObjectValue val(value);
    Object::setValue(&val);
}

void U16Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    unsigned int newValue;
    newValue = (buf[2]<<8) | buf[3];
    if (forceUpdate() || newValue != value_m)
    {
        value_m = newValue;
        onUpdate();
    }
}

void U16Object::doSend(bool isWrite)
{
	BufferBuilder builder(4, U16Object::logger_m);
	builder << 0 << (isWrite ? 0x80 : 0x40) << ((value_m & 0xff00)>>8) << (value_m & 0xff);
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 4);
}

U32ObjectValue::U32ObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "U32ObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string U32ObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

Logger& U32Object::logger_m(Logger::getInstance("U32Object"));

U32Object::U32Object()
{}

U32Object::~U32Object()
{}

ObjectValue* U32Object::createObjectValue(const std::string& value)
{
    return new U32ObjectValue(value);
}

void U32Object::setValue(const std::string& value)
{
    U32ObjectValue val(value);
    Object::setValue(&val);
}

void U32Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    unsigned int newValue;
    newValue = (buf[2]<<24) | (buf[3]<<16) | (buf[4]<<8) | buf[5];
    if (forceUpdate() || newValue != value_m)
    {
        value_m = newValue;
        onUpdate();
    }
}

void U32Object::doSend(bool isWrite)
{
	BufferBuilder builder(6, U32Object::logger_m);
    builder << 0 << (isWrite ? 0x80 : 0x40) << ((value_m & 0xff000000)>>24) << ((value_m & 0xff0000)>>16) << ((value_m & 0xff00)>>8) << (value_m & 0xff);
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 6);
}

RGBObjectValue::RGBObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val.setf(std::ios::hex, std::ios::basefield);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "RGBObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string RGBObjectValue::toString()
{
    std::ostringstream out;
    out.setf(std::ios::hex, std::ios::basefield);
    out.fill('0');
    out << std::setw(6) << value_m;
    return out.str();
}

Logger& RGBObject::logger_m(Logger::getInstance("RGBObject"));

RGBObject::RGBObject()
{}

RGBObject::~RGBObject()
{}

ObjectValue* RGBObject::createObjectValue(const std::string& value)
{
    return new RGBObjectValue(value);
}

void RGBObject::setValue(const std::string& value)
{
    RGBObjectValue val(value);
    Object::setValue(&val);
}

void RGBObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    unsigned int newValue;
    newValue = (buf[2]<<16) | (buf[3]<<8) | buf[4];
    if (forceUpdate() || newValue != value_m)
    {
        value_m = newValue;
        onUpdate();
    }
}

void RGBObject::doSend(bool isWrite)
{
	BufferBuilder builder(5, RGBObject::logger_m);
    builder << 0 << (isWrite ? 0x80 : 0x40) << ((value_m & 0xff0000)>>16) << ((value_m & 0xff00)>>8) << (value_m & 0xff);
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 5);
}

#ifdef STL_STREAM_SUPPORT_INT64
RGBWObjectValue::RGBWObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val.setf(std::ios::hex, std::ios::basefield);
    val >> RGBWObjectValue::value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "RGBWObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string RGBWObjectValue::toString()
{
    std::ostringstream out;
    out.setf(std::ios::hex, std::ios::basefield);
    out.fill('0');
    out << std::setw(8) << RGBWObjectValue::value_m;
    return out.str();
}

Logger& RGBWObject::logger_m(Logger::getInstance("RGBWObject"));

RGBWObject::RGBWObject()
{}

RGBWObject::~RGBWObject()
{}

ObjectValue* RGBWObject::createObjectValue(const std::string& value)
{
    return new RGBWObjectValue(value);
}

void RGBWObject::setValue(const std::string& value)
{
    RGBWObjectValue val(value);
    Object::setValue(&val);
}

void RGBWObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    unsigned int newValue;
    newValue = (buf[2]<<24) | (buf[3]<<16) | (buf[4]<<8) | buf[5];
    if (forceUpdate() || newValue != RGBWObjectValue::value_m)
    {
        RGBWObjectValue::value_m = newValue;
        onUpdate();
    }
}

void RGBWObject::doSend(bool isWrite)
{
	BufferBuilder builder(8, RGBWObject::logger_m);
    builder << 0 << (isWrite ? 0x80 : 0x40)
    << ((RGBWObjectValue::value_m & 0xff000000)>>24)
    << ((RGBWObjectValue::value_m & 0xff0000)>>16)
    << ((RGBWObjectValue::value_m & 0xff00)>>8)
    << (RGBWObjectValue::value_m & 0xff)
    << 0x00 << 0x0f;
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 8);
}
#endif

IntObjectValue::IntObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "IntObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string IntObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

double IntObjectValue::toNumber()
{
    return value_m;
}

bool IntObjectValue::equals(ObjectValue* value)
{
    assert(value);
    IntObjectValue* val = dynamic_cast<IntObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "IntObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "IntObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int IntObjectValue::compare(ObjectValue* value)
{
    assert(value);
    IntObjectValue* val = dynamic_cast<IntObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "IntObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "IntObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;

    if (value_m == val->value_m)
        return 0;
    if (value_m > val->value_m)
        return 1;
    else
        return -1;
}

bool IntObjectValue::set(ObjectValue* value)
{
    assert(value);
    IntObjectValue* val = dynamic_cast<IntObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "IntObjectValue: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        if (value_m != val->value_m)
        {
            value_m = val->value_m;
            return true;
        }
    }
    return false;
}

Logger& IntObject::logger_m(Logger::getInstance("IntObject"));

IntObject::IntObject()
{}

IntObject::~IntObject()
{}

void IntObject::setIntValue(int32_t value)
{
    if (setInt(value) || forceUpdate())
        onInternalUpdate();
}

int32_t IntObject::getIntValue()
{
    if (!init_m)
        read();
    return getInt();
}

S8ObjectValue::S8ObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         value_m > 127 ||
         value_m < -128)
    {
        std::stringstream msg;
        msg << "S8ObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string S8ObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

Logger& S8Object::logger_m(Logger::getInstance("S8Object"));

S8Object::S8Object()
{}

S8Object::~S8Object()
{}

ObjectValue* S8Object::createObjectValue(const std::string& value)
{
    return new S8ObjectValue(value);
}

void S8Object::setValue(const std::string& value)
{
    S8ObjectValue val(value);
    Object::setValue(&val);
}

void S8Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int32_t newValue;
    if (len == 2)
        newValue = (buf[1] & 0x3F);
    else
        newValue = buf[2];
    if (newValue > 127)
        newValue -= 256;
    if (forceUpdate() || newValue != value_m)
    {
        value_m = newValue;
        onUpdate();
    }
}

void S8Object::doSend(bool isWrite)
{
	BufferBuilder builder(3, S8Object::logger_m);
    builder << 0 << (isWrite ? 0x80 : 0x40) << (value_m & 0xff);
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 3);
}

S16ObjectValue::S16ObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         value_m > 32767 ||
         value_m < -32768)
    {
        std::stringstream msg;
        msg << "S16ObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string S16ObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

Logger& S16Object::logger_m(Logger::getInstance("S16Object"));

S16Object::S16Object()
{}

S16Object::~S16Object()
{}

ObjectValue* S16Object::createObjectValue(const std::string& value)
{
    return new S16ObjectValue(value);
}

void S16Object::setValue(const std::string& value)
{
    S16ObjectValue val(value);
    Object::setValue(&val);
}

void S16Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int32_t newValue;
    newValue = (buf[2]<<8) | buf[3];
    if (newValue > 32767)
        newValue -= 65536;
    if (forceUpdate() || newValue != value_m)
    {
        value_m = newValue;
        onUpdate();
    }
}

void S16Object::doSend(bool isWrite)
{
	BufferBuilder builder(4, S16Object::logger_m);
    builder << 0 << (isWrite ? 0x80 : 0x40) << ((value_m & 0xff00)>>8) << (value_m & 0xff);
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 4);
}

S32ObjectValue::S32ObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "S32ObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string S32ObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

Logger& S32Object::logger_m(Logger::getInstance("S32Object"));

S32Object::S32Object()
{}

S32Object::~S32Object()
{}

ObjectValue* S32Object::createObjectValue(const std::string& value)
{
    return new S32ObjectValue(value);
}

void S32Object::setValue(const std::string& value)
{
    S32ObjectValue val(value);
    Object::setValue(&val);
}

void S32Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int32_t newValue;
    newValue = (buf[2]<<24) | (buf[3]<<16) | (buf[4]<<8) | buf[5];
    if (forceUpdate() || newValue != value_m)
    {
        value_m = newValue;
        onUpdate();
    }
}

void S32Object::doSend(bool isWrite)
{
	BufferBuilder builder(6, S32Object::logger_m);
    builder << 0 << (isWrite ? 0x80 : 0x40) << ((value_m & 0xff000000)>>24) << ((value_m & 0xff0000)>>16) << ((value_m & 0xff00)>>8) << (value_m & 0xff);
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 6);
}

#ifdef STL_STREAM_SUPPORT_INT64
S64ObjectValue::S64ObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        std::stringstream msg;
        msg << "S64ObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string S64ObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

double S64ObjectValue::toNumber()
{
    return value_m;
}

bool S64ObjectValue::equals(ObjectValue* value)
{
    assert(value);
    S64ObjectValue* val = dynamic_cast<S64ObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "S64ObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "S64ObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int S64ObjectValue::compare(ObjectValue* value)
{
    assert(value);
    S64ObjectValue* val = dynamic_cast<S64ObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "S64ObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "S64ObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;

    if (value_m == val->value_m)
        return 0;
    if (value_m > val->value_m)
        return 1;
    else
        return -1;
}

bool S64ObjectValue::set(ObjectValue* value)
{
    assert(value);
    S64ObjectValue* val = dynamic_cast<S64ObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "S64ObjectValue: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        if (value_m != val->value_m)
        {
            value_m = val->value_m;
            return true;
        }
    }
    return false;
}

Logger& S64Object::logger_m(Logger::getInstance("S64Object"));

S64Object::S64Object()
{}

S64Object::~S64Object()
{}

ObjectValue* S64Object::createObjectValue(const std::string& value)
{
    return new S64ObjectValue(value);
}

void S64Object::setIntValue(int64_t value)
{
    if (setInt(value) || forceUpdate())
        onInternalUpdate();
}

int64_t S64Object::getIntValue()
{
    if (!init_m)
        read();
    return getInt();
}

void S64Object::setValue(const std::string& value)
{
    S64ObjectValue val(value);
    if (set(&val) || forceUpdate())
        onInternalUpdate();
}

void S64Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int64_t newValue;
    newValue = ((int64_t)buf[2]<<56) | ((int64_t)buf[3]<<48) | ((int64_t)buf[4]<<40) | ((int64_t)buf[5]<<32) | (buf[6]<<24) | (buf[7]<<16) | (buf[8]<<8) | buf[9];
    if (forceUpdate() || newValue != value_m)
    {
        value_m = newValue;
        onUpdate();
    }
}

void S64Object::doSend(bool isWrite)
{
	BufferBuilder builder(10, S64Object::logger_m);
    builder << 0 << (isWrite ? 0x80 : 0x40) << ((value_m & 0xff00000000000000LL)>>56) << ((value_m & 0xff000000000000LL)>>48) << ((value_m & 0xff0000000000LL)>>40) << ((value_m & 0xff00000000LL)>>32)
                                                    << ((value_m & 0xff000000LL)>>24) << ((value_m & 0xff0000LL)>>16) << ((value_m & 0xff00LL)>>8) << (value_m & 0xffLL);
    Services::instance()->getKnxConnection()->write(getGad(), builder.getBuffer(), 10);
}
#endif

StringObjectValue::StringObjectValue(const std::string& value)
{
    value_m = value;
//    logger_m.debugStream() << "StringObjectValue: Value: '" << value_m << "'" << endlog;
}

std::string StringObjectValue::toString()
{
    return value_m;
}

double StringObjectValue::toNumber()
{
    std::istringstream val(value_m);
    double value;
    val >> value;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof()) // workaround for wrong val.eof() flag in uClibc++
    {
        value = 0;
    }
    return value;
}

bool StringObjectValue::equals(ObjectValue* value)
{
    assert(value);
    StringObjectValue* val = dynamic_cast<StringObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "StringObjectValue: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    logger_m.infoStream() << "StringObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int StringObjectValue::compare(ObjectValue* value)
{
    assert(value);
    StringObjectValue* val = dynamic_cast<StringObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "StringObjectValue: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return -1;
    }
    logger_m.infoStream() << "StringObjectValue: Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;

    if (value_m == val->value_m)
        return 0;
    if (value_m < val->value_m)
        return -1;
    else
        return 1;
}

bool StringObjectValue::set(ObjectValue* value)
{
    assert(value);
    StringObjectValue* val = dynamic_cast<StringObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "StringObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        if (value_m != val->value_m)
        {
            value_m = val->value_m;
            return true;
        }
    }
    return false;
}

bool StringObjectValue::set(double value)
{
    std::ostringstream out;
    out << value;
    if (value_m != out.str())
    {
        value_m = out.str();
        return true;
    }
    return false;
}

const std::string &StringObjectValue::getLatin1Encoding()
{
	static const std::string encoding("ISO-8859-1"); // As iconv expects it (see https://www.gnu.org/savannah-checkouts/gnu/libiconv/documentation/libiconv-1.13/iconv_open.3.html).
	return encoding;
}

const std::string &StringObjectValue::getUTF8Encoding()
{
	static const std::string encoding("UTF-8"); // As iconv expects it (see https://www.gnu.org/savannah-checkouts/gnu/libiconv/documentation/libiconv-1.13/iconv_open.3.html).
	return encoding;
}

std::string StringObjectValue::transcode(const std::string &source, const std::string &sourceEncoding, const std::string &targetEncoding)
{
	if (source.empty()) return source; // iconv seems to not support empty strings.

	// Ask iconv to use replacement chars that look like the original ones for characters
	// that are unavailable in target encoding.
	iconv_t conversionDescriptor = iconv_open((targetEncoding + "//TRANSLIT").c_str(), sourceEncoding.c_str());
	char cSource[source.size()];
	memcpy(cSource, source.c_str(), source.size() + 1);
	char *sourceStart = &cSource[0];
	size_t sourceLength = source.size();
	const size_t targetLength = source.size() * 5; // Should be pretty enough even in worst cases.
	char targetChars[targetLength];
	char *targetCharPointer = &targetChars[0];
	size_t targetSizeLeft = targetLength;
	size_t res = iconv(conversionDescriptor, &sourceStart, &sourceLength, &targetCharPointer, &targetSizeLeft);
	if(res == static_cast<size_t>(-1))
	{
		logger_m.errorStream() << "Transcoding of " << source << " from " << sourceEncoding << " to " << targetEncoding << " failed." << endlog;
		return source;
	}

	std::string transcodedString(targetChars, targetLength - targetSizeLeft);
	return transcodedString;
}


Logger& StringObject::logger_m(Logger::getInstance("StringObject"));

StringObject::StringObject()
{}

StringObject::~StringObject()
{}

ObjectValue* StringObject::createObjectValue(const std::string& value)
{
    return new StringObjectValue(value);
}

void StringObject::setValue(const std::string& value)
{
    StringObjectValue val(value);
    Object::setValue(&val);
}

void StringObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 2)
    {
        logger_m.errorStream() << "Invalid packet received for StringObject (too short)" << endlog;
        return;
    }
    std::string value;
    for(int j=2; j<len && buf[j]!=0; j++)
        value.push_back(buf[j]);
    StringObjectValue val(value);

    if (set(&val) || forceUpdate())
        onUpdate();
}

void StringObject::doSend(bool isWrite)
{
    logger_m.debugStream() << "StringObject: Value: " << value_m << endlog;
    uint bufsz = value_m.size()+3;
    uint8_t *buf = new uint8_t[bufsz];
    memset(buf,0,bufsz);
    buf[1] = (isWrite ? 0x80 : 0x40);
    // Convert to hex
    for(uint j=2;j<bufsz;j++)
        buf[j] = static_cast<uint8_t>(value_m[j-2]);

    Services::instance()->getKnxConnection()->write(getGad(), buf, bufsz);
    delete buf;
}

void StringObject::setStringValue(const std::string& value)
{
    StringObjectValue val(value);
    Object::setValue(&val);
}

String14ObjectValue::String14ObjectValue(const std::string& value): StringObjectValue(value)
{
    if (transcode(value, getUTF8Encoding(), getLatin1Encoding()).length() > 14)
    {
        std::stringstream msg;
        msg << "String14ObjectValue: Bad value (too long): '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

String14AsciiObjectValue::String14AsciiObjectValue(const std::string& value): StringObjectValue(value)
{
    if ( value.length() > 14)
    {
        std::stringstream msg;
        msg << "String14AsciiObjectValue: Bad value (too long): '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    std::string::const_iterator it = value.begin();
    while ( it != value.end())
    {
        if (*it < 0 || *it > 127)
        {
            std::stringstream msg;
            msg << "String14AsciiObjectValue: Bad value (invalid character): '" << value << "'" << std::endl;
            throw ticpp::Exception(msg.str());
        }
        ++it;
    }
}

Logger& String14Object::logger_m(Logger::getInstance("String14Object"));

String14Object::String14Object()
{}

String14Object::~String14Object()
{}

ObjectValue* String14Object::createObjectValue(const std::string& value)
{
    return new String14ObjectValue(value);
}

void String14Object::setValue(const std::string& value)
{
    String14ObjectValue val(value);
    Object::setValue(&val);
}

void String14Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 2)
    {
        logger_m.errorStream() << "Invalid packet received for String14Object (too short)" << endlog;
        return;
    }
    std::string value;
    for(int j=2; j<len && buf[j]!=0; j++)
        value.push_back(buf[j]);

	value = transcode(value, getLatin1Encoding(), getUTF8Encoding());
    String14ObjectValue val(value);

    if (set(&val) || forceUpdate())
        onUpdate();
}

void String14Object::doSend(bool isWrite)
{
    logger_m.debugStream() << "String14Object: Value: " << value_m << endlog;
    uint8_t buf[16];
    memset(buf,0,sizeof(buf));
    buf[1] = (isWrite ? 0x80 : 0x40);
    // Convert to hex
	std::string latin1Value = transcode(value_m, getUTF8Encoding(), getLatin1Encoding());
    for(uint j=0;j<latin1Value.size();j++)
        buf[j+2] = static_cast<uint8_t>(latin1Value[j]);

    Services::instance()->getKnxConnection()->write(getGad(), buf, sizeof(buf));
}

void String14Object::setStringValue(const std::string& value)
{
    String14ObjectValue val(value);
    Object::setValue(&val);
}

Logger& String14AsciiObject::logger_m(Logger::getInstance("String14AsciiObject"));

String14AsciiObject::String14AsciiObject()
{}

String14AsciiObject::~String14AsciiObject()
{}

ObjectValue* String14AsciiObject::createObjectValue(const std::string& value)
{
    return new String14AsciiObjectValue(value);
}

void String14AsciiObject::setValue(const std::string& value)
{
    String14AsciiObjectValue val(value);
    Object::setValue(&val);
}

void String14AsciiObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 2)
    {
        logger_m.errorStream() << "Invalid packet received for String14AsciiObject (too short)" << endlog;
        return;
    }
    std::string value;
    for(int j=2; j<len && buf[j]!=0; j++)
        value.push_back(buf[j]);
    String14AsciiObjectValue val(value);

    if (set(&val) || forceUpdate())
        onUpdate();
}

void String14AsciiObject::doSend(bool isWrite)
{
    logger_m.debugStream() << "String14AsciiObject: Value: " << value_m << endlog;
    uint8_t buf[16];
    memset(buf,0,sizeof(buf));
    buf[1] = (isWrite ? 0x80 : 0x40);
    // Convert to hex
    for(uint j=0;j<value_m.size();j++)
        buf[j+2] = static_cast<uint8_t>(value_m[j]);

    Services::instance()->getKnxConnection()->write(getGad(), buf, sizeof(buf));
}

void String14AsciiObject::setStringValue(const std::string& value)
{
    String14AsciiObjectValue val(value);
    Object::setValue(&val);
}

Logger& ObjectController::logger_m(Logger::getInstance("ObjectController"));

ObjectController::ObjectController()
{}

ObjectController::~ObjectController()
{
    ObjectIdMap_t::iterator it;
    for (it = objectIdMap_m.begin(); it != objectIdMap_m.end(); it++)
        delete (*it).second;
}

ObjectController* ObjectController::instance()
{
    if (instance_m == 0)
        instance_m = new ObjectController();
    return instance_m;
}

void ObjectController::onWrite(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    std::pair<ObjectMap_t::iterator, ObjectMap_t::iterator> range;
    range = objectMap_m.equal_range(dest);
    ObjectMap_t::iterator it;
    for (it = range.first; it != range.second; it++)
        (*it).second->onWrite(buf, len, src);
    if (range.first == range.second)
        logger_m.debugStream() << "onWrite - dest eibaddr not found: "
            << Object::WriteGroupAddr(dest)
            << " sender=" << Object::WriteAddr( src ) << endlog;
}

void ObjectController::onRead(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    std::pair<ObjectMap_t::iterator, ObjectMap_t::iterator> range;
    range = objectMap_m.equal_range(dest);
    ObjectMap_t::iterator it;
    for (it = range.first; it != range.second; it++)
        (*it).second->onRead(buf, len, src);
    if (range.first == range.second)
        logger_m.debugStream() << "onRead - dest eibaddr not found: "
            << Object::WriteGroupAddr(dest)
            << " sender=" << Object::WriteAddr( src ) << endlog;
}

void ObjectController::onResponse(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    std::pair<ObjectMap_t::iterator, ObjectMap_t::iterator> range;
    range = objectMap_m.equal_range(dest);
    ObjectMap_t::iterator it;
    for (it = range.first; it != range.second; it++)
        (*it).second->onResponse(buf, len, src);
    if (range.first == range.second)
        logger_m.debugStream() << "onResponse - dest eibaddr not found: "
            << Object::WriteGroupAddr(dest)
            << " sender=" << Object::WriteAddr( src ) << endlog;
}

Object* ObjectController::getObject(const std::string& id)
{
    ObjectIdMap_t::iterator it = objectIdMap_m.find(id);
    if (it == objectIdMap_m.end())
    {
        std::stringstream msg;
        msg << "ObjectController: Object ID not found: '" << id << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    it->second->incRefCount();
    return (*it).second;
}

void ObjectController::addObject(Object* object)
{
    if (!objectIdMap_m.insert(ObjectIdPair_t(object->getID(), object)).second)
        throw ticpp::Exception("Object ID already exists");
    if (object->getGad())
        objectMap_m.insert(ObjectPair_t(object->getGad(), object));
    std::list<eibaddr_t>::iterator it2, it_end;
    it_end = object->getListenerGadEnd();
    for (it2=object->getListenerGad(); it2!=it_end; it2++)
        objectMap_m.insert(ObjectPair_t((*it2), object));
}

void ObjectController::removeObjectFromAddressMap(eibaddr_t gad, Object* object)
{
    if (gad == 0)
        return;
    std::pair<ObjectMap_t::iterator, ObjectMap_t::iterator> range =
        objectMap_m.equal_range(gad);
    ObjectMap_t::iterator it = range.first;
    while(it != range.second) {
        if ((*it).second == object)
            objectMap_m.erase(it++);
        else
            ++it;
    }
}

void ObjectController::removeObject(Object* object)
{
    ObjectIdMap_t::iterator it = objectIdMap_m.find(object->getID());
    if (it != objectIdMap_m.end())
    {
        eibaddr_t gad = it->second->getGad();
        removeObjectFromAddressMap(gad, object);

        std::list<eibaddr_t>::iterator it2, it_end;
        it_end = object->getListenerGadEnd();
        for (it2=object->getListenerGad(); it2!=it_end; it2++)
            removeObjectFromAddressMap((*it2), object);

        if (it->second->inUse())
            throw ticpp::Exception("Delete failed! Object still in use.");
        delete it->second;
        objectIdMap_m.erase(it);
    }
}

void ObjectController::importXml(ticpp::Element* pConfig)
{
    ticpp::Iterator< ticpp::Element > child("object");
    for ( child = pConfig->FirstChildElement("object", false); child != child.end(); child++ )
    {
        std::string id = child->GetAttribute("id");
        bool del = child->GetAttribute("delete") == "true";
        ObjectIdMap_t::iterator it = objectIdMap_m.find(id);
        if (it != objectIdMap_m.end())
        {
            Object* object = it->second;

            removeObjectFromAddressMap(object->getGad(), object);
            std::list<eibaddr_t>::iterator it2, it_end;
            it_end = object->getListenerGadEnd();
            for (it2=object->getListenerGad(); it2!=it_end; it2++)
                removeObjectFromAddressMap((*it2), object);

            if (del)
            {
                if (object->inUse())
                    throw ticpp::Exception("Delete failed! Object still in use.");
                delete object;
                objectIdMap_m.erase(it);
            }
            else
            {
                object->importXml(&(*child));
                if (object->getGad())
                    objectMap_m.insert(ObjectPair_t(object->getGad(), object));
                std::list<eibaddr_t>::iterator it2, it_end;
                it_end = object->getListenerGadEnd();
                for (it2=object->getListenerGad(); it2!=it_end; it2++)
                    objectMap_m.insert(ObjectPair_t((*it2), object));
                objectIdMap_m.insert(ObjectIdPair_t(id, object));
            }
        }
        else
        {
            if (del)
                throw ticpp::Exception("Object not found");
            Object* object = Object::create(&(*child));
            if (object->getGad())
                objectMap_m.insert(ObjectPair_t(object->getGad(), object));
            std::list<eibaddr_t>::iterator it2, it_end;
            it_end = object->getListenerGadEnd();
            for (it2=object->getListenerGad(); it2!=it_end; it2++)
                objectMap_m.insert(ObjectPair_t((*it2), object));
            objectIdMap_m.insert(ObjectIdPair_t(id, object));
        }
    }

}

void ObjectController::exportXml(ticpp::Element* pConfig)
{
    ObjectIdMap_t::iterator it;
    for (it = objectIdMap_m.begin(); it != objectIdMap_m.end(); it++)
    {
        ticpp::Element pElem("object");
        (*it).second->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

void ObjectController::exportObjectValues(ticpp::Element* pObjects)
{
    ObjectIdMap_t::iterator it;
    for (it = objectIdMap_m.begin(); it != objectIdMap_m.end(); it++)
    {
        ticpp::Element pElem("object");
        pElem.SetAttribute("id", (*it).second->getID());
        pElem.SetAttribute("value", (*it).second->getValue());
        pObjects->LinkEndChild(&pElem);
    }
}

// Delivers all objects
std::list<Object*> ObjectController::getObjects()
{
    std::list<Object*> objects;
    ObjectIdMap_t::iterator it;
    for (it = objectIdMap_m.begin(); it != objectIdMap_m.end(); it++)
    {
      it->second->incRefCount();
      objects.push_back((*it).second);
    }
    return objects;
}

Object::BufferBuilder::BufferBuilder(int size, Logger &logger)
 : buffer_m(NULL), index_m(0), size_m(size), logger_m(logger)
{
	buffer_m = new uint8_t[size];
}

Object::BufferBuilder::~BufferBuilder()
{
	delete[] buffer_m;
}
