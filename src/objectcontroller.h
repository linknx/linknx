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
#include <cfloat>
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
    virtual std::string toLogString() { return toString(); };
    virtual bool equals(ObjectValue* value) = 0;
    virtual int compare(ObjectValue* value) = 0;
    virtual bool set(ObjectValue* value) = 0;
    virtual double toNumber() = 0;
    virtual void setPrecision(std::string precision) {};
    virtual std::string getPrecision() { return ""; };
protected:
    static Logger& logger_m;
};

class Object
{
protected:
	class BufferBuilder
	{
		public:
			BufferBuilder(int size, Logger &logger);
			~BufferBuilder();

		public:
			template <class T> BufferBuilder &operator<<(const T &value)
			{
				setValue(index_m, value);
				++index_m;
				return *this;
			}

			uint8_t *getBuffer() {return buffer_m;}
			template <class T> void setValue(int index, const T &value)
			{
				if (index >= size_m)
				{
					logger_m.errorStream() << "Buffer size is exceeded." << endlog;
					return;
				}

				if ((value & 0xFF) != value)
				{
					logger_m.errorStream() << "Value must be one significant byte in length." << endlog;
				}

				buffer_m[index] = (uint8_t)value;
			}

		private:
			uint8_t *buffer_m;
			int index_m;
			int size_m;
			Logger &logger_m;
	};

public:
    Object();
    virtual ~Object();

    static Object* create(ticpp::Element* pConfig);
    static Object* create(const std::string& type);

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual void setValue(ObjectValue* value);
    virtual void setValue(const std::string& value) = 0;
    virtual void setFloatValue(double value);
    virtual ObjectValue* get();
    virtual std::string getValue() { return get()->toString(); };
    virtual double getFloatValue() { return get()->toNumber(); };
    virtual std::string getLogValue() { return get()->toLogString(); };
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

    void incRefCount() { refCount_m++; };
    int decRefCount() { if (refCount_m < 1) { printf("REFCOUNT ERROR %d\n", refCount_m); exit(1); }
        return --refCount_m; };
    bool inUse() { return refCount_m > 0; };

    static eibaddr_t ReadGroupAddr(const std::string& addr);
    static eibaddr_t ReadAddr(const std::string& addr);
    static std::string WriteGroupAddr(eibaddr_t addr);
    static std::string WriteAddr(eibaddr_t addr);
protected:
    virtual bool set(ObjectValue* value) = 0;
    virtual bool set(double value) = 0;
    virtual ObjectValue* getObjectValue() = 0;
    KnxConnection* getKnxConnection();
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
    int refCount_m;
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
    virtual ~SwitchingObject() {};

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getType() = 0;

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual void setBoolValue(bool value) = 0;
    virtual bool getBoolValue() = 0;
    virtual std::string getLogValue() { return get()->toLogString(); };
protected:
    virtual bool set(bool value) = 0;
    virtual bool set(ObjectValue* value) = 0;
    virtual bool set(double value) = 0;
    virtual bool getBoolObjectValue() = 0;
    virtual ObjectValue* getObjectValue() = 0;
    static Logger& logger_m;
};

template <typename TObjectValue>
class SwitchingObjectImpl : public SwitchingObject, public TObjectValue
{
public:
    SwitchingObjectImpl() : TObjectValue(false) {};
    virtual ~SwitchingObjectImpl() {};

    virtual ObjectValue* createObjectValue(const std::string& value) { return new TObjectValue(value); };
    virtual void setValue(const std::string& value) {
        TObjectValue val(value);
        Object::setValue(&val);
    };
    virtual std::string getType() { return TObjectValue::getType(); };

    void setBoolValue(bool value) {
        TObjectValue val(value);
        Object::setValue(&val);
    };
    bool getBoolValue() { get(); return TObjectValue::value_m; };
protected:
    virtual bool set(bool value) { return TObjectValue::set(value); };
    virtual bool set(ObjectValue* value) { return TObjectValue::set(value); };
    virtual bool set(double value) { return TObjectValue::set(value); };
    virtual bool getBoolObjectValue() { return TObjectValue::value_m; };
    virtual ObjectValue* getObjectValue() { return static_cast<TObjectValue*>(this); };
    static Logger& logger_m;
};

