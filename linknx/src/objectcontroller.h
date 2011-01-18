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
#include <stdint.h>
#include "config.h"
#include "logger.h"
#include "ticpp.h"
#include "knxconnection.h"

class Object;

class ChangeListener
{
public:
    virtual ~ChangeListener() {};
    virtual void onChange(Object* object) = 0;
    virtual const char* getID() { return "?"; };
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
    virtual int compare(ObjectValue* value) = 0;
    virtual void setValue(ObjectValue* value) = 0;
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getValue() = 0;
    virtual std::string getType() = 0;

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    void setID(const char* id) { id_m = id; };
    const char* getID() { return id_m.c_str(); };
    const char* getDescr() { return descr_m.c_str(); };
    const eibaddr_t getGad() { return gad_m; };
    const eibaddr_t getReadRequestGad() { return readRequestGad_m; };
    std::list<eibaddr_t>::iterator getListenerGad() { return listenerGadList_m.begin(); };
    std::list<eibaddr_t>::iterator getListenerGadEnd() { return listenerGadList_m.end(); };
    //    eibaddr_t getListenerGad(int idx) { return listenerGadList_m[idx]; };
    const eibaddr_t getLastTx() { return lastTx_m; };
    void read();
    virtual void onUpdate();
    void onInternalUpdate();
    bool forceUpdate() { return (!init_m || (flags_m & Stateless)); };
    void addChangeListener(ChangeListener* listener);
    void removeChangeListener(ChangeListener* listener);
    void onWrite(const uint8_t* buf, int len, eibaddr_t src);
    void onRead(const uint8_t* buf, int len, eibaddr_t src);
    void onResponse(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src) = 0;
    virtual void doSend(bool isWrite) = 0;
protected:
    bool init_m;
    enum Flags
    {
        Comm = 0x01,
        Read = 0x02,
        Write = 0x04,
        Transmit = 0x08,
        Update = 0x10,
        Init = 0x20,
        Stateless = 0x80,
        Default = Comm | Write | Transmit | Update
    };
    int flags_m;
    static Logger& logger_m;
private:
    std::string id_m;
    std::string initValue_m;
    std::string descr_m;
    eibaddr_t gad_m;
    eibaddr_t readRequestGad_m;
    eibaddr_t lastTx_m;
    bool persist_m;
    bool writeLog_m;
    bool readPending_m;
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
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "1.001"; };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    void setBoolValue(bool value);
    bool getBoolValue()
    {
        if (!init_m)
            read();
        return value_m;
    };
protected:
    bool value_m;
    static Logger& logger_m;
};

class StepDirObject : public Object
{
public:
    StepDirObject();
    virtual ~StepDirObject();

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getValue() = 0;
    virtual std::string getType() = 0;

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual void setStepValue(int direction, int stepcode);
protected:
    int direction_m;
    int stepcode_m;
    static Logger& logger_m;
};

class DimmingObject : public StepDirObject
{
public:
    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "3.007"; };
protected:
    static Logger& logger_m;
};

class BlindsObject : public StepDirObject
{
public:
    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "3.008"; };
protected:
    static Logger& logger_m;
};

class TimeObject : public Object
{
public:
    TimeObject();
    virtual ~TimeObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "10.001"; };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    void setTime(time_t time);
    void setTime(int wday, int hour, int min, int sec);
    void getTime(int *wday, int *hour, int *min, int *sec);
protected:
    int wday_m;
    int hour_m;
    int min_m;
    int sec_m;
    static Logger& logger_m;
};

class DateObject : public Object
{
public:
    DateObject();
    virtual ~DateObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "11.001"; };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    void setDate(time_t time);
    void setDate(int day, int month, int year);
    void getDate(int *day, int *month, int *year);
protected:
    int day_m;
    int month_m;
    int year_m;
    static Logger& logger_m;
};

class ValueObject : public Object
{
public:
    ValueObject();
    virtual ~ValueObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "9.xxx"; };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    void setFloatValue(double value);
    double getFloatValue()
    {
        if (!init_m)
            read();
        return value_m;
    };
protected:
    double value_m;
    static Logger& logger_m;
};

class ValueObject32 : public ValueObject
{
public:
    ValueObject32() {};
    virtual ~ValueObject32() {};

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "14.xxx"; };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
private:
    typedef union {
        uint32_t u32;
        float fl;
    } convfloat;
};


