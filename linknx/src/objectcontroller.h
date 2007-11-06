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

#ifndef OBJECTCONTROLLER_H
#define OBJECTCONTROLLER_H

#include <list>
#include <string>
#include <map>
#include "config.h"
#include "ticpp.h"
#include "knxconnection.h"

class Object;

class ChangeListener
{
public:
    virtual void onChange(Object* object) = 0;
};

class ObjectValue
{
public:
    virtual ~ObjectValue() {};
    virtual std::string toString() = 0;
};

class Object
{
public:
    Object();
    virtual ~Object();

    static Object* create(ticpp::Element* pConfig);
    static Object* create(const std::string& type);

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual bool equals(ObjectValue* value) = 0;
    virtual void setValue(ObjectValue* value) = 0;
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getValue() = 0;

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    void setID(const char* id) { id_m = id; };
    const char* getID() { return id_m.c_str(); };
    const char* getDescr() { return descr_m.c_str(); };
    const eibaddr_t getGad() { return gad_m; };
    std::list<eibaddr_t>::iterator getListenerGad() { return listenerGadList_m.begin(); };
    std::list<eibaddr_t>::iterator getListenerGadEnd() { return listenerGadList_m.end(); };
//    eibaddr_t getListenerGad(int idx) { return listenerGadList_m[idx]; };
    const eibaddr_t getLastTx() { return lastTx_m; };
    void read();
    virtual void onUpdate();
    void addChangeListener(ChangeListener* listener);
    void removeChangeListener(ChangeListener* listener);
    virtual void onWrite(const uint8_t* buf, int len, eibaddr_t src);
protected:
    bool init_m;
    bool forcewrite_m;
private:
    std::string id_m;
    eibaddr_t gad_m;
    eibaddr_t lastTx_m;
    std::string initValue_m;
    bool readPending_m;
    std::string descr_m;
    typedef std::list<ChangeListener*> ListenerList_t;
    ListenerList_t listenerList_m;
    typedef std::list<eibaddr_t> ListenerGadList_t;
    ListenerGadList_t listenerGadList_m;
};

class SwitchingObject : public Object
{
public:
    SwitchingObject();
    virtual ~SwitchingObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();

    virtual void onWrite(const uint8_t* buf, int len, eibaddr_t src);
    void setBoolValue(bool value);
    bool getBoolValue()
    {
        if (!init_m)
            read();
        return value_m;
    };
protected:
    bool value_m;
};

class DimmingObject : public Object
{
public:
    DimmingObject();
    virtual ~DimmingObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();

    virtual void exportXml(ticpp::Element* pConfig);

    virtual void onWrite(const uint8_t* buf, int len, eibaddr_t src);
    void setDimmerValue(int direction, int stepcode);
protected:
    int direction_m;
    int stepcode_m;
};

class TimeObject : public Object
{
public:
    TimeObject();
    virtual ~TimeObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();

    virtual void exportXml(ticpp::Element* pConfig);

    virtual void onWrite(const uint8_t* buf, int len, eibaddr_t src);
    void setTime(time_t time);
    void setTime(int wday, int hour, int min, int sec);
    void getTime(int *wday, int *hour, int *min, int *sec);
protected:
    int wday_m;
    int hour_m;
    int min_m;
    int sec_m;
};

class DateObject : public Object
{
public:
    DateObject();
    virtual ~DateObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();

    virtual void exportXml(ticpp::Element* pConfig);

    virtual void onWrite(const uint8_t* buf, int len, eibaddr_t src);
    void setDate(time_t time);
    void setDate(int day, int month, int year);
    void getDate(int *day, int *month, int *year);
protected:
    int day_m;
    int month_m;
    int year_m;
};

class ValueObject : public Object
{
public:
    ValueObject();
    virtual ~ValueObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();

    virtual void exportXml(ticpp::Element* pConfig);

    virtual void onWrite(const uint8_t* buf, int len, eibaddr_t src);
    void setFloatValue(double value);
    double getFloatValue()
    {
        if (!init_m)
            read();
        return value_m;
    };
protected:
    double value_m;
};

