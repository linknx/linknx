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
extern "C"
{
#include "common.h"
}

ObjectController* ObjectController::instance_m;

Object::Object() : gad_m(0), init_m(false), readPending_m(false), flags_m(Default)
{}

Object::~Object()
{}

Object* Object::create(const std::string& type)
{
    if (type == "" || type == "EIS1")
        return new SwitchingObject();
    else if (type == "EIS2")
        return new DimmingObject();
    else if (type == "EIS3")
        return new TimeObject();
    else if (type == "EIS4")
        return new DateObject();
    else if (type == "EIS5")
        return new ValueObject();
    else if (type == "EIS6")
        return new ScalingObject();
    else if (type == "heat-mode")
        return new HeatingModeObject();
    else if (type == "EIS15")
        return new StringObject();
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

void Object::importXml(ticpp::Element* pConfig)
{
    std::string id = pConfig->GetAttribute("id");
    if (id == "")
        throw ticpp::Exception("Missing or empty object ID");
    if (id_m == "")
        id_m = id;

    std::string gad = pConfig->GetAttributeOrDefault("gad", "nochange");
    // set default value to "nochange" just to see if the attribute was present or not in xml
    if (gad == "")
        gad_m = 0;
    else if (gad != "nochange")
        gad_m = readgaddr(gad.c_str());

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
            listenerGadList_m.push_back(readgaddr(listener_gad.c_str()));
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
        if (flags.find('f') != flags.npos)
            flags_m |= Force;
    }
    catch( ticpp::Exception& ex )
    {
        flags_m = Default;
    }

    // BEGIN: backward compatibility with 0.0.1.17
    if (pConfig->GetAttribute("forcewrite") == "true")
        flags_m |= Force;
    // END: backward compatibility with 0.0.1.17

    // TODO: do we need to use the 'i' flag instead of init="request" attribute
    initValue_m = pConfig->GetAttribute("init");
    if (initValue_m == "persist")
    {
        PersistentStorage *persistence = Services::instance()->getPersistentStorage();
        if (persistence)
        {
            initValue_m = ""; // avoid setValue() to immediately write back what we read
            std::string val = persistence->read(id_m);
            if (val != "")
                setValue(val);
            initValue_m = "persist";
        }
        else
        {
            std::stringstream msg;
            msg << "Unable to persist object '" << id_m << "'; PersistentStorage not configured" << std::endl;
            throw ticpp::Exception(msg.str());
        }
    }
    else if (initValue_m != "" && initValue_m != "request")
        setValue(initValue_m);

    std::cout << "Configured object '" << id_m << "': gad='" << gad_m << "'" << std::endl;
}

void Object::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("id", id_m);

    if (gad_m != 0)
        pConfig->SetAttribute("gad", writegaddr(gad_m));

    if (initValue_m != "")
        pConfig->SetAttribute("init", initValue_m);

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
        if (flags_m & Force)
            flags << 'f';
        pConfig->SetAttribute("flags", flags.str());
    }

    if (descr_m != "")
        pConfig->SetText(descr_m);

    ListenerGadList_t::iterator it;
    for (it = listenerGadList_m.begin(); it != listenerGadList_m.end(); it++)
    {
        ticpp::Element pElem("listener");
        pElem.SetAttribute("gad", writegaddr(*it));
        pConfig->LinkEndChild(&pElem);
    }
}

void Object::read()
{
    KnxConnection* con = Services::instance()->getKnxConnection();
    if (!readPending_m)
    {
        uint8_t buf[2] = { 0, 0 };
        con->write(getGad(), buf, 2);
    }
    readPending_m = true;

    int cnt = 0;
    while (cnt < 100 && readPending_m)
    {
        if (con->isRunning())
            con->checkInput();
        else
            pth_usleep(10000);
        ++cnt;
    }
}