class SwitchingObjectValue : public ObjectValue
{
public:
    SwitchingObjectValue(const std::string& value) { init(value); };
    SwitchingObjectValue(bool value) : value_m(value) {};
    virtual ~SwitchingObjectValue() {};
    void init (const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual std::string toLogString();
    virtual double toNumber();
    virtual std::string getType() { return "1.001"; };
    virtual std::string getValueString(bool value) { return value ? "on" : "off"; };
    virtual std::string getLogString(bool value) { return value ? "t" : "f"; };
protected:
    SwitchingObjectValue() : value_m(false) {};
    virtual bool set(bool value);
    virtual bool set(ObjectValue* value);
    virtual bool set(double value);
    bool value_m;
};
typedef SwitchingObjectImpl<SwitchingObjectValue> SwitchingSwitchObject;
struct SwitchingType {
    const char *type;
    const char *valueTrue;
    const char *valueFalse;
};
static const SwitchingType SwitchingTypes[] = {
    {"1.xxx", "1", "0"},
    {"1.001", "on", "off"},
    {"1.002", "true", "false"},
    {"1.003", "enable", "disable"},
    {"1.004", "ramp", "no ramp"},
    {"1.005", "alarm", "no alarm"},
    {"1.006", "high", "low"},
    {"1.007", "increase", "decrease"},
    {"1.008", "down", "up"},
    {"1.009", "close", "open"},
    {"1.010", "start", "stop"},
    {"1.011", "active", "inactive"},
    {"1.012", "inverted", "not inverted"},
    {"1.013", "cyclically", "start stop"},
    {"1.014", "calculated", "fixed"}
};

template <int eis>
class SwitchingImplObjectValue : public SwitchingObjectValue
{
public:
    SwitchingImplObjectValue(const std::string& value) { init(value); };
    SwitchingImplObjectValue(bool value) : SwitchingObjectValue(value) {};
    virtual ~SwitchingImplObjectValue() {};
    virtual std::string getType() { return SwitchingTypes[eis].type; };
    virtual std::string getValueString(bool value) { return value ? SwitchingTypes[eis].valueTrue : SwitchingTypes[eis].valueFalse; };
};

template <typename TObjectValue>
class SwitchingControlObject : public Object, public TObjectValue
{
public:
    SwitchingControlObject() : TObjectValue(false, false) {};
    virtual ~SwitchingControlObject() {};

    virtual ObjectValue* createObjectValue(const std::string& value) { return new TObjectValue(value); };
    virtual void setValue(const std::string& value) {
        TObjectValue val(value);
        Object::setValue(&val);
    };
    virtual std::string getType() { return TObjectValue::getType(); };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src) {
        int newValue;
        if (len == 2)
            newValue = (buf[1] & 0x3F);
        else
            newValue = buf[2];

        if (set(newValue) || forceUpdate())
            onUpdate();
    };
    virtual void doSend(bool isWrite) {
		BufferBuilder builder(2, Object::logger_m);
        builder << 0 << ((isWrite ? 0x80 : 0x40) | (TObjectValue::control_m ? 2 : 0) | (TObjectValue::value_m ? 1 : 0));
        getKnxConnection()->write(getGad(), builder.getBuffer(), 2);
    };
    void setBoolValue(bool value) {
        TObjectValue val(value, true);
        Object::setValue(&val);
    };
    bool getBoolValue() { get(); return TObjectValue::value_m; };
    bool getControlValue() { get(); return TObjectValue::control_m; };
protected:
    virtual bool set(bool value, bool control) { return TObjectValue::set(value, control); };
    virtual bool set(ObjectValue* value) { return TObjectValue::set(value); };
    virtual bool set(double value) { return TObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<TObjectValue*>(this); };
    // static Logger& logger_m;
};