class UIntObject : public Object
{
public:
    UIntObject();
    virtual ~UIntObject();

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getValue() = 0;
    virtual std::string getType() = 0;

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src) = 0;
    virtual void doSend(bool isWrite) = 0;
    void setIntValue(uint32_t value);
    uint32_t getIntValue()
    {
        if (!init_m)
            read();
        return value_m;
    };
protected:
    uint32_t value_m;
    static Logger& logger_m;
};

class U8Object : public UIntObject
{
public:
    U8Object();
    virtual ~U8Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "5.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
};

class ScalingObject : public U8Object
{
public:
    ScalingObject();
    virtual ~ScalingObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "5.001"; };
protected:
    static Logger& logger_m;
};

class AngleObject : public U8Object
{
public:
    AngleObject();
    virtual ~AngleObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "5.003"; };
protected:
    static Logger& logger_m;
};

class HeatingModeObject : public U8Object
{
public:
    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "20.102"; };
protected:
    static Logger& logger_m;
};

class U16Object : public UIntObject
{
public:
    U16Object();
    virtual ~U16Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "7.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
};

class U32Object : public UIntObject
{
public:
    U32Object();
    virtual ~U32Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "12.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
};

class IntObject : public Object
{
public:
    IntObject();
    virtual ~IntObject();

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getValue() = 0;
    virtual std::string getType() = 0;

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src) = 0;
    virtual void doSend(bool isWrite) = 0;
    void setIntValue(int32_t value);
    int32_t getIntValue()
    {
        if (!init_m)
            read();
        return value_m;
    };
protected:
    int32_t value_m;
    static Logger& logger_m;
};

class S8Object : public IntObject
{
public:
    S8Object();
    virtual ~S8Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "6.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
};

class S16Object : public IntObject
{
public:
    S16Object();
    virtual ~S16Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "8.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
};

class S32Object : public IntObject
{
public:
    S32Object();
    virtual ~S32Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "13.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
};

class S64Object : public Object
{
public:
    S64Object();
    virtual ~S64Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "29.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);

    void setIntValue(int64_t value);
    int64_t getIntValue()
    {
        if (!init_m)
            read();
        return value_m;
    };
protected:
    int64_t value_m;
    static Logger& logger_m;
};

class StringObject : public Object
{
public:
    StringObject();
    virtual ~StringObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value);
    virtual std::string getValue();
    virtual std::string getType() { return "28.001"; };

    void setStringValue(const std::string& val);

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    std::string value_m;
    static Logger& logger_m;
};

class String14Object : public StringObject
{
public:
    String14Object();
    virtual ~String14Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "16.000"; };

    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
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

class StepDirObjectValue : public ObjectValue
{
public:
    virtual ~StepDirObjectValue() {};
    virtual std::string toString() = 0;
protected:
    StepDirObjectValue() : direction_m(0), stepcode_m(0) {};
    StepDirObjectValue(int direction, int stepcode) : direction_m(direction), stepcode_m(stepcode) {};
    friend class StepDirObject;
    int direction_m;
    int stepcode_m;
};

class DimmingObjectValue : public StepDirObjectValue
{
public:
    DimmingObjectValue(const std::string& value);
    virtual ~DimmingObjectValue() {};
    virtual std::string toString();
protected:
    DimmingObjectValue(int direction, int stepcode) : StepDirObjectValue(direction, stepcode) {};
    friend class DimmingObject;
};

class BlindsObjectValue : public StepDirObjectValue
{
public:
    BlindsObjectValue(const std::string& value);
    virtual ~BlindsObjectValue() {};
    virtual std::string toString();
protected:
    BlindsObjectValue(int direction, int stepcode) : StepDirObjectValue(direction, stepcode) {};
    friend class BlindsObject;
};