void Object::onUpdate()
{
    ListenerList_t::iterator it;
    for (it = listenerList_m.begin(); it != listenerList_m.end(); it++)
    {
        // std::cout << "Calling onChange on listener for " << id_m << std::endl;
        (*it)->onChange(this);
    }
    if (initValue_m == "persist")
    {
        PersistentStorage *persistence = Services::instance()->getPersistentStorage();
        if (persistence)
            persistence->write(id_m, getValue());
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
    std::cout << "Adding listener to object '" << id_m << "'" << std::endl;
    listenerList_m.push_back(listener);
}
void Object::removeChangeListener(ChangeListener* listener)
{
    listenerList_m.remove(listener);
}

SwitchingObjectValue::SwitchingObjectValue(const std::string& value)
{
    if (value == "1" || value == "on" || value == "true")
        value_m = true;
    else if (value == "0" || value == "off" || value == "false")
        value_m = false;
    else
    {
        std::stringstream msg;
        msg << "SwitchingObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string SwitchingObjectValue::toString()
{
    return value_m ? "on" : "off";
}

DimmingObjectValue::DimmingObjectValue(const std::string& value)
{
    std::string dir;
    int pos = value.find(":");
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

TimeObjectValue::TimeObjectValue(const std::string& value) : hour_m(-1), min_m(-1), sec_m(-1), wday_m(-1)
{
    if (value == "now")
        return;
    std::istringstream val(value);
    char s1, s2;
    val >> hour_m >> s1 >> min_m >> s2 >> sec_m;
    wday_m = 0;

    if ( val.fail() || !val.eof() || s1 != ':' || s2 != ':' || hour_m < 0 || hour_m > 23  || min_m < 0 || min_m > 59 || sec_m < 0 || sec_m > 59 )
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
    out << hour_m << ":" << min_m << ":" << sec_m;
    return out.str();
}

DateObjectValue::DateObjectValue(const std::string& value) : year_m(-1), month_m(-1), day_m(-1)
{
    if (value == "now")
        return;
    std::istringstream val(value);
    char s1, s2;
    val >> year_m >> s1 >> month_m >> s2 >> day_m;
    year_m -= 1900;
    if ( val.fail() || !val.eof() || s1 != '-' || s2 != '-' || year_m < 0 || year_m > 255 || month_m < 1 || month_m > 12 || day_m < 1 || day_m > 31)
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

ValueObjectValue::ValueObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() || !val.eof() || value_m > 670760.96 || value_m < -671088.64)
    {
        std::stringstream msg;
        msg << "ValueObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string ValueObjectValue::toString()
{
    std::ostringstream out;
    out.precision(8);
    out << value_m;
    return out.str();
}

ScalingObjectValue::ScalingObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() || !val.eof() || value_m > 255 || value_m < 0)
    {
        std::stringstream msg;
        msg << "ScalingObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string ScalingObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
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

StringObjectValue::StringObjectValue(const std::string& value)
{
    if ( value.length() > 14)
    {
        std::stringstream msg;
        msg << "StringObjectValue: Bad value (too long): '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    std::string::const_iterator it = value.begin();
    while ( it != value.end())
    {
        if (*it < 0)
        {
            std::stringstream msg;
            msg << "StringObjectValue: Bad value (invalid character): '" << value << "'" << std::endl;
            throw ticpp::Exception(msg.str());
        }
        ++it;
    }
    value_m = value;
    std::cout << "StringObjectValue: Value: '" << value_m << "'" << std::endl;
}

std::string StringObjectValue::toString()
{
    return value_m;
}

SwitchingObject::SwitchingObject() : value_m(false)
{}

SwitchingObject::~SwitchingObject()
{}

ObjectValue* SwitchingObject::createObjectValue(const std::string& value)
{
    return new SwitchingObjectValue(value);
}

bool SwitchingObject::equals(ObjectValue* value)
{
    assert(value);
    SwitchingObjectValue* val = dynamic_cast<SwitchingObjectValue*>(value);
    if (val == 0)
    {
        std::cout << "SwitchingObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
        return false;
    }
    if (!init_m)
        read();
    std::cout << "SwitchingObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << std::endl;
    return value_m == val->value_m;
}

void SwitchingObject::setValue(ObjectValue* value)
{
    assert(value);
    SwitchingObjectValue* val = dynamic_cast<SwitchingObjectValue*>(value);
    if (val == 0)
        std::cout << "SwitchingObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
    setBoolValue(val->value_m);
}

void SwitchingObject::setValue(const std::string& value)
{
    SwitchingObjectValue val(value);
    setBoolValue(val.value_m);
}

std::string SwitchingObject::getValue()
{
    return SwitchingObjectValue(getBoolValue()).toString();
}

void SwitchingObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    bool newValue;
    if (len == 2)
        newValue = (buf[1] & 0x3F) != 0;
    else
        newValue = buf[2] != 0;
    if (!init_m || newValue != value_m)
    {
        std::cout << "New value " << newValue << " for switching object " << getID() << std::endl;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void SwitchingObject::doSend(bool isWrite)
{
    uint8_t buf[2] = { 0, (isWrite ? 0x80 : 0x40) | (value_m ? 1 : 0) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 2);
}

void SwitchingObject::setBoolValue(bool value)
{
    if (!init_m || value != value_m || (flags_m & Force))
    {
        value_m = value;
        if ((flags_m & Transmit) && (flags_m & Comm))
            doSend(true);
        init_m = true;
        onUpdate();
    }
}

DimmingObject::DimmingObject() : direction_m(0), stepcode_m(0)
{}

DimmingObject::~DimmingObject()
{}

void DimmingObject::exportXml(ticpp::Element* pConfig)
{
    Object::exportXml(pConfig);
    pConfig->SetAttribute("type", "EIS2");
}

ObjectValue* DimmingObject::createObjectValue(const std::string& value)
{
    return new DimmingObjectValue(value);
}

bool DimmingObject::equals(ObjectValue* value)
{
    assert(value);
    DimmingObjectValue* val = dynamic_cast<DimmingObjectValue*>(value);
    if (val == 0)
    {
        std::cout << "DimmingObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
        return false;
    }
    if (!init_m)
        read();
    std::cout << "DimmingObject (id=" << getID() << "): Compare object='"
    << (direction_m ? "up" : "down") << ":" << stepcode_m << "' to value='"
    << (val->direction_m ? "up" : "down") << ":" << val->stepcode_m << "'" << std::endl;
    return (direction_m == val->direction_m) && (stepcode_m == val->stepcode_m);
}

void DimmingObject::setValue(ObjectValue* value)
{
    assert(value);
    DimmingObjectValue* val = dynamic_cast<DimmingObjectValue*>(value);
    if (val == 0)
        std::cout << "DimmingObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
    setDimmerValue(val->direction_m, val->stepcode_m);
}

void DimmingObject::setValue(const std::string& value)
{
    DimmingObjectValue val(value);
    setDimmerValue(val.direction_m, val.stepcode_m);
}

std::string DimmingObject::getValue()
{
    if (!init_m)
        read();
    return DimmingObjectValue(direction_m, stepcode_m).toString();
}

void DimmingObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int newValue;
    if (len == 2)
        newValue = (buf[1] & 0x3F);
    else
        newValue = buf[2];
    int direction = newValue & 0x08;
    int stepcode = newValue & 0x07;
    if (stepcode == 0)
        direction = 0;

    if (!init_m || stepcode != stepcode_m  || direction != direction_m)
    {
        std::cout << "New value " << (direction ? "up" : "down") << ":" << stepcode << " for dimming object " << getID() << std::endl;
        stepcode_m = stepcode;
        direction_m = direction;
        init_m = true;
        onUpdate();
    }
}

void DimmingObject::doSend(bool isWrite)
{
    uint8_t buf[2] = { 0, (isWrite ? 0x80 : 0x40) | (direction_m ? 8 : 0) | (stepcode_m & 0x07) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 2);
}

void DimmingObject::setDimmerValue(int direction, int stepcode)
{
    if (!init_m || stepcode_m != stepcode  || direction_m != direction || (flags_m & Force))
    {
        stepcode_m = stepcode;
        direction_m = direction;
        if ((flags_m & Transmit) && (flags_m & Comm))
            doSend(true);
        init_m = true;
        onUpdate();
    }
}

TimeObject::TimeObject() : wday_m(0), hour_m(0), min_m(0), sec_m(0)
{}

TimeObject::~TimeObject()
{}

void TimeObject::exportXml(ticpp::Element* pConfig)
{
    Object::exportXml(pConfig);
    pConfig->SetAttribute("type", "EIS3");
}

ObjectValue* TimeObject::createObjectValue(const std::string& value)
{
    return new TimeObjectValue(value);
}

bool TimeObject::equals(ObjectValue* value)
{
    assert(value);
    TimeObjectValue* val = dynamic_cast<TimeObjectValue*>(value);
    if (val == 0)
    {
        std::cout << "TimeObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
        return false;
    }
    if (!init_m)
        read();
    //    std::cout << "TimeObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << std::endl;
    return (sec_m == val->sec_m) && (min_m == val->min_m) && (hour_m == val->hour_m) && (wday_m == val->wday_m);
}

void TimeObject::setValue(ObjectValue* value)
{
    assert(value);
    TimeObjectValue* val = dynamic_cast<TimeObjectValue*>(value);
    if (val == 0)
        std::cout << "TimeObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
    if (val->hour_m == -1)
        setTime(time(0));
    else
        setTime(val->wday_m, val->hour_m, val->min_m, val->sec_m);
}

void TimeObject::setValue(const std::string& value)
{
    TimeObjectValue val(value);
    setTime(val.wday_m, val.hour_m, val.min_m, val.sec_m);
}

std::string TimeObject::getValue()
{
    if (!init_m)
        read();
    return TimeObjectValue(wday_m, hour_m, min_m, sec_m).toString();
}

void TimeObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 5)
    {
        std::cout << "Invlalid packet received for TimeObject (too short)" << std::endl;
        return;
    }
    int wday, hour, min, sec;

    wday = (buf[2] & 0xE0) >> 5;
    hour = buf[2] & 0x1F;
    min = buf[3];
    sec = buf[4];
    if (!init_m || wday != wday_m || hour != hour_m || min != min_m || sec != sec_m)
    {
        std::cout << "New value " << wday << " " << hour << ":" << min << ":" << sec << " for time object " << getID() << std::endl;
        wday_m = wday;
        hour_m = hour;
        min_m = min;
        sec_m = sec;
        init_m = true;
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
    uint8_t buf[5] = { 0, (isWrite ? 0x80 : 0x40), ((wday_m<<5) & 0xE0) | (hour_m & 0x1F), min_m, sec_m };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 5);
}

void TimeObject::setTime(int wday, int hour, int min, int sec)
{
    if (!init_m ||
            wday_m != wday ||
            hour_m != hour ||
            min_m != min ||
            sec_m != sec ||
            (flags_m & Force))
    {
        std::cout << "TimeObject: setTime "
        << wday << " "
        << hour << ":"
        << min << ":"
        << sec << std::endl;
        wday_m = wday;
        hour_m = hour;
        min_m = min;
        sec_m = sec;

        if ((flags_m & Transmit) && (flags_m & Comm))
            doSend(true);
        init_m = true;
        onUpdate();
    }
}

void TimeObject::getTime(int *wday, int *hour, int *min, int *sec)
{
    *wday = wday_m;
    *hour = hour_m;
    *min = min_m;
    *sec = sec_m;
}

DateObject::DateObject() : day_m(0), month_m(0), year_m(0)
{}

DateObject::~DateObject()
{}

void DateObject::exportXml(ticpp::Element* pConfig)
{
    Object::exportXml(pConfig);
    pConfig->SetAttribute("type", "EIS4");
}

ObjectValue* DateObject::createObjectValue(const std::string& value)
{
    return new DateObjectValue(value);
}

bool DateObject::equals(ObjectValue* value)
{
    assert(value);
    DateObjectValue* val = dynamic_cast<DateObjectValue*>(value);
    if (val == 0)
    {
        std::cout << "DateObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
        return false;
    }
    if (!init_m)
        read();
    //    std::cout << "DateObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << std::endl;
    return (day_m == val->day_m) && (month_m == val->month_m) && (year_m == val->year_m);
}

void DateObject::setValue(ObjectValue* value)
{
    assert(value);
    DateObjectValue* val = dynamic_cast<DateObjectValue*>(value);
    if (val == 0)
        std::cout << "DateObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
    if (val->day_m == -1)
        setDate(time(0));
    else
        setDate(val->day_m, val->month_m, val->year_m);
}

void DateObject::setValue(const std::string& value)
{
    DateObjectValue val(value);
    setDate(val.day_m, val.month_m, val.year_m);
}

std::string DateObject::getValue()
{
    if (!init_m)
        read();
    return DateObjectValue(day_m, month_m, year_m).toString();
}

void DateObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 5)
    {
        std::cout << "Invlalid packet received for DateObject (too short)" << std::endl;
        return;
    }
    int day, month, year;

    day = buf[2];
    month = buf[3];
    year = buf[4];
    if (year < 90)
        year += 100;
    if (!init_m || day != day_m || month != month_m || year != year_m)
    {
        std::cout << "New value " << year+1900 << "-" << month << "-" << day << " for date object " << getID() << std::endl;
        day_m = day;
        month_m = month;
        year_m = year;
        init_m = true;
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
    uint8_t buf[5] = { 0, (isWrite ? 0x80 : 0x40), day_m, month_m, year_m };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 5);
}

void DateObject::setDate(int day, int month, int year)
{
    if (year >= 1900)
        year -= 1900;
    if (!init_m ||
            day_m != day ||
            month_m != month ||
            year_m != year ||
            (flags_m & Force))
    {
        std::cout << "DateObject: setDate "
        << year + 1900 << "-"
        << month << "-"
        << day << std::endl;
        day_m = day;
        month_m = month;
        year_m = year;

        if ((flags_m & Transmit) && (flags_m & Comm))
            doSend(true);
        init_m = true;
        onUpdate();
    }
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

ValueObject::ValueObject() : value_m(0)
{}

ValueObject::~ValueObject()
{}

void ValueObject::exportXml(ticpp::Element* pConfig)
{
    Object::exportXml(pConfig);
    pConfig->SetAttribute("type", "EIS5");
}

ObjectValue* ValueObject::createObjectValue(const std::string& value)
{
    return new ValueObjectValue(value);
}

bool ValueObject::equals(ObjectValue* value)
{
    assert(value);
    ValueObjectValue* val = dynamic_cast<ValueObjectValue*>(value);
    if (val == 0)
    {
        std::cout << "ValueObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
        return false;
    }
    if (!init_m)
        read();
    std::cout << "ValueObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << std::endl;
    return value_m == val->value_m;
}

void ValueObject::setValue(ObjectValue* value)
{
    assert(value);
    ValueObjectValue* val = dynamic_cast<ValueObjectValue*>(value);
    if (val == 0)
        std::cout << "ValueObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
    setFloatValue(val->value_m);
}

void ValueObject::setValue(const std::string& value)
{
    ValueObjectValue val(value);
    setFloatValue(val.value_m);
}

std::string ValueObject::getValue()
{
    return ValueObjectValue(getFloatValue()).toString();
}

void ValueObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 4)
    {
        std::cout << "Invlalid packet received for ValueObject (too short)" << std::endl;
        return;
    }
    double newValue;
    int d1 = ((unsigned char) buf[2]) * 256 + (unsigned char) buf[3];
    int m = d1 & 0x7ff;
    if (d1 & 0x8000)
        m |= ~0x7ff;
    int ex = (d1 & 0x7800) >> 11;
    newValue = ((double)m * (1 << ex) / 100);
    if (!init_m || newValue != value_m)
    {
        std::cout << "New value " << newValue << " for value object " << getID() << std::endl;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void ValueObject::doSend(bool isWrite)
{
    uint8_t buf[4] = { 0, (isWrite ? 0x80 : 0x40), 0, 0 };
    int ex = 0;
    int m = (int)rint(value_m * 100);
    if (m < 0)
    {
        m = -m;
        while (m > 2048)
        {
            m = m >> 1;
            ex++;
        }
        m = -m;
        buf[2] = ((m >> 8) & 0x07) | ((ex << 3) & 0x78) | (1 << 7);
    }
    else
    {
        while (m > 2047)
        {
            m = m >> 1;
            ex++;
        }
        buf[2] = ((m >> 8) & 0x07) | ((ex << 3) & 0x78);
    }
    buf[3] = (m & 0xff);

    Services::instance()->getKnxConnection()->write(getGad(), buf, 4);
}

void ValueObject::setFloatValue(double value)
{
    if (!init_m || value != value_m || (flags_m & Force))
    {
        value_m = value;
        if ((flags_m & Transmit) && (flags_m & Comm))
            doSend(true);
        init_m = true;
        onUpdate();
    }
}

ScalingObject::ScalingObject() : value_m(0)
{}

ScalingObject::~ScalingObject()
{}

void ScalingObject::exportXml(ticpp::Element* pConfig)
{
    Object::exportXml(pConfig);
    pConfig->SetAttribute("type", "EIS6");
}

ObjectValue* ScalingObject::createObjectValue(const std::string& value)
{
    return new ScalingObjectValue(value);
}

bool ScalingObject::equals(ObjectValue* value)
{
    assert(value);
    ScalingObjectValue* val = dynamic_cast<ScalingObjectValue*>(value);
    if (val == 0)
    {
        std::cout << "ScalingObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
        return false;
    }
    if (!init_m)
        read();
    std::cout << "ScalingObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << std::endl;
    return value_m == val->value_m;
}

void ScalingObject::setValue(ObjectValue* value)
{
    assert(value);
    ScalingObjectValue* val = dynamic_cast<ScalingObjectValue*>(value);
    if (val == 0)
        std::cout << "ScalingObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
    setIntValue(val->value_m);
}

void ScalingObject::setValue(const std::string& value)
{
    ScalingObjectValue val(value);
    setIntValue(val.value_m);
}

std::string ScalingObject::getValue()
{
    return ScalingObjectValue(getIntValue()).toString();
}

void ScalingObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int newValue;
    if (len == 2)
        newValue = (buf[1] & 0x3F);
    else
        newValue = buf[2];
    if (!init_m || newValue != value_m)
    {
        std::cout << "New value " << newValue << " for scaling object " << getID() << std::endl;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void ScalingObject::doSend(bool isWrite)
{
    uint8_t buf[3] = { 0, (isWrite ? 0x80 : 0x40), (value_m & 0xff) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 3);
}

void ScalingObject::setIntValue(int value)
{
    if (!init_m || value != value_m || (flags_m & Force))
    {
        value_m = value;
        if ((flags_m & Transmit) && (flags_m & Comm))
            doSend(true);
        init_m = true;
        onUpdate();
    }
}

void HeatingModeObject::exportXml(ticpp::Element* pConfig)
{
    Object::exportXml(pConfig);
    pConfig->SetAttribute("type", "heat-mode");
}

ObjectValue* HeatingModeObject::createObjectValue(const std::string& value)
{
    return new HeatingModeObjectValue(value);
}

void HeatingModeObject::setValue(const std::string& value)
{
    HeatingModeObjectValue val(value);
    setIntValue(val.value_m);
}

std::string HeatingModeObject::getValue()
{
    return HeatingModeObjectValue(getIntValue()).toString();
}

StringObject::StringObject()
{}

StringObject::~StringObject()
{}

void StringObject::exportXml(ticpp::Element* pConfig)
{
    Object::exportXml(pConfig);
    pConfig->SetAttribute("type", "EIS15");
}

ObjectValue* StringObject::createObjectValue(const std::string& value)
{
    return new StringObjectValue(value);
}

bool StringObject::equals(ObjectValue* value)
{
    assert(value);
    StringObjectValue* val = dynamic_cast<StringObjectValue*>(value);
    if (val == 0)
    {
        std::cout << "StringObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
        return false;
    }
    if (!init_m)
        read();
    std::cout << "StringObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << std::endl;
    return value_m == val->value_m;
}

void StringObject::setValue(ObjectValue* value)
{
    assert(value);
    StringObjectValue* val = dynamic_cast<StringObjectValue*>(value);
    if (val == 0)
        std::cout << "StringObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << std::endl;
    setStringValue(val->value_m);
}

void StringObject::setValue(const std::string& value)
{
    StringObjectValue val(value);
    setStringValue(val.value_m);
}

std::string StringObject::getValue()
{
    return value_m;
}

void StringObject::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    if (len < 2)
    {
        std::cout << "Invlalid packet received for StringObject (too short)" << std::endl;
        return;
    }
    std::string value;
    for(int j=2; j<len && buf[j]!=0; j++)
        value.push_back(buf[j]);

    if (!init_m || value != value_m)
    {
        std::cout << "New value " << value << " for string object " << getID() << std::endl;
        value_m = value;
        init_m = true;
        onUpdate();
    }
}

void StringObject::doSend(bool isWrite)
{
    std::cout << "StringObject: Value: " << value_m << std::endl;
    uint8_t buf[16];
    memset(buf,0,sizeof(buf));
    buf[1] = (isWrite ? 0x80 : 0x40);
    // Convert to hex
    for(int j=0;j<value_m.size();j++)
        buf[j+2] = static_cast<uint8_t>(value_m[j]);

    Services::instance()->getKnxConnection()->write(getGad(), buf, sizeof(buf));
}

void StringObject::setStringValue(const std::string& value)
{
    if (!init_m || value != value_m || (flags_m & Force))
    {
        value_m = value;
        if ((flags_m & Transmit) && (flags_m & Comm))
            doSend(true);
        init_m = true;
        onUpdate();
    }
}

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
}

void ObjectController::onRead(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    std::pair<ObjectMap_t::iterator, ObjectMap_t::iterator> range;
    range = objectMap_m.equal_range(dest);
    ObjectMap_t::iterator it;
    for (it = range.first; it != range.second; it++)
        (*it).second->onRead(buf, len, src);
}

void ObjectController::onResponse(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    std::pair<ObjectMap_t::iterator, ObjectMap_t::iterator> range;
    range = objectMap_m.equal_range(dest);
    ObjectMap_t::iterator it;
    for (it = range.first; it != range.second; it++)
        (*it).second->onResponse(buf, len, src);
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
    ObjectMap_t::iterator it;
    for (it = range.first; it != range.second; it++)
        if ((*it).second == object)
            objectMap_m.erase(it);
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
