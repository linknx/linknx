#include <cppunit/extensions/HelperMacros.h>
#include "objectcontroller.h"

class ObjectTest : public CppUnit::TestFixture, public ChangeListener
{
    CPPUNIT_TEST_SUITE( ObjectTest );
    CPPUNIT_TEST( testSwitchingObject );
    CPPUNIT_TEST( testSwitchingObjectWrite );
    CPPUNIT_TEST( testSwitchingObjectUpdate );
    CPPUNIT_TEST( testSwitchingExportImport );
    CPPUNIT_TEST( testDimmingObject );
    CPPUNIT_TEST( testDimmingObjectWrite );
    CPPUNIT_TEST( testDimmingObjectUpdate );
    CPPUNIT_TEST( testDimmingExportImport );
    CPPUNIT_TEST( testTimeObject );
    CPPUNIT_TEST( testTimeObjectWrite );
    CPPUNIT_TEST( testTimeObjectUpdate );
    CPPUNIT_TEST( testTimeExportImport );
    CPPUNIT_TEST( testDateObject );
    CPPUNIT_TEST( testDateObjectWrite );
    CPPUNIT_TEST( testDateObjectUpdate );
    CPPUNIT_TEST( testDateExportImport );
//    CPPUNIT_TEST(  );
//    CPPUNIT_TEST(  );
    
    CPPUNIT_TEST_SUITE_END();

private:
    bool isOnChangeCalled_m;
public:
    void setUp()
    {
        isOnChangeCalled_m = false;
    }

    void tearDown()
    {
    }

    void onChange(Object* obj)
    {
        isOnChangeCalled_m = true;
    }

    void testSwitchingObject()
    {
        const std::string on = "on";
        const std::string off = "off";
        const std::string zero = "0";
        const std::string one = "1";
        const std::string tr = "true";
        const std::string fa = "false";
        ObjectValue* val;
        SwitchingObject sw, sw2;
        sw.setValue("on");
        CPPUNIT_ASSERT(sw.getValue() == "on");
        sw.setValue("1");
        CPPUNIT_ASSERT(sw.getValue() == "on");
        sw.setValue("true");
        CPPUNIT_ASSERT(sw.getValue() == "on");

        sw2.setValue("off");
        CPPUNIT_ASSERT(sw2.getValue() == "off");
        sw2.setValue("0");
        CPPUNIT_ASSERT(sw2.getValue() == "off");
        sw2.setValue("false");
        CPPUNIT_ASSERT(sw2.getValue() == "off");

        CPPUNIT_ASSERT(sw.getBoolValue() == true);
        CPPUNIT_ASSERT(sw2.getBoolValue() == false);

        SwitchingObjectValue swval(tr);
        CPPUNIT_ASSERT(sw.equals(&swval));
        CPPUNIT_ASSERT(!sw2.equals(&swval));

        val = sw.createObjectValue(one);
        CPPUNIT_ASSERT(sw.equals(val));
        CPPUNIT_ASSERT(!sw2.equals(val));
        delete val;      

        SwitchingObjectValue swval2(zero);
        CPPUNIT_ASSERT(!sw.equals(&swval2));
        CPPUNIT_ASSERT(sw2.equals(&swval2));

        val = sw.createObjectValue(fa);
        CPPUNIT_ASSERT(!sw.equals(val));
        CPPUNIT_ASSERT(sw2.equals(val));
        delete val;      

        sw.setBoolValue(false);
        CPPUNIT_ASSERT(sw.getValue() == "off");
        sw2.setBoolValue(true);
        CPPUNIT_ASSERT(sw2.getValue() == "on");
    }