class SwitchingControlObjectValue : public ObjectValue
{
public:
    SwitchingControlObjectValue(const std::string& value) { init(value); };
    SwitchingControlObjectValue(bool value, bool control) : value_m(value), control_m(control) {};
    virtual ~SwitchingControlObjectValue() {};
    void init(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual std::string toLogString();
    virtual double toNumber();
    virtual std::string getType() { return "2.001"; };
    virtual std::string getValueString(bool value) { return value ? "on" : "off"; };
    virtual std::string getLogString(bool value) { return value ? "t" : "f"; };
protected:
    SwitchingControlObjectValue() : value_m(false), control_m(false) {};
    virtual bool set(bool value, bool control);
    virtual bool set(ObjectValue* value);
    virtual bool set(double value);
    bool value_m;
    bool control_m;
};

struct SwitchingControlType {
    const char *type;
    const char *valueTrue;
    const char *valueFalse;
};
static const SwitchingControlType SwitchingControlTypes[] = {
    {"2.xxx", "1", "0"},
    {"2.001", "on", "off"},
    {"2.002", "true", "false"},
    {"2.003", "enable", "disable"},
    {"2.004", "ramp", "no ramp"},
    {"2.005", "alarm", "no alarm"},
    {"2.006", "high", "low"},
    {"2.007", "increase", "decrease"},
    {"2.008", "down", "up"},
    {"2.009", "close", "open"},
    {"2.010", "start", "stop"},
    {"2.011", "active", "inactive"},
    {"2.012", "inverted", "not inverted"}
};

template <int eis>
class SwitchingControlImplObjectValue : public SwitchingControlObjectValue
{
public:
    SwitchingControlImplObjectValue(const std::string& value)  { init(value); };
    SwitchingControlImplObjectValue(bool value, bool control) : SwitchingControlObjectValue(value, control) {};
    virtual ~SwitchingControlImplObjectValue() {};
    virtual std::string getType() { return SwitchingControlTypes[eis].type; };
    virtual std::string getValueString(bool value) { return value ? SwitchingControlTypes[eis].valueTrue : SwitchingControlTypes[eis].valueFalse; };
};

class StepDirObjectValue : public ObjectValue
{
public:
    virtual ~StepDirObjectValue() {};
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString() = 0;
    virtual double toNumber();
protected:
    virtual bool set(ObjectValue* value);
    virtual bool set(double value);
    friend class StepDirObject;
    StepDirObjectValue() : direction_m(0), stepcode_m(0) {};
    StepDirObjectValue(int direction, int stepcode) : direction_m(direction), stepcode_m(stepcode) {};
    int direction_m;
    int stepcode_m;
};

class StepDirObject : public Object
{
public:
    StepDirObject();
    virtual ~StepDirObject();

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getType() = 0;

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual void setStepValue(int direction, int stepcode) = 0;
protected:
    virtual bool set(ObjectValue* value) = 0;
    virtual bool set(double value) = 0;
    virtual bool setStep(int direction, int stepcode) = 0;
    virtual int getDirection() = 0;
    virtual int getStepCode() = 0;
    static Logger& logger_m;
};

class DimmingObjectValue : public StepDirObjectValue
{
public:
    DimmingObjectValue(const std::string& value);
    virtual ~DimmingObjectValue() {};
    virtual std::string toString();
protected:
    DimmingObjectValue(int direction, int stepcode) : StepDirObjectValue(direction, stepcode) {};
};

class DimmingObject : public StepDirObject, public DimmingObjectValue
{
public:
    DimmingObject() : DimmingObjectValue(0, 0) {};
    virtual ~DimmingObject() {};
    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "3.007"; };
    virtual void setStepValue(int direction, int stepcode);
protected:
    virtual bool set(ObjectValue* value) { return DimmingObjectValue::set(value); };
    virtual bool set(double value) { return DimmingObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<DimmingObjectValue*>(this); };
    virtual bool setStep(int direction, int stepcode) { if (direction_m != direction || stepcode_m != stepcode) { direction_m = direction; stepcode_m = stepcode; return true; } return false; };
    virtual int getDirection() { return direction_m; };
    virtual int getStepCode() { return stepcode_m; };
    static Logger& logger_m;
};

class BlindsObjectValue : public StepDirObjectValue
{
public:
    BlindsObjectValue(const std::string& value);
    virtual ~BlindsObjectValue() {};
    virtual std::string toString();
protected:
    BlindsObjectValue(int direction, int stepcode) : StepDirObjectValue(direction, stepcode) {};
};

class BlindsObject : public StepDirObject, public BlindsObjectValue
{
public:
    BlindsObject() : BlindsObjectValue(0, 0) {};
    virtual ~BlindsObject() {};
    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "3.008"; };
    virtual void setStepValue(int direction, int stepcode);
protected:
    virtual bool set(ObjectValue* value) { return BlindsObjectValue::set(value); };
    virtual bool set(double value) { return BlindsObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<BlindsObjectValue*>(this); };
    virtual bool setStep(int direction, int stepcode) { if (direction_m != direction || stepcode_m != stepcode) { direction_m = direction; stepcode_m = stepcode; return true; } return false; };
    virtual int getDirection() { return direction_m; };
    virtual int getStepCode() { return stepcode_m; };
    static Logger& logger_m;
};

