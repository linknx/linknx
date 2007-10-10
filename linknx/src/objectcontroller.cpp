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

Object::Object() : gad_m(0), init_m(false), readPending_m(false)
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

    forcewrite_m = (pConfig->GetAttribute("forcewrite") == "true");

    initValue_m = pConfig->GetAttribute("init");
    if (initValue_m == "persist")
    {
        std::string val = PersistentStorage::read(id_m);
        if (val != "")
            setValue(val);
    }
    else if (initValue_m != "" && initValue_m != "request")
        setValue(initValue_m);

    descr_m = pConfig->GetText(false);
    std::cout << "Configured object '" << id_m << "': gad='" << gad_m << "'" << std::endl;
}

void Object::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("id", id_m);

    if (gad_m != 0)
        pConfig->SetAttribute("gad", writegaddr(gad_m));

    if (initValue_m != "")
        pConfig->SetAttribute("init", initValue_m);

    if (forcewrite_m)
        pConfig->SetAttribute("forcewrite", "true");

    if (descr_m != "")
        pConfig->SetText(descr_m);
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
        PersistentStorage::write(id_m, getValue());
    }
}

void Object::onWrite(const uint8_t* buf, int len)
{
    readPending_m = false;
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

    if ( val.fail() || s1 != ':' || s2 != ':' )
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

    if ( val.fail() || s1 != '-' || s2 != '-' )
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
    out << year_m << "-" << month_m << "-" << day_m;
    return out.str();
}

ValueObjectValue::ValueObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() )
    {
        std::stringstream msg;
        msg << "ValueObjectValue: Bad value: '" << value << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

std::string ValueObjectValue::toString()
{
    std::ostringstream out;
    out << value_m;
    return out.str();
}

ScalingObjectValue::ScalingObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() )
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