class TimeObjectValue : public ObjectValue
{
public:
    TimeObjectValue(const std::string& value);
    virtual ~TimeObjectValue() {};
    virtual std::string toString();
    void getTimeValue(int *wday, int *hour, int *min, int *sec);
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
    void getDateValue(int *day, int *month, int *year);
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

class ValueObject32Value : public ObjectValue
{
public:
    ValueObject32Value(const std::string& value);
    virtual ~ValueObject32Value() {};
    virtual std::string toString();
protected:
    ValueObject32Value(double value) : value_m(value) {};
    friend class ValueObject32;
    double value_m;
};

class UIntObjectValue : public ObjectValue
{
public:
    UIntObjectValue(const std::string& value);
    virtual ~UIntObjectValue() {};
    virtual std::string toString();
protected:
    UIntObjectValue(uint32_t value) : value_m(value) {};
    UIntObjectValue() {};
    friend class UIntObject;
    uint32_t value_m;
};

class U8ObjectValue : public UIntObjectValue
{
public:
    U8ObjectValue(const std::string& value);
    virtual ~U8ObjectValue() {};
    virtual std::string toString();
protected:
    U8ObjectValue(uint32_t value) : UIntObjectValue(value) {};
    U8ObjectValue() {};
    friend class U8Object;
};

class ScalingObjectValue : public U8ObjectValue
{
public:
    ScalingObjectValue(const std::string& value);
    virtual ~ScalingObjectValue() {};
    virtual std::string toString();
protected:
    ScalingObjectValue(uint32_t value) : U8ObjectValue(value) {};
    friend class ScalingObject;
};

class AngleObjectValue : public U8ObjectValue
{
public:
    AngleObjectValue(const std::string& value);
    virtual std::string toString();
protected:
    AngleObjectValue(uint32_t value) : U8ObjectValue(value) {};
    friend class AngleObject;
};

class HeatingModeObjectValue : public U8ObjectValue
{
public:
    HeatingModeObjectValue(const std::string& value);
    virtual std::string toString();
protected:
    HeatingModeObjectValue(uint32_t value) : U8ObjectValue(value) {};
    friend class HeatingModeObject;
};

class U16ObjectValue : public UIntObjectValue
{
public:
    U16ObjectValue(const std::string& value);
    virtual ~U16ObjectValue() {};
    virtual std::string toString();
protected:
    U16ObjectValue(uint32_t value) : UIntObjectValue(value) {};
    U16ObjectValue() {};
    friend class U16Object;
};

class U32ObjectValue : public UIntObjectValue
{
public:
    U32ObjectValue(const std::string& value);
    virtual ~U32ObjectValue() {};
    virtual std::string toString();
protected:
    U32ObjectValue(uint32_t value) : UIntObjectValue(value) {};
    U32ObjectValue() {};
    friend class U32Object;
};

class IntObjectValue : public ObjectValue
{
public:
    IntObjectValue(const std::string& value);
    virtual ~IntObjectValue() {};
    virtual std::string toString();
protected:
    IntObjectValue(int32_t value) : value_m(value) {};
    IntObjectValue() {};
    friend class IntObject;
    int32_t value_m;
};

class S8ObjectValue : public IntObjectValue
{
public:
    S8ObjectValue(const std::string& value);
    virtual ~S8ObjectValue() {};
    virtual std::string toString();
protected:
    S8ObjectValue(int32_t value) : IntObjectValue(value) {};
    S8ObjectValue() {};
    friend class S8Object;
};

class S16ObjectValue : public IntObjectValue
{
public:
    S16ObjectValue(const std::string& value);
    virtual ~S16ObjectValue() {};
    virtual std::string toString();
protected:
    S16ObjectValue(int32_t value) : IntObjectValue(value) {};
    S16ObjectValue() {};
    friend class S16Object;
};

class S32ObjectValue : public IntObjectValue
{
public:
    S32ObjectValue(const std::string& value);
    virtual ~S32ObjectValue() {};
    virtual std::string toString();
protected:
    S32ObjectValue(int32_t value) : IntObjectValue(value) {};
    S32ObjectValue() {};
    friend class S32Object;
};

class S64ObjectValue : public ObjectValue
{
public:
    S64ObjectValue(const std::string& value);
    virtual ~S64ObjectValue() {};
    virtual std::string toString();
protected:
    S64ObjectValue(int64_t value) : value_m(value) {};
    S64ObjectValue() {};
    friend class S64Object;
    int64_t value_m;
};

class StringObjectValue : public ObjectValue
{
public:
    StringObjectValue(const std::string& value);
    virtual ~StringObjectValue() {};
    virtual std::string toString();
protected:
    friend class StringObject;
    friend class String14Object;
    std::string value_m;
};

class String14ObjectValue : public StringObjectValue
{
public:
    String14ObjectValue(const std::string& value);
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

    virtual void exportObjectValues(ticpp::Element* pObjects);

    virtual void onWrite(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);
    virtual void onRead(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);
    virtual void onResponse(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);
    virtual std::list<Object*> getObjects();

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