class ScalingObject : public Object
{
public:
    ScalingObject();
    virtual ~ScalingObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();

    virtual void exportXml(ticpp::Element* pConfig);

    virtual void onWrite(const uint8_t* buf, int len, eibaddr_t src);
    void setIntValue(int value);
    int getIntValue()
    {
        if (!init_m)
            read();
        return value_m;
    };
protected:
    int value_m;
};

class HeatingModeObject : public ScalingObject
{
public:
    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();

    virtual void exportXml(ticpp::Element* pConfig);
};

class SwitchingObjectValue : public ObjectValue
{
public:
    SwitchingObjectValue(const std::string& value);
    virtual ~SwitchingObjectValue() {};
    virtual std::string toString();
protected:
    SwitchingObjectValue(bool value) : value_m(value) {};
    friend class SwitchingObject;
    bool value_m;
};

class DimmingObjectValue : public ObjectValue
{
public:
    DimmingObjectValue(const std::string& value);
    virtual ~DimmingObjectValue() {};
    virtual std::string toString();
protected:
    DimmingObjectValue(int direction, int stepcode) : direction_m(direction), stepcode_m(stepcode) {};
    friend class DimmingObject;
    int direction_m;
    int stepcode_m;
};

class TimeObjectValue : public ObjectValue
{
public:
    TimeObjectValue(const std::string& value);
    virtual ~TimeObjectValue() {};
    virtual std::string toString();
protected:
    TimeObjectValue(int wday, int hour, int min, int sec) : wday_m(wday), hour_m(hour), min_m(min), sec_m(sec) {};
    friend class TimeObject;
    int wday_m;
    int hour_m;
    int min_m;
    int sec_m;
};

class DateObjectValue : public ObjectValue
{
public:
    DateObjectValue(const std::string& value);
    virtual ~DateObjectValue() {};
    virtual std::string toString();
protected:
    DateObjectValue(int day, int month, int year) : day_m(day), month_m(month), year_m(year) {};
    friend class DateObject;
    int day_m;
    int month_m;
    int year_m;
};

class ValueObjectValue : public ObjectValue
{
public:
    ValueObjectValue(const std::string& value);
    virtual ~ValueObjectValue() {};
    virtual std::string toString();
protected:
    ValueObjectValue(double value) : value_m(value) {};
    friend class ValueObject;
    double value_m;
};

class ScalingObjectValue : public ObjectValue
{
public:
    ScalingObjectValue(const std::string& value);
    virtual ~ScalingObjectValue() {};
    virtual std::string toString();
protected:
    ScalingObjectValue(int value) : value_m(value) {};
    ScalingObjectValue() {};
    friend class ScalingObject;
    int value_m;
};

class HeatingModeObjectValue : public ScalingObjectValue
{
public:
    HeatingModeObjectValue(const std::string& value);
    virtual std::string toString();
protected:
    HeatingModeObjectValue(int value) : ScalingObjectValue(value) {};
    friend class HeatingModeObject;
};

class ObjectController : public TelegramListener
{
public:
    static ObjectController* instance();
    static void reset()
    {
        if (instance_m)
            delete instance_m;
        instance_m = 0;
    };
    void addObject(Object* object);
    void removeObject(Object* object);

    Object* getObject(const std::string& id);

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    virtual void onWrite(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);
    virtual void onRead(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);
    virtual void onResponse(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);

private:
    ObjectController();
    virtual ~ObjectController();

    void removeObjectFromAddressMap(eibaddr_t gad, Object* object);

    typedef std::pair<eibaddr_t ,Object*> ObjectPair_t;
    typedef std::multimap<eibaddr_t ,Object*> ObjectMap_t;
    typedef std::pair<std::string ,Object*> ObjectIdPair_t;
    typedef std::map<std::string ,Object*> ObjectIdMap_t;
    ObjectMap_t objectMap_m;
    ObjectIdMap_t objectIdMap_m;
    static ObjectController* instance_m;
};

#endif
