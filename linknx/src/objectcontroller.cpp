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

Logger& Object::logger_m(Logger::getInstance("Object"));

Object::Object() : init_m(false), flags_m(Default), gad_m(0), persist_m(false), writeLog_m(false), readPending_m(false)
{}

Object::~Object()
{}

Object* Object::create(const std::string& type)
{
    if (type == "" || type == "EIS1" || type == "1.001")
        return new SwitchingObject();
    else if (type == "EIS2" || type == "3.007")
        return new DimmingObject();
    else if (type == "3.008")
        return new BlindsObject();
    else if (type == "EIS3" || type == "10.001")
        return new TimeObject();
    else if (type == "EIS4" || type == "11.001")
        return new DateObject();
    else if (type == "EIS5" || type == "9.xxx")
        return new ValueObject();
    else if (type == "14.xxx")
        return new ValueObject32();
    else if (type == "EIS6" || type == "5.xxx")
        return new U8Object();
    else if (type == "5.001")
        return new ScalingObject();
    else if (type == "5.003")
        return new AngleObject();
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
    else if (type == "EIS15" || type == "16.000")
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
    
    writeLog_m = (pConfig->GetAttribute("log") == "true");

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
                setValue(val);
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
        setValue(initValue_m);

    logger_m.infoStream() << "Configured object '" << id_m << "': gad='" << gad_m << "'" << endlog;
}

void Object::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", getType());
    pConfig->SetAttribute("id", id_m);

    if (gad_m != 0)
        pConfig->SetAttribute("gad", writegaddr(gad_m));

    if (initValue_m != "")
        pConfig->SetAttribute("init", initValue_m);

    if (writeLog_m)
        pConfig->SetAttribute("log", "true");

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
    // If the device didn't answer after 1 second, we consider the object's
    // default value as the current value to avoid waiting forever.
    init_m = true;
}