class TimeObjectValue : public ObjectValue
{
public:
    TimeObjectValue(const std::string& value);
    virtual ~TimeObjectValue() {};
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual double toNumber();
    void getTimeValue(int *wday, int *hour, int *min, int *sec);
protected:
    virtual bool set(ObjectValue* value);
    virtual bool set(double value);
    int wday_m;
    int hour_m;
    int min_m;
    int sec_m;
    friend class TimeObject;
    TimeObjectValue(int wday, int hour, int min, int sec) : wday_m(wday), hour_m(hour), min_m(min), sec_m(sec) {};
};

class TimeObject : public Object, public TimeObjectValue
{
public:
    TimeObject();
    virtual ~TimeObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "10.001"; };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    void setTime(time_t time);
    void setTime(int wday, int hour, int min, int sec);
    void getTime(int *wday, int *hour, int *min, int *sec);
protected:
    virtual bool set(ObjectValue* value) { return TimeObjectValue::set(value); };
    virtual bool set(double value) { return TimeObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<TimeObjectValue*>(this); };
   static Logger& logger_m;
};

class DateObjectValue : public ObjectValue
{
public:
    DateObjectValue(const std::string& value);
    virtual ~DateObjectValue() {};
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual double toNumber();
    void getDateValue(int *day, int *month, int *year);
protected:
    virtual bool set(ObjectValue* value);
    virtual bool set(double value);
    int day_m;
    int month_m;
    int year_m;
    friend class DateObject;
    DateObjectValue(int day, int month, int year) : day_m(day), month_m(month), year_m(year) {};
};

class DateObject : public Object, public DateObjectValue
{
public:
    DateObject();
    virtual ~DateObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "11.001"; };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    void setDate(time_t time);
    void setDate(int day, int month, int year);
    void getDate(int *day, int *month, int *year);
protected:
    virtual bool set(ObjectValue* value) { return DateObjectValue::set(value); };
    virtual bool set(double value) { return DateObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<DateObjectValue*>(this); };
    static Logger& logger_m;
};

class ValueObjectValue : public ObjectValue
{
public:
    ValueObjectValue(const std::string& value) { init(value); };
    virtual ~ValueObjectValue() {};
    void init(const std::string& value);
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual std::string toLogString();
    virtual double toNumber();
    virtual void setPrecision(std::string precision);
    virtual std::string getPrecision();
    virtual double roundToKnxPrecision(double value);
    virtual std::string getType() = 0;
    virtual double getBound(bool upper) = 0;
protected:
    virtual bool set(ObjectValue* value);
    virtual bool set(double value);
    double value_m;
    double precision_m;
    friend class ValueObject;
    ValueObjectValue(double value) : value_m(value), precision_m(0) {};
    ValueObjectValue() : value_m(0), precision_m(0) {};
};

class ValueObject : public Object
{
public:
    ValueObject();
    virtual ~ValueObject();

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getType() = 0;

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    void setFloatValue(double value) = 0;
    double getFloatValue() = 0;
protected:
    virtual bool set(ObjectValue* value) = 0;
    virtual bool set(double value) = 0;
    virtual ObjectValue* getObjectValue() = 0;
    static Logger& logger_m;
};

template <typename TObjectValue>
class ValueObjectImpl : public ValueObject, public TObjectValue
{
public:
    ValueObjectImpl() : TObjectValue(0) {};
    virtual ~ValueObjectImpl() {};

    virtual ObjectValue* createObjectValue(const std::string& value) { return new TObjectValue(value); };
    virtual void setValue(const std::string& value) {
        TObjectValue val(value);
        Object::setValue(&val);
    };
    virtual std::string getType() { return TObjectValue::getType(); };

    void setFloatValue(double value) {
        TObjectValue val(value);
        Object::setValue(&val);
    };
    double getFloatValue() { get(); return TObjectValue::value_m; };
protected:
    virtual bool set(bool value) { return TObjectValue::set(value); };
    virtual bool set(ObjectValue* value) { return TObjectValue::set(value); };
    virtual bool set(double value) { return TObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<TObjectValue*>(this); };
    static Logger& logger_m;
};