void SwitchingObject::onWrite(const uint8_t* buf, int len)
{
    bool newValue;
    Object::onWrite(buf, len);
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

void SwitchingObject::setBoolValue(bool value)
{
    if (!init_m || value != value_m || forcewrite_m)
    {
        value_m = value;
        uint8_t buf[3] = { 0, 0x80 };
        buf[1] = value ? 0x81 : 0x80;
        Services::instance()->getKnxConnection()->write(getGad(), buf, 2);
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
//    DimmingObjectValue val;
    if (!init_m)
        read();
    return DimmingObjectValue(direction_m, stepcode_m).toString();
//    val.direction_m = direction_m;
//    val.stepcode_m = stepcode_m;
//    return val.toString();
}

void DimmingObject::onWrite(const uint8_t* buf, int len)
{
    int newValue;
    Object::onWrite(buf, len);
    if (len == 2)
        newValue = (buf[1] & 0x3F);
    else
        newValue = buf[2];
    int direction = newValue & 0x08;
    int stepcode = newValue & 0x07;

    if (!init_m || stepcode != stepcode_m  || direction != direction_m)
    {
        std::cout << "New value " << (direction ? "up" : "down") << ":" << stepcode << " for dimming object " << getID() << std::endl;
        stepcode_m = stepcode;
        direction_m = direction;
        init_m = true;
        onUpdate();
    }
}

void DimmingObject::setDimmerValue(int direction, int stepcode)
{
    if (!init_m || stepcode_m != stepcode  || direction_m != direction || forcewrite_m)
    {
        stepcode_m = stepcode;
        direction_m = direction;
        uint8_t buf[3] = { 0, 0x80 };
        
        buf[1] = (direction_m ? 0x88 : 0x80) | (stepcode_m & 0x07);
        Services::instance()->getKnxConnection()->write(getGad(), buf, 2);

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

void TimeObject::onWrite(const uint8_t* buf, int len)
{
    if (len < 5)
    {
        std::cout << "Invlalid packet received for TimeObject (too short)" << std::endl;
        return;
    }
    int wday, hour, min, sec;
    Object::onWrite(buf, len);

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

void TimeObject::setTime(int wday, int hour, int min, int sec)
{
    if (!init_m ||
            wday_m != wday ||
            hour_m != hour ||
            min_m != min ||
            sec_m != sec ||
            forcewrite_m)
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

        uint8_t buf[5] = { 0, 0x80, ((wday<<5) & 0xE0) | (hour & 0x1F), min, sec };

        Services::instance()->getKnxConnection()->write(getGad(), buf, 5);
        init_m = true;
        onUpdate();
    }
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

void DateObject::onWrite(const uint8_t* buf, int len)
{
    if (len < 5)
    {
        std::cout << "Invlalid packet received for DateObject (too short)" << std::endl;
        return;
    }
    int day, month, year;
    Object::onWrite(buf, len);

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

void DateObject::setDate(int day, int month, int year)
{
    if (year >= 1900)
        year -= 1900;
    if (!init_m ||
            day_m != day ||
            month_m != month ||
            year_m != year ||
            forcewrite_m)
    {
        std::cout << "DateObject: setDate "
        << year + 1900 << "-"
        << month << "-"
        << day << std::endl;
        day_m = day;
        month_m = month;
        year_m = year;

        uint8_t buf[5] = { 0, 0x80, day, month, year };

        Services::instance()->getKnxConnection()->write(getGad(), buf, 5);
        init_m = true;
        onUpdate();
    }
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

void ValueObject::onWrite(const uint8_t* buf, int len)
{
    if (len < 4)
    {
        std::cout << "Invlalid packet received for ValueObject (too short)" << std::endl;
        return;
    }
    float newValue;
    Object::onWrite(buf, len);
    int d1 = ((unsigned char) buf[2]) * 256 + (unsigned char) buf[3];
    int m = d1 & 0x7ff;
    if (d1 & 0x8000)
        m |= ~0x7ff;
    int ex = (d1 & 0x7800) >> 11;
    newValue = ((float)m * (1 << ex) / 100);
    if (!init_m || newValue != value_m)
    {
        std::cout << "New value " << newValue << " for value object " << getID() << std::endl;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void ValueObject::setFloatValue(float value)
{
    if (!init_m || value != value_m || forcewrite_m)
    {
        value_m = value;
        uint8_t buf[4] = { 0, 0x80, 0, 0 };
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

void ScalingObject::onWrite(const uint8_t* buf, int len)
{
    int newValue;
    Object::onWrite(buf, len);
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

void ScalingObject::setIntValue(int value)
{
    if (!init_m || value != value_m || forcewrite_m)
    {
        value_m = value;
        uint8_t buf[3] = { 0, 0x80, 0 };
        buf[2] = (value & 0xff);

        Services::instance()->getKnxConnection()->write(getGad(), buf, 3);
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

ObjectController::ObjectController()
{
    Services::instance()->getKnxConnection()->addTelegramListener(this);
}

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
    ObjectMap_t::iterator it = objectMap_m.find(dest);
    if (it != objectMap_m.end())
        (*it).second->onWrite(buf, len);
}

void ObjectController::onRead(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len) { };
void ObjectController::onResponse(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len) { onWrite(src, dest, buf, len); };

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
    if (object->getGad() && !objectMap_m.insert(ObjectPair_t(object->getGad(), object)).second)
        throw ticpp::Exception("Object GAD is already registered");
}

void ObjectController::importXml(ticpp::Element* pConfig)
{
    ticpp::Iterator< ticpp::Element > child("object");
    for ( child = pConfig->FirstChildElement("object", false); child != child.end(); child++ )
    {
        std::string id = child->GetAttribute("id");
        bool del = child->GetAttribute("delete") == "true";
        ObjectIdMap_t::iterator it = objectIdMap_m.find(id);
        if (it == objectIdMap_m.end())
        {
            if (del)
                throw ticpp::Exception("Object not found");
            Object* object = Object::create(&(*child));
            if (object->getGad() && !objectMap_m.insert(ObjectPair_t(object->getGad(), object)).second)
            {
                delete object;
                throw ticpp::Exception("Object GAD is already registered");
            }
            objectIdMap_m.insert(ObjectIdPair_t(id, object));
        }
        else if (del)
        {
            eibaddr_t gad = it->second->getGad();
            if (gad)
                objectMap_m.erase(gad);
            delete it->second;
            objectIdMap_m.erase(it);
        }
        else
        {
            eibaddr_t gad = it->second->getGad();
            it->second->importXml(&(*child));
            eibaddr_t gad2 = it->second->getGad();
            if (gad != gad2)
            {
                if (gad2)
                {
                    if (!objectMap_m.insert(ObjectPair_t(gad2, it->second)).second)
                        throw ticpp::Exception("New object GAD is already registered");
                }
                if (gad)
                    objectMap_m.erase(gad);
            }
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