void Object::onUpdate()
{
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
    unsigned int pos = value.find(":");
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

BlindsObjectValue::BlindsObjectValue(const std::string& value)
{
    std::string dir;
    unsigned int pos = value.find(":");
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
    out << hour_m << ":" << min_m << ":" << sec_m;
    return out.str();
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

ValueObjectValue::ValueObjectValue(const std::string& value)
{
    std::istringstream val(value);
    val >> value_m;

    if ( val.fail() ||
         val.peek() != std::char_traits<char>::eof() || // workaround for wrong val.eof() flag in uClibc++
         value_m > 670760.96 ||
         value_m < -671088.64)
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
//    logger_m.debugStream() << "StringObjectValue: Value: '" << value_m << "'" << endlog;
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

Logger& SwitchingObject::logger_m(Logger::getInstance("SwitchingObject"));

bool SwitchingObject::equals(ObjectValue* value)
{
    assert(value);
    SwitchingObjectValue* val = dynamic_cast<SwitchingObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "SwitchingObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "SwitchingObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int SwitchingObject::compare(ObjectValue* value)
{
    assert(value);
    SwitchingObjectValue* val = dynamic_cast<SwitchingObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "SwitchingObject: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "SwitchingObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    if (value_m == val->value_m)
        return 0;
    else if (value_m)
        return 1;
    else
        return -1;
}

void SwitchingObject::setValue(ObjectValue* value)
{
    assert(value);
    SwitchingObjectValue* val = dynamic_cast<SwitchingObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "SwitchingObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
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
        logger_m.infoStream() << "New value " << newValue << " for switching object " << getID() << endlog;
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

Logger& StepDirObject::logger_m(Logger::getInstance("StepDirObject"));

StepDirObject::StepDirObject() : direction_m(0), stepcode_m(0)
{}

StepDirObject::~StepDirObject()
{}

bool StepDirObject::equals(ObjectValue* value)
{
    assert(value);
    StepDirObjectValue* val = dynamic_cast<StepDirObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "StepDirObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "StepDirObject (id=" << getID() << "): Compare object='"
    << getValue() << "' to value='"
    << val->toString() << "'" << endlog;
    return (direction_m == val->direction_m) && (stepcode_m == val->stepcode_m);
}

int StepDirObject::compare(ObjectValue* value)
{
    assert(value);
    StepDirObjectValue* val = dynamic_cast<StepDirObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "StepDirObject: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "StepDirObject (id=" << getID() << "): Compare object='"
    << getValue() << "' to value='"
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

void StepDirObject::setValue(ObjectValue* value)
{
    assert(value);
    StepDirObjectValue* val = dynamic_cast<StepDirObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream()  << "StepDirObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
        setStepValue(val->direction_m, val->stepcode_m);
}

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

    if (!init_m || stepcode != stepcode_m  || direction != direction_m)
    {
        stepcode_m = stepcode;
        direction_m = direction;
        init_m = true;
        logger_m.infoStream() << "New value " << getValue() << " for object " << getID() << endlog;
        onUpdate();
    }
}

void StepDirObject::doSend(bool isWrite)
{
    uint8_t buf[2] = { 0, (isWrite ? 0x80 : 0x40) | (direction_m ? 8 : 0) | (stepcode_m & 0x07) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 2);
}

void StepDirObject::setStepValue(int direction, int stepcode)
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

Logger& DimmingObject::logger_m(Logger::getInstance("DimmingObject"));

ObjectValue* DimmingObject::createObjectValue(const std::string& value)
{
    return new DimmingObjectValue(value);
}


void DimmingObject::setValue(const std::string& value)
{
    DimmingObjectValue val(value);
    setStepValue(val.direction_m, val.stepcode_m);
}

std::string DimmingObject::getValue()
{
    if (!init_m)
        read();
    return DimmingObjectValue(direction_m, stepcode_m).toString();
}


Logger& BlindsObject::logger_m(Logger::getInstance("BlindsObject"));

ObjectValue* BlindsObject::createObjectValue(const std::string& value)
{
    return new BlindsObjectValue(value);
}

void BlindsObject::setValue(const std::string& value)
{
    BlindsObjectValue val(value);
    setStepValue(val.direction_m, val.stepcode_m);
}

std::string BlindsObject::getValue()
{
    if (!init_m)
        read();
    return BlindsObjectValue(direction_m, stepcode_m).toString();
}

Logger& TimeObject::logger_m(Logger::getInstance("TimeObject"));

TimeObject::TimeObject() : wday_m(0), hour_m(0), min_m(0), sec_m(0)
{}

TimeObject::~TimeObject()
{}

ObjectValue* TimeObject::createObjectValue(const std::string& value)
{
    return new TimeObjectValue(value);
}

bool TimeObject::equals(ObjectValue* value)
{
    int wday, hour, min, sec;
    assert(value);
    TimeObjectValue* val = dynamic_cast<TimeObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "TimeObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    // logger_m.debugStream() << "TimeObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    val->getTimeValue(&wday, &hour, &min, &sec);
    return (sec_m == sec) && (min_m == min) && (hour_m == hour) && (wday_m == wday);
}

int TimeObject::compare(ObjectValue* value)
{
    int wday, hour, min, sec;
    assert(value);
    TimeObjectValue* val = dynamic_cast<TimeObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "TimeObject: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    // logger_m.debugStream() << "TimeObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
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

void TimeObject::setValue(ObjectValue* value)
{
    int wday, hour, min, sec;
    assert(value);
    TimeObjectValue* val = dynamic_cast<TimeObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "TimeObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        val->getTimeValue(&wday, &hour, &min, &sec);
        setTime(wday, hour, min, sec);
    }
}

void TimeObject::setValue(const std::string& value)
{
    int wday, hour, min, sec;
    TimeObjectValue val(value);
    val.getTimeValue(&wday, &hour, &min, &sec);
    setTime(wday, hour, min, sec);
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
        logger_m.errorStream() << "Invalid packet received for TimeObject (too short)" << endlog;
        return;
    }
    int wday, hour, min, sec;

    wday = (buf[2] & 0xE0) >> 5;
    hour = buf[2] & 0x1F;
    min = buf[3];
    sec = buf[4];
    if (!init_m || wday != wday_m || hour != hour_m || min != min_m || sec != sec_m)
    {
        logger_m.infoStream() << "New value " << wday << " " << hour << ":" << min << ":" << sec << " for time object " << getID() << endlog;
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
        logger_m.infoStream() << "TimeObject: setTime "
        << wday << " "
        << hour << ":"
        << min << ":"
        << sec << endlog;
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

Logger& DateObject::logger_m(Logger::getInstance("DateObject"));

DateObject::DateObject() : day_m(0), month_m(0), year_m(0)
{}

DateObject::~DateObject()
{}

ObjectValue* DateObject::createObjectValue(const std::string& value)
{
    return new DateObjectValue(value);
}

bool DateObject::equals(ObjectValue* value)
{
    int day, month, year;
    assert(value);
    DateObjectValue* val = dynamic_cast<DateObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream()  << "DateObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    // logger_m.debugStream() << "DateObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    val->getDateValue(&day, &month, &year);
    return (day_m == day) && (month_m == month) && (year_m == year);
}

int DateObject::compare(ObjectValue* value)
{
    int day, month, year;
    assert(value);
    DateObjectValue* val = dynamic_cast<DateObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "DateObject: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    // logger_m.debugStream() << "DateObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
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

void DateObject::setValue(ObjectValue* value)
{
    int day, month, year;
    assert(value);
    DateObjectValue* val = dynamic_cast<DateObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "DateObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
    {
        val->getDateValue(&day, &month, &year);
        setDate(day, month, year);
    }
}

void DateObject::setValue(const std::string& value)
{
    int day, month, year;
    DateObjectValue val(value);
    val.getDateValue(&day, &month, &year);
    setDate(day, month, year);
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
        logger_m.errorStream() << "Invalid packet received for DateObject (too short)" << endlog;
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
        logger_m.infoStream() << "New value " << year+1900 << "-" << month << "-" << day << " for date object " << getID() << endlog;
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
    uint8_t buf[5] = { 0,
                       (isWrite ? 0x80 : 0x40),
                       day_m, month_m,
                       (year_m >= 100 && year_m < 190) ? year_m-100 : year_m };
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
        logger_m.infoStream() << "DateObject: setDate "
        << year + 1900 << "-"
        << month << "-"
        << day << endlog;
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

Logger& ValueObject::logger_m(Logger::getInstance("ValueObject"));

ValueObject::ValueObject() : value_m(0)
{}

ValueObject::~ValueObject()
{}

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
        logger_m.errorStream() << "ValueObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "ValueObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int ValueObject::compare(ObjectValue* value)
{
    assert(value);
    ValueObjectValue* val = dynamic_cast<ValueObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "ValueObject: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "ValueObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    
    if (value_m == val->value_m)
        return 0;
    if (value_m > val->value_m)
        return 1;
    else
        return -1;
}

void ValueObject::setValue(ObjectValue* value)
{
    assert(value);
    ValueObjectValue* val = dynamic_cast<ValueObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "ValueObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
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
    if (!init_m || newValue != value_m)
    {
        logger_m.infoStream() << "New value " << newValue << " for value object " << getID() << endlog;
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

Logger& ValueObject32::logger_m(Logger::getInstance("valueObject32"));

ObjectValue* ValueObject32::createObjectValue(const std::string& value)
{
    return new ValueObject32Value(value);
}

void ValueObject32::setValue(ObjectValue* value)
{
    assert(value);
    ValueObject32Value* val = dynamic_cast<ValueObject32Value*>(value);
    if (val == 0)
        logger_m.errorStream() << "ValueObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
        setFloatValue(val->value_m);
}

void ValueObject32::setValue(const std::string& value)
{
    ValueObject32Value val(value);
    setFloatValue(val.value_m);
}

std::string ValueObject32::getValue()
{
    return ValueObject32Value(getFloatValue()).toString();
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
    if (!init_m || newValue != value_m)
    {
        logger_m.infoStream() << "New value " << newValue << " for ValueObject32 " << getID() << endlog;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void ValueObject32::doSend(bool isWrite)
{
    uint8_t buf[6] = { 0, (isWrite ? 0x80 : 0x40), 0, 0, 0, 0 };
    convfloat tmp;
    tmp.fl = static_cast<float>(value_m);
    buf[5] = static_cast<uint8_t>(tmp.u32 & 0x000000FF); 
    buf[4] = static_cast<uint8_t>((tmp.u32 & 0x0000FF00) >> 8); 
    buf[3] = static_cast<uint8_t>((tmp.u32 & 0x00FF0000) >> 16); 
    buf[2] = static_cast<uint8_t>((tmp.u32 & 0xFF000000) >> 24);

    Services::instance()->getKnxConnection()->write(getGad(), buf, 6);
}

Logger& UIntObject::logger_m(Logger::getInstance("UIntObject"));

UIntObject::UIntObject() : value_m(0)
{}

UIntObject::~UIntObject()
{}

bool UIntObject::equals(ObjectValue* value)
{
    assert(value);
    UIntObjectValue* val = dynamic_cast<UIntObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "UIntObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "UIntObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int UIntObject::compare(ObjectValue* value)
{
    assert(value);
    UIntObjectValue* val = dynamic_cast<UIntObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "UIntObject: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "UIntObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;

    if (value_m == val->value_m)
        return 0;
    if (value_m > val->value_m)
        return 1;
    else
        return -1;
}

void UIntObject::setValue(ObjectValue* value)
{
    assert(value);
    UIntObjectValue* val = dynamic_cast<UIntObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "UIntObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
        setIntValue(val->value_m);
}

void UIntObject::setIntValue(uint32_t value)
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
    setIntValue(val.value_m);
}

std::string U8Object::getValue()
{
    return U8ObjectValue(getIntValue()).toString();
}

void U8Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    uint32_t newValue;
    if (len == 2)
        newValue = (buf[1] & 0x3F);
    else
        newValue = buf[2];
    if (!init_m || newValue != value_m)
    {
        logger_m.infoStream() << "New value " << newValue << " for U8 object " << getID() << endlog;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void U8Object::doSend(bool isWrite)
{
    uint8_t buf[3] = { 0, (isWrite ? 0x80 : 0x40), (value_m & 0xff) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 3);
}

Logger& ScalingObject::logger_m(Logger::getInstance("ScalingObject"));

ScalingObject::ScalingObject()
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
    setIntValue(val.value_m);
}

std::string ScalingObject::getValue()
{
    return ScalingObjectValue(getIntValue()).toString();
}

Logger& AngleObject::logger_m(Logger::getInstance("AngleObject"));

AngleObject::AngleObject()
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
    setIntValue(val.value_m);
}

std::string AngleObject::getValue()
{
    return AngleObjectValue(getIntValue()).toString();
}

Logger& HeatingModeObject::logger_m(Logger::getInstance("HeatingModeObject"));

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
    setIntValue(val.value_m);
}

std::string U16Object::getValue()
{
    return U16ObjectValue(getIntValue()).toString();
}

void U16Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    unsigned int newValue;
    newValue = (buf[2]<<8) | buf[3];
    if (!init_m || newValue != value_m)
    {
        logger_m.infoStream() << "New value " << newValue << " for U16 object " << getID() << endlog;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void U16Object::doSend(bool isWrite)
{
    uint8_t buf[4] = { 0, (isWrite ? 0x80 : 0x40), ((value_m & 0xff00)>>8), (value_m & 0xff) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 4);
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
    setIntValue(val.value_m);
}

std::string U32Object::getValue()
{
    return U32ObjectValue(getIntValue()).toString();
}

void U32Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    unsigned int newValue;
    newValue = (buf[2]<<24) | (buf[3]<<16) | (buf[4]<<8) | buf[5];
    if (!init_m || newValue != value_m)
    {
        logger_m.infoStream() << "New value " << newValue << " for U32 object " << getID() << endlog;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void U32Object::doSend(bool isWrite)
{
    uint8_t buf[6] = { 0, (isWrite ? 0x80 : 0x40), ((value_m & 0xff000000)>>24), ((value_m & 0xff0000)>>16), ((value_m & 0xff00)>>8), (value_m & 0xff) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 6);
}

Logger& IntObject::logger_m(Logger::getInstance("IntObject"));

IntObject::IntObject() : value_m(0)
{}

IntObject::~IntObject()
{}

bool IntObject::equals(ObjectValue* value)
{
    assert(value);
    IntObjectValue* val = dynamic_cast<IntObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "IntObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "IntObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int IntObject::compare(ObjectValue* value)
{
    assert(value);
    IntObjectValue* val = dynamic_cast<IntObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "IntObject: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "IntObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;

    if (value_m == val->value_m)
        return 0;
    if (value_m > val->value_m)
        return 1;
    else
        return -1;
}

void IntObject::setValue(ObjectValue* value)
{
    assert(value);
    IntObjectValue* val = dynamic_cast<IntObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "IntObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
        setIntValue(val->value_m);
}

void IntObject::setIntValue(int32_t value)
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
    setIntValue(val.value_m);
}

std::string S8Object::getValue()
{
    return S8ObjectValue(getIntValue()).toString();
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
    if (!init_m || newValue != value_m)
    {
        logger_m.infoStream() << "New value " << newValue << " for S8 object " << getID() << endlog;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void S8Object::doSend(bool isWrite)
{
    uint8_t buf[3] = { 0, (isWrite ? 0x80 : 0x40), (value_m & 0xff) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 3);
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
    setIntValue(val.value_m);
}

std::string S16Object::getValue()
{
    return S16ObjectValue(getIntValue()).toString();
}

void S16Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int32_t newValue;
    newValue = (buf[2]<<8) | buf[3];
    if (newValue > 32767)
        newValue -= 65536;
    if (!init_m || newValue != value_m)
    {
        logger_m.infoStream() << "New value " << newValue << " for S16 object " << getID() << endlog;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void S16Object::doSend(bool isWrite)
{
    uint8_t buf[4] = { 0, (isWrite ? 0x80 : 0x40), ((value_m & 0xff00)>>8), (value_m & 0xff) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 4);
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
    setIntValue(val.value_m);
}

std::string S32Object::getValue()
{
    return S32ObjectValue(getIntValue()).toString();
}

void S32Object::doWrite(const uint8_t* buf, int len, eibaddr_t src)
{
    int32_t newValue;
    newValue = (buf[2]<<24) | (buf[3]<<16) | (buf[4]<<8) | buf[5];
    if (!init_m || newValue != value_m)
    {
        logger_m.infoStream() << "New value " << newValue << " for S32 object " << getID() << endlog;
        value_m = newValue;
        init_m = true;
        onUpdate();
    }
}

void S32Object::doSend(bool isWrite)
{
    uint8_t buf[6] = { 0, (isWrite ? 0x80 : 0x40), ((value_m & 0xff000000)>>24), ((value_m & 0xff0000)>>16), ((value_m & 0xff00)>>8), (value_m & 0xff) };
    Services::instance()->getKnxConnection()->write(getGad(), buf, 6);
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

bool StringObject::equals(ObjectValue* value)
{
    assert(value);
    StringObjectValue* val = dynamic_cast<StringObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "StringObject: ERROR, equals() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return false;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "StringObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;
    return value_m == val->value_m;
}

int StringObject::compare(ObjectValue* value)
{
    assert(value);
    StringObjectValue* val = dynamic_cast<StringObjectValue*>(value);
    if (val == 0)
    {
        logger_m.errorStream() << "StringObject: ERROR, compare() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
        return -1;
    }
    if (!init_m)
        read();
    logger_m.infoStream() << "StringObject (id=" << getID() << "): Compare value_m='" << value_m << "' to value='" << val->value_m << "'" << endlog;

    if (value_m == val->value_m)
        return 0;
    if (value_m < val->value_m)
        return -1;
    else
        return 1;
}

void StringObject::setValue(ObjectValue* value)
{
    assert(value);
    StringObjectValue* val = dynamic_cast<StringObjectValue*>(value);
    if (val == 0)
        logger_m.errorStream() << "StringObject: ERROR, setValue() received invalid class object (typeid=" << typeid(*value).name() << ")" << endlog;
    else
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
        logger_m.errorStream() << "Invalid packet received for StringObject (too short)" << endlog;
        return;
    }
    std::string value;
    for(int j=2; j<len && buf[j]!=0; j++)
        value.push_back(buf[j]);

    if (!init_m || value != value_m)
    {
        logger_m.infoStream() << "New value " << value << " for string object " << getID() << endlog;
        value_m = value;
        init_m = true;
        onUpdate();
    }
}

void StringObject::doSend(bool isWrite)
{
    logger_m.debugStream() << "StringObject: Value: " << value_m << endlog;
    uint8_t buf[16];
    memset(buf,0,sizeof(buf));
    buf[1] = (isWrite ? 0x80 : 0x40);
    // Convert to hex
    for(uint j=0;j<value_m.size();j++)
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