struct FloatValueType {
    const char *type;
    double valueMin;
    double valueMax;
};
static const FloatValueType FloatValueTypes[] = {
    {"9.xxx", -671088.64, 670760.96},
    {"9.001", -273, 670760},    /* Temperature in °C */
    {"9.002", -670760, 670760}, /* Temperature difference in Kelvin */
    {"9.003", -670760, 670760}, /* Heating rate in K/h */
    {"9.004", 0, 670760},       /* Illuminance in lux */
    {"9.005", 0, 670760},       /* (Wind) Speed in m/s */
    {"9.006", 0, 670760},       /* Pressure in Pa */
    {"9.007", 0, 670760},       /* Humidity in % */
    {"9.008", 0, 670760},       /* Air Quality in ppm */
    {0, 0, 0},
    {"9.010", -670760, 670760}, /* Time value in seconds */
    {"9.011", -670760, 670760}, /* Time value in milliseconds */
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {"9.020", -670760, 670760}, /* Voltage in mV */
    {"9.021", -670760, 670760}, /* Current in mA */
    {"9.022", -670760, 670760}, /* Power Density in W/m² */
    {"9.023", -670760, 670760}, /* Kelvin Per Cent K/% */
    {"9.024", -670760, 670760}, /* Power in kW */
    {"9.025", -670760, 670760}, /* Volume Flow in l/h */
    {"9.026", -671088.64, 670760.96}, /* Rain Amount in l/m² */
    {"9.027", -459.6, 670760.96}, /* Temperature in °F */
    {"9.028", 0, 670760.96}     /* (Wind) Speed in km/h */
};

template <int subtype>
class ValueImplObjectValue : public ValueObjectValue
{
public:
    ValueImplObjectValue(const std::string& value) { init(value); };
    ValueImplObjectValue(double value) : ValueObjectValue(value) {};
    virtual ~ValueImplObjectValue() {};
    virtual std::string getType() { return FloatValueTypes[subtype].type; };
    virtual double getBound(bool upper) { return upper ? FloatValueTypes[subtype].valueMax : FloatValueTypes[subtype].valueMin; };
};

typedef ValueImplObjectValue<0> ValueObjectValue0;
typedef ValueObjectImpl<ValueImplObjectValue<0> > ValueObject0;

class ValueObject32Value : public ValueObjectValue
{
public:
    ValueObject32Value(const std::string& value);
    virtual ~ValueObject32Value() {};
    virtual std::string toString();
    virtual double roundToKnxPrecision(double value) { return value; };
    virtual std::string getType() { return "14.xxx"; };
    virtual double getBound(bool upper) { return upper ? DBL_MAX : DBL_MIN; };
protected:
    ValueObject32Value(double value) : ValueObjectValue(value) {};
    ValueObject32Value() {};
};

class ValueObject32 : public Object, public ValueObject32Value
{
public:
    ValueObject32() {};
    virtual ~ValueObject32() {};

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "14.xxx"; };

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    virtual bool set(ObjectValue* value) { return ValueObject32Value::set(value); };
    virtual bool set(double value) { return ValueObject32Value::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<ValueObject32Value*>(this); };
    static Logger& logger_m;
private:
    typedef union {
        uint32_t u32;
        float fl;
    } convfloat;
};

class UIntObjectValue : public ObjectValue
{
public:
    UIntObjectValue(const std::string& value);
    virtual ~UIntObjectValue() {};
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual std::string toLogString();
    virtual double toNumber();
protected:
    virtual bool set(ObjectValue* value);
    uint32_t value_m;
    UIntObjectValue(uint32_t value) : value_m(value) {};
    UIntObjectValue() : value_m(0) {};
};

class UIntObject : public Object
{
public:
    UIntObject();
    virtual ~UIntObject();

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getType() = 0;

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src) = 0;
    virtual void doSend(bool isWrite) = 0;
    void setIntValue(uint32_t value);
    uint32_t getIntValue();
protected:
    virtual bool set(ObjectValue* value) = 0;
    virtual bool set(double value) { return setInt(static_cast<uint32_t>(value)); };
    virtual bool setInt(uint32_t value) = 0;
    virtual uint32_t getInt() = 0;
    static Logger& logger_m;
};

class U8ObjectValue : public UIntObjectValue
{
public:
    U8ObjectValue(const std::string& value);
    virtual ~U8ObjectValue() {};
    virtual std::string toString();
    virtual std::string toLogString();
protected:
    U8ObjectValue(uint32_t value) : UIntObjectValue(value) {};
    U8ObjectValue() {};
};