    void testSwitchingObjectWrite()
    {
        SwitchingObject sw;
        sw.setBoolValue(false);
        sw.addChangeListener(this);

        uint8_t buf[3] = {0, 0x81, 0};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        sw.onWrite(buf, 2, src);        
        CPPUNIT_ASSERT(sw.getBoolValue() == true);
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x80;
        isOnChangeCalled_m = false;
        sw.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(sw.getBoolValue() == false);
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 0x00;
        isOnChangeCalled_m = false;
        sw.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(sw.getBoolValue() == false);
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0x01;
        isOnChangeCalled_m = false;
        sw.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(sw.getBoolValue() == true);
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);
    }

    void testSwitchingObjectUpdate()
    {
        SwitchingObject sw;
        sw.addChangeListener(this);

        isOnChangeCalled_m = false;
        sw.setValue("on");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        sw.setValue("off");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        sw.setValue("off");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        sw.removeChangeListener(this);

        isOnChangeCalled_m = false;
        sw.setValue("on");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testSwitchingExportImport()
    {
        SwitchingObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<SwitchingObject*>(res));
        delete res;
    }
    
    void testDimmingObject()
    {
        const std::string up = "up";
        const std::string down = "down";
        const std::string stop = "stop";
        ObjectValue* val;
        DimmingObject dim;
        dim.setValue("stop");
        CPPUNIT_ASSERT(dim.getValue() == "stop");
        dim.setValue("up");
        CPPUNIT_ASSERT(dim.getValue() == "up");
        dim.setValue("down");
        CPPUNIT_ASSERT(dim.getValue() == "down");
        dim.setValue("up:2");
        CPPUNIT_ASSERT(dim.getValue() == "up:2");
        dim.setValue("down:7");
        CPPUNIT_ASSERT(dim.getValue() == "down:7");
        dim.setValue("up:1");
        CPPUNIT_ASSERT(dim.getValue() == "up");
        CPPUNIT_ASSERT_THROW(dim.setValue("down:0"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(dim.setValue("up:8"), ticpp::Exception);

        DimmingObjectValue dimval("up");
        DimmingObjectValue dimval2("up:3");
        DimmingObjectValue dimval3("down");
        DimmingObjectValue dimval4("stop");
        CPPUNIT_ASSERT(dim.equals(&dimval));
        CPPUNIT_ASSERT(!dim.equals(&dimval2));
        CPPUNIT_ASSERT(!dim.equals(&dimval3));
        CPPUNIT_ASSERT(!dim.equals(&dimval4));

        dim.setValue("up:3");

        CPPUNIT_ASSERT(!dim.equals(&dimval));
        CPPUNIT_ASSERT(dim.equals(&dimval2));
        CPPUNIT_ASSERT(!dim.equals(&dimval3));
        CPPUNIT_ASSERT(!dim.equals(&dimval4));

        dim.setValue("down");

        CPPUNIT_ASSERT(!dim.equals(&dimval));
        CPPUNIT_ASSERT(!dim.equals(&dimval2));
        CPPUNIT_ASSERT(dim.equals(&dimval3));
        CPPUNIT_ASSERT(!dim.equals(&dimval4));

        dim.setValue("stop");

        CPPUNIT_ASSERT(!dim.equals(&dimval));
        CPPUNIT_ASSERT(!dim.equals(&dimval2));
        CPPUNIT_ASSERT(!dim.equals(&dimval3));
        CPPUNIT_ASSERT(dim.equals(&dimval4));

        DimmingObject dim2;
        dim2.setValue("down:5");

        val = dim.createObjectValue("down:5");
        CPPUNIT_ASSERT(!dim.equals(val));
        CPPUNIT_ASSERT(dim2.equals(val));
        delete val;      

        val = dim.createObjectValue("stop");
        CPPUNIT_ASSERT(dim.equals(val));
        CPPUNIT_ASSERT(!dim2.equals(val));
        delete val;      
    }

    void testDimmingObjectWrite()
    {
        DimmingObject dim;
        dim.setValue("stop");
        dim.addChangeListener(this);

        uint8_t buf[3] = {0, 0x8b, 0};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);        
        CPPUNIT_ASSERT(dim.getValue() == "up:3");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x80;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "stop");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 0x08;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(dim.getValue() == "stop");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0x04;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(dim.getValue() == "down:4");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x8f;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "up:7");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x81;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "down");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x89;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "up");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);
    }

    void testDimmingObjectUpdate()
    {
        DimmingObject dim;
        dim.addChangeListener(this);

        isOnChangeCalled_m = false;
        dim.setValue("down");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        dim.setValue("up");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        dim.setValue("up:1");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        isOnChangeCalled_m = false;
        dim.setValue("stop");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        dim.setValue("down:7");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        dim.removeChangeListener(this);

        isOnChangeCalled_m = false;
        dim.setValue("up:3");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testDimmingExportImport()
    {
        DimmingObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<DimmingObject*>(res));
        delete res;
    }
    
    void testTimeObject()
    {
        ObjectValue* val;
        TimeObject t, t2;
        int wday, hour, min, sec;
        t.setValue("00:00:00");
        CPPUNIT_ASSERT(t.getValue() == "0:0:0");
        t2.setValue("now");
        CPPUNIT_ASSERT(t2.getValue() != "0:0:0" || (sleep(2) == 0 && t2.getValue() != "0:0:0"));

        t.setValue("17:30:05");
        CPPUNIT_ASSERT(t.getValue() == "17:30:5");
        t2.setValue("18:30:29");
        CPPUNIT_ASSERT(t2.getValue() == "18:30:29");

        t.getTime(&wday, &hour, &min, &sec);
        CPPUNIT_ASSERT_EQUAL(0, wday);
        CPPUNIT_ASSERT_EQUAL(17, hour);
        CPPUNIT_ASSERT_EQUAL(30, min);
        CPPUNIT_ASSERT_EQUAL(05, sec);
        t2.getTime(&wday, &hour, &min, &sec);
        CPPUNIT_ASSERT_EQUAL(0, wday);
        CPPUNIT_ASSERT_EQUAL(18, hour);
        CPPUNIT_ASSERT_EQUAL(30, min);
        CPPUNIT_ASSERT_EQUAL(29, sec);

        CPPUNIT_ASSERT_THROW(t.setValue("24:30:00"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("23:-1:10"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("23:-1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("23:60:0"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("0:50:111"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("now:10:50"), ticpp::Exception);

        TimeObjectValue tval("17:30:5");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("17:30:05");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        TimeObjectValue tval2("18:30:29");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("18:30:29");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setTime(1, 20, 45, 0);
        CPPUNIT_ASSERT(t.getValue() == "20:45:0");
        t.getTime(&wday, &hour, &min, &sec);
        CPPUNIT_ASSERT_EQUAL(1, wday);
        CPPUNIT_ASSERT_EQUAL(20, hour);
        CPPUNIT_ASSERT_EQUAL(45, min);
        CPPUNIT_ASSERT_EQUAL(0, sec);
    }

    void testTimeObjectWrite()
    {
        TimeObject t;
        t.setValue("22:01:00");
        t.addChangeListener(this);

        uint8_t buf[5] = {0, 0x80, 0, 0, 0};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);        
        CPPUNIT_ASSERT(t.getValue() == "0:0:0");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 23;
        buf[3] = 10;
        buf[4] = 4;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "23:10:4");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);        
        CPPUNIT_ASSERT(t.getValue() == "23:10:4");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 20;
        buf[3] = 10;
        buf[4] = 4;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "20:10:4");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 20 | (3 << 5);
        buf[3] = 10;
        buf[4] = 4;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "20:10:4");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        int wday, hour, min, sec;
        t.getTime(&wday, &hour, &min, &sec);
        CPPUNIT_ASSERT_EQUAL(3, wday);
        CPPUNIT_ASSERT_EQUAL(20, hour);
        CPPUNIT_ASSERT_EQUAL(10, min);
        CPPUNIT_ASSERT_EQUAL(4, sec);
    }

    void testTimeObjectUpdate()
    {
        TimeObject t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("6:30:00");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("6:30:01");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("6:30:01");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("7:20:00");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testTimeExportImport()
    {
        TimeObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<TimeObject*>(res));
        delete res;
    }

    void testDateObject()
    {
        ObjectValue* val;
        DateObject t, t2;
        int day, month, year;
        t.setValue("1900-01-01");
        CPPUNIT_ASSERT(t.getValue() == "1900-1-1");
        t2.setValue("now");
        CPPUNIT_ASSERT(t2.getValue() != "1900-1-1");

        t.setValue("2007-10-31");
        CPPUNIT_ASSERT(t.getValue() == "2007-10-31");
        t2.setValue("2006-10-05");
        CPPUNIT_ASSERT(t2.getValue() == "2006-10-5");

        t.getDate(&day, &month, &year);
        CPPUNIT_ASSERT_EQUAL(31, day);
        CPPUNIT_ASSERT_EQUAL(10, month);
        CPPUNIT_ASSERT_EQUAL(2007, year);
        t2.getDate(&day, &month, &year);
        CPPUNIT_ASSERT_EQUAL(5, day);
        CPPUNIT_ASSERT_EQUAL(10, month);
        CPPUNIT_ASSERT_EQUAL(2006, year);

        CPPUNIT_ASSERT_THROW(t.setValue("2007:11:5"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("-1-10-5"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("2007-13-5"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("2007-0-5"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("2007-10-0"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("2007-10-32"), ticpp::Exception);

        DateObjectValue tval("2007-10-31");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("2007-10-31");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        DateObjectValue tval2("2006-10-5");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("2006-10-5");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setDate(15, 8, 2007);
        CPPUNIT_ASSERT(t.getValue() == "2007-8-15");
        t.getDate(&day, &month, &year);
        CPPUNIT_ASSERT_EQUAL(15, day);
        CPPUNIT_ASSERT_EQUAL(8, month);
        CPPUNIT_ASSERT_EQUAL(2007, year);
    }

    void testDateObjectWrite()
    {
        DateObject t;
        t.setValue("2007-8-15");
        t.addChangeListener(this);

        uint8_t buf[6] = {0, 0x80, 1, 1, 0};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);        
        CPPUNIT_ASSERT(t.getValue() == "2000-1-1");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 23;
        buf[3] = 10;
        buf[4] = 99;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "1999-10-23");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);        
        CPPUNIT_ASSERT(t.getValue() == "1999-10-23");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 20;
        buf[3] = 10;
        buf[4] = 7;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "2007-10-20");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        int day, month, year;
        t.getDate(&day, &month, &year);
        CPPUNIT_ASSERT_EQUAL(20, day);
        CPPUNIT_ASSERT_EQUAL(10, month);
        CPPUNIT_ASSERT_EQUAL(2007, year);
    }

    void testDateObjectUpdate()
    {
        DateObject t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("2007-5-30");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("2007-5-29");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("2007-05-29");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("2007-6-16");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testDateExportImport()
    {
        DateObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<DateObject*>(res));
        delete res;
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION( ObjectTest );