class U8ImplObject : public UIntObject
{
public:
    U8ImplObject();
    virtual ~U8ImplObject();

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getType() = 0;
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
protected:
    static Logger& logger_m;
};

class U8Object : public U8ImplObject, public U8ObjectValue
{
public:
    U8Object();
    virtual ~U8Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "5.xxx"; };
    virtual std::string toString() { return U8ObjectValue::toString(); };
    virtual std::string toLogString() { return U8ObjectValue::toLogString(); };
protected:
    virtual bool set(ObjectValue* value) { return U8ObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<U8ObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class U8CountObject : public U8Object
{
public:
    U8CountObject() {};
    virtual ~U8CountObject() {};

    virtual std::string getType() { return "5.010"; };
};

class ScalingObjectValue : public U8ObjectValue
{
public:
    ScalingObjectValue(const std::string& value);
    virtual ~ScalingObjectValue() {};
    virtual std::string toString();
    virtual std::string toLogString();
protected:
    ScalingObjectValue(uint32_t value) : U8ObjectValue(value) {};
};

class ScalingObject : public U8ImplObject, public ScalingObjectValue
{
public:
    ScalingObject();
    virtual ~ScalingObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "5.001"; };
    virtual std::string toString() { return ScalingObjectValue::toString(); };
    virtual std::string toLogString() { return ScalingObjectValue::toLogString(); };
protected:
    virtual bool set(ObjectValue* value) { return ScalingObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<ScalingObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class AngleObjectValue : public U8ObjectValue
{
public:
    AngleObjectValue(const std::string& value);
    virtual std::string toString();
protected:
    AngleObjectValue(uint32_t value) : U8ObjectValue(value) {};
};

class AngleObject : public U8ImplObject, public AngleObjectValue
{
public:
    AngleObject();
    virtual ~AngleObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "5.003"; };
    virtual std::string toString() { return AngleObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return AngleObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<AngleObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class HeatingModeObjectValue : public U8ObjectValue
{
public:
    HeatingModeObjectValue(const std::string& value);
    virtual std::string toString();
protected:
    HeatingModeObjectValue() {};
    HeatingModeObjectValue(uint32_t value) : U8ObjectValue(value) {};
};

class HeatingModeObject : public U8ImplObject, public HeatingModeObjectValue
{
public:
    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "20.102"; };
    virtual std::string toString() { return HeatingModeObjectValue::toString(); };
    virtual std::string toLogString() { return HeatingModeObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return HeatingModeObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<HeatingModeObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class Latin1CharObjectValue : public U8ObjectValue
{
public:
    Latin1CharObjectValue(const std::string& value);
    virtual std::string toString();
protected:
    Latin1CharObjectValue(uint32_t value) : U8ObjectValue(value) {};
};

class AsciiCharObjectValue : public U8ObjectValue
{
public:
    AsciiCharObjectValue(const std::string& value);
    virtual std::string toString();
protected:
    AsciiCharObjectValue(uint32_t value) : U8ObjectValue(value) {};
};

class Latin1CharObject : public U8ImplObject, public Latin1CharObjectValue
{
public:
    Latin1CharObject();
    virtual ~Latin1CharObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "4.002"; };
    virtual std::string toString() { return Latin1CharObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return Latin1CharObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<Latin1CharObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class AsciiCharObject : public U8ImplObject, public AsciiCharObjectValue
{
public:
    AsciiCharObject();
    virtual ~AsciiCharObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "4.001"; };
    virtual std::string toString() { return AsciiCharObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return AsciiCharObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<AsciiCharObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
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
};

class U16Object : public UIntObject, public U16ObjectValue
{
public:
    U16Object();
    virtual ~U16Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "7.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return U16ObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return U16ObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<U16ObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class U32ObjectValue : public UIntObjectValue
{
public:
    U32ObjectValue(const std::string& value);
    virtual ~U32ObjectValue() {};
    virtual std::string toString();
    virtual std::string toLogString();
protected:
    U32ObjectValue(uint32_t value) : UIntObjectValue(value) {};
    U32ObjectValue() {};
};

class U32Object : public UIntObject, public U32ObjectValue
{
public:
    U32Object();
    virtual ~U32Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "12.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return U32ObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return U32ObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<U32ObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class RGBObjectValue : public UIntObjectValue
{
public:
    RGBObjectValue(const std::string& value);
    virtual ~RGBObjectValue() {};
    virtual std::string toString();
protected:
    RGBObjectValue(uint32_t value) : UIntObjectValue(value) {};
    RGBObjectValue() {};
};

class RGBObject : public UIntObject, public RGBObjectValue
{
public:
    RGBObject();
    virtual ~RGBObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "232.600"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return RGBObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return RGBObjectValue::set(value); };
    virtual bool setInt(uint32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<RGBObjectValue*>(this); };
    virtual uint32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class IntObjectValue : public ObjectValue
{
public:
    IntObjectValue(const std::string& value);
    virtual ~IntObjectValue() {};
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual std::string toLogString();
    virtual double toNumber();
protected:
    virtual bool set(ObjectValue* value);
    int32_t value_m;
    IntObjectValue(int32_t value) : value_m(value) {};
    IntObjectValue() {};
};

class IntObject : public Object
{
public:
    IntObject();
    virtual ~IntObject();

    virtual ObjectValue* createObjectValue(const std::string& value) = 0;
    virtual void setValue(const std::string& value) = 0;
    virtual std::string getType() = 0;

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src) = 0;
    virtual void doSend(bool isWrite) = 0;
    void setIntValue(int32_t value);
    int32_t getIntValue();
protected:
    virtual bool set(ObjectValue* value) = 0;
    virtual bool set(double value) { return setInt(static_cast<int32_t>(value)); };
    virtual bool setInt(int32_t value) = 0;
    virtual int32_t getInt() = 0;
    static Logger& logger_m;
};

class S8ObjectValue : public IntObjectValue
{
public:
    S8ObjectValue(const std::string& value);
    virtual ~S8ObjectValue() {};
    virtual std::string toString();
    virtual std::string toLogString();
protected:
    S8ObjectValue(int32_t value) : IntObjectValue(value) {};
    S8ObjectValue() {};
};

class S8Object : public IntObject, public S8ObjectValue
{
public:
    S8Object();
    virtual ~S8Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "6.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return S8ObjectValue::toString(); };
    virtual std::string toLogString() { return S8ObjectValue::toLogString(); };
protected:
    virtual bool set(ObjectValue* value) { return S8ObjectValue::set(value); };
    virtual bool setInt(int32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<S8ObjectValue*>(this); };
    virtual int32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class S16ObjectValue : public IntObjectValue
{
public:
    S16ObjectValue(const std::string& value);
    virtual ~S16ObjectValue() {};
    virtual std::string toString();
    virtual std::string toLogString();
protected:
    S16ObjectValue(int32_t value) : IntObjectValue(value) {};
    S16ObjectValue() {};
};

class S16Object : public IntObject, public S16ObjectValue
{
public:
    S16Object();
    virtual ~S16Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "8.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return S16ObjectValue::toString(); };
    virtual std::string toLogString() { return S16ObjectValue::toLogString(); };
protected:
    virtual bool set(ObjectValue* value) { return S16ObjectValue::set(value); };
    virtual bool setInt(int32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<S16ObjectValue*>(this); };
    virtual int32_t getInt() { return value_m; };
    static Logger& logger_m;
};

class S32ObjectValue : public IntObjectValue
{
public:
    S32ObjectValue(const std::string& value);
    virtual ~S32ObjectValue() {};
    virtual std::string toString();
    virtual std::string toLogString();
protected:
    S32ObjectValue(int32_t value) : IntObjectValue(value) {};
    S32ObjectValue() {};
};

class S32Object : public IntObject, public S32ObjectValue
{
public:
    S32Object();
    virtual ~S32Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "13.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return S32ObjectValue::toString(); };
    virtual std::string toLogString() { return S32ObjectValue::toLogString(); };
protected:
    virtual bool set(ObjectValue* value) { return S32ObjectValue::set(value); };
    virtual bool setInt(int32_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<S32ObjectValue*>(this); };
    virtual int32_t getInt() { return value_m; };
    static Logger& logger_m;
};

#ifdef STL_STREAM_SUPPORT_INT64
class S64ObjectValue : public ObjectValue
{
public:
    S64ObjectValue(const std::string& value);
    virtual ~S64ObjectValue() {};
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual std::string toLogString();
    virtual double toNumber();
protected:
    virtual bool set(ObjectValue* value);
    int64_t value_m;
    S64ObjectValue(int64_t value) : value_m(value) {};
    S64ObjectValue() {};
};

class S64Object : public Object, public S64ObjectValue
{
public:
    S64Object();
    virtual ~S64Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "29.xxx"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);

    void setIntValue(int64_t value);
    int64_t getIntValue();
protected:
    virtual bool set(ObjectValue* value) { return S64ObjectValue::set(value); };
    virtual bool set(double value) { return setInt(static_cast<int64_t>(value)); };
    virtual ObjectValue* getObjectValue() { return static_cast<S64ObjectValue*>(this); };
    virtual bool setInt(int64_t value) { if (value_m != value) { value_m = value; return true; } return false; };
    virtual int64_t getInt() { return value_m; };
    static Logger& logger_m;
};

class RGBWObjectValue : public S64ObjectValue
{
public:
    RGBWObjectValue(const std::string& value);
    virtual ~RGBWObjectValue() {};
    virtual std::string toString();
protected:
    RGBWObjectValue(int64_t value) : S64ObjectValue(value) {};
    RGBWObjectValue() {};
};

class RGBWObject : public S64Object, public RGBWObjectValue
{
public:
    RGBWObject();
    virtual ~RGBWObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "251.600"; };
    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return RGBWObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return RGBWObjectValue::set(value); };
    virtual bool setInt(int64_t value) { if (RGBWObjectValue::value_m != value) { RGBWObjectValue::value_m = value; return true; } return false; };
    virtual ObjectValue* getObjectValue() { return static_cast<RGBWObjectValue*>(this); };
    virtual int64_t getInt() { return RGBWObjectValue::value_m; };
    static Logger& logger_m;
};
#endif

class StringObjectValue : public ObjectValue
{
public:
    StringObjectValue(const std::string& value);
    virtual ~StringObjectValue() {};
    virtual bool equals(ObjectValue* value);
    virtual int compare(ObjectValue* value);
    virtual std::string toString();
    virtual double toNumber();

	static std::string transcode(const std::string &source, const std::string &sourceEncoding, const std::string &targetEncoding);
	static const std::string &getUTF8Encoding();
	static const std::string &getLatin1Encoding();
protected:
    virtual bool set(ObjectValue* value);
    virtual bool set(double value);
    std::string value_m;
    StringObjectValue() {};
};

class StringObject : public Object, public StringObjectValue
{
public:
    StringObject();
    virtual ~StringObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "28.001"; };

    void setStringValue(const std::string& val);

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return StringObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return StringObjectValue::set(value); };
    virtual bool set(double value) { return StringObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<StringObjectValue*>(this); };
    static Logger& logger_m;
};

class String14ObjectValue : public StringObjectValue
{
public:
    String14ObjectValue(const std::string& value);
protected:
    String14ObjectValue() {};
};

class String14AsciiObjectValue : public StringObjectValue
{
public:
    String14AsciiObjectValue(const std::string& value);
protected:
    String14AsciiObjectValue() {};
};

class String14Object : public Object, public String14ObjectValue
{
public:
    String14Object();
    virtual ~String14Object();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "16.001"; };

    void setStringValue(const std::string& val);

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return String14ObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return String14ObjectValue::set(value); };
    virtual bool set(double value) { return String14ObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<String14ObjectValue*>(this); };
    static Logger& logger_m;
};

class String14AsciiObject : public Object, public String14AsciiObjectValue
{
public:
    String14AsciiObject();
    virtual ~String14AsciiObject();

    virtual ObjectValue* createObjectValue(const std::string& value);
    virtual void setValue(const std::string& value);
    virtual std::string getType() { return "16.000"; };

    void setStringValue(const std::string& val);

    virtual void doWrite(const uint8_t* buf, int len, eibaddr_t src);
    virtual void doSend(bool isWrite);
    virtual std::string toString() { return String14AsciiObjectValue::toString(); };
protected:
    virtual bool set(ObjectValue* value) { return String14AsciiObjectValue::set(value); };
    virtual bool set(double value) { return String14AsciiObjectValue::set(value); };
    virtual ObjectValue* getObjectValue() { return static_cast<String14AsciiObjectValue*>(this); };
    static Logger& logger_m;
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
    static Logger& logger_m;
};

#endif

