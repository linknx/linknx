#include <cppunit/extensions/HelperMacros.h>
#include <cstdlib>
#include "objectcontroller.h"
#include "services.h"

class ObjectTest : public CppUnit::TestFixture, public ChangeListener
{
    CPPUNIT_TEST_SUITE( ObjectTest );
    CPPUNIT_TEST( testSwitchingObject );
    CPPUNIT_TEST( testSwitchingObjectWrite );
    CPPUNIT_TEST( testSwitchingObjectUpdate );
    CPPUNIT_TEST( testSwitchingExportImport );
    CPPUNIT_TEST( testSwitchingPersist );
    CPPUNIT_TEST( testDimmingObject );
    CPPUNIT_TEST( testDimmingObjectWrite );
    CPPUNIT_TEST( testDimmingObjectUpdate );
    CPPUNIT_TEST( testDimmingExportImport );
    CPPUNIT_TEST( testDimmingPersist );
    CPPUNIT_TEST( testBlindsObject );
    CPPUNIT_TEST( testBlindsObjectWrite );
    CPPUNIT_TEST( testBlindsObjectUpdate );
    CPPUNIT_TEST( testBlindsExportImport );
    CPPUNIT_TEST( testBlindsPersist );
    CPPUNIT_TEST( testTimeObject );
    CPPUNIT_TEST( testTimeObjectWrite );
    CPPUNIT_TEST( testTimeObjectUpdate );
    CPPUNIT_TEST( testTimeExportImport );
    CPPUNIT_TEST( testTimePersist );
    CPPUNIT_TEST( testDateObject );
    CPPUNIT_TEST( testDateObjectWrite );
    CPPUNIT_TEST( testDateObjectUpdate );
    CPPUNIT_TEST( testDateExportImport );
    CPPUNIT_TEST( testDatePersist );
    CPPUNIT_TEST( testValueObject );
    CPPUNIT_TEST( testValueObjectWrite );
    CPPUNIT_TEST( testValueObjectUpdate );
    CPPUNIT_TEST( testValueExportImport );
    CPPUNIT_TEST( testValuePersist );
    CPPUNIT_TEST( testValueObject32Write );
    CPPUNIT_TEST( testU8Object );
    CPPUNIT_TEST( testU8ObjectWrite );
    CPPUNIT_TEST( testU8ObjectUpdate );
    CPPUNIT_TEST( testU8ExportImport );
    CPPUNIT_TEST( testU8Persist );
    CPPUNIT_TEST( testScalingObject );
    CPPUNIT_TEST( testScalingObjectWrite );
    CPPUNIT_TEST( testScalingObjectUpdate );
    CPPUNIT_TEST( testScalingExportImport );
    CPPUNIT_TEST( testScalingPersist );
    CPPUNIT_TEST( testHeatingModeObject );
    CPPUNIT_TEST( testHeatingModeObjectWrite );
    CPPUNIT_TEST( testHeatingModeObjectUpdate );
    CPPUNIT_TEST( testHeatingModeExportImport );
    CPPUNIT_TEST( testHeatingModePersist );
    CPPUNIT_TEST( testU16Object );
    CPPUNIT_TEST( testU16ObjectWrite );
    CPPUNIT_TEST( testU16ObjectUpdate );
    CPPUNIT_TEST( testU16ExportImport );
    CPPUNIT_TEST( testU16Persist );
    CPPUNIT_TEST( testU32Object );
    CPPUNIT_TEST( testU32ObjectWrite );
    CPPUNIT_TEST( testU32ObjectUpdate );
    CPPUNIT_TEST( testU32ExportImport );
    CPPUNIT_TEST( testU32Persist );
    CPPUNIT_TEST( testS8Object );
    CPPUNIT_TEST( testS8ObjectWrite );
    CPPUNIT_TEST( testS8ObjectUpdate );
    CPPUNIT_TEST( testS8ExportImport );
    CPPUNIT_TEST( testS8Persist );
    CPPUNIT_TEST( testS16Object );
    CPPUNIT_TEST( testS16ObjectWrite );
    CPPUNIT_TEST( testS16ObjectUpdate );
    CPPUNIT_TEST( testS16ExportImport );
    CPPUNIT_TEST( testS16Persist );
    CPPUNIT_TEST( testS32Object );
    CPPUNIT_TEST( testS32ObjectWrite );
    CPPUNIT_TEST( testS32ObjectUpdate );
    CPPUNIT_TEST( testS32ExportImport );
    CPPUNIT_TEST( testS32Persist );
    CPPUNIT_TEST( testS64Object );
    CPPUNIT_TEST( testS64ObjectWrite );
    CPPUNIT_TEST( testS64ObjectUpdate );
    CPPUNIT_TEST( testS64ExportImport );
    CPPUNIT_TEST( testS64Persist );
    CPPUNIT_TEST( testString14Object );
    CPPUNIT_TEST( testString14ObjectWrite );
    CPPUNIT_TEST( testString14ObjectUpdate );
    CPPUNIT_TEST( testString14ExportImport );
    CPPUNIT_TEST( testString14Persist );
    CPPUNIT_TEST( testString14AsciiObject );
    CPPUNIT_TEST( testString14AsciiObjectWrite );
    CPPUNIT_TEST( testString14AsciiObjectUpdate );
    CPPUNIT_TEST( testString14AsciiExportImport );
    CPPUNIT_TEST( testString14AsciiPersist );
    CPPUNIT_TEST( testStringObject );
    CPPUNIT_TEST( testStringObjectWrite );
    CPPUNIT_TEST( testStringObjectUpdate );
    CPPUNIT_TEST( testStringExportImport );
    CPPUNIT_TEST( testStringPersist );
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
        Services::reset();
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

        const std::string tr = "true";
        const std::string fa = "false";
        SwitchingObjectValue swvaltrue(tr);
        SwitchingObjectValue swvalfalse(fa);

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
        CPPUNIT_ASSERT_EQUAL(0, sw.compare(&swvalfalse));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 0x00;
        isOnChangeCalled_m = false;
        sw.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(sw.getBoolValue() == false);
        CPPUNIT_ASSERT_EQUAL(0, sw.compare(&swvalfalse));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0x01;
        isOnChangeCalled_m = false;
        sw.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(sw.getBoolValue() == true);
        CPPUNIT_ASSERT_EQUAL(0, sw.compare(&swvaltrue));
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
    
    void testSwitchingPersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_sw");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("on");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "on");
        res->setValue("off");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "off");
        delete res2;
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
        DimmingObjectValue dimval1("up:3");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x80;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "stop");
        DimmingObjectValue dimval2("stop");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 0x08;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(dim.getValue() == "stop");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0x04;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(dim.getValue() == "down:4");
        DimmingObjectValue dimval3("down:4");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x8f;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "up:7");
        DimmingObjectValue dimval4("up:7");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval4));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x81;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "down");
        DimmingObjectValue dimval5("down");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval5));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x89;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "up");
        DimmingObjectValue dimval6("up");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval6));
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
    
    void testDimmingPersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_dim");
        pConfig.SetAttribute("type", "EIS2");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("up");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "up");
        res->setValue("stop");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "stop");
        res2->setValue("down:3");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "down:3");
        delete res3;
    }
    
    void testBlindsObject()
    {
        const std::string close = "close";
        const std::string open = "open";
        const std::string stop = "stop";
        ObjectValue* val;
        BlindsObject dim;
        dim.setValue("stop");
        CPPUNIT_ASSERT(dim.getValue() == "stop");
        dim.setValue("close");
        CPPUNIT_ASSERT(dim.getValue() == "close");
        dim.setValue("open");
        CPPUNIT_ASSERT(dim.getValue() == "open");
        dim.setValue("close:2");
        CPPUNIT_ASSERT(dim.getValue() == "close:2");
        dim.setValue("open:7");
        CPPUNIT_ASSERT(dim.getValue() == "open:7");
        dim.setValue("close:1");
        CPPUNIT_ASSERT(dim.getValue() == "close");
        CPPUNIT_ASSERT_THROW(dim.setValue("open:0"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(dim.setValue("close:8"), ticpp::Exception);

        BlindsObjectValue dimval("close");
        BlindsObjectValue dimval2("close:3");
        BlindsObjectValue dimval3("open");
        BlindsObjectValue dimval4("stop");
        CPPUNIT_ASSERT(dim.equals(&dimval));
        CPPUNIT_ASSERT(!dim.equals(&dimval2));
        CPPUNIT_ASSERT(!dim.equals(&dimval3));
        CPPUNIT_ASSERT(!dim.equals(&dimval4));

        dim.setValue("close:3");

        CPPUNIT_ASSERT(!dim.equals(&dimval));
        CPPUNIT_ASSERT(dim.equals(&dimval2));
        CPPUNIT_ASSERT(!dim.equals(&dimval3));
        CPPUNIT_ASSERT(!dim.equals(&dimval4));

        dim.setValue("open");

        CPPUNIT_ASSERT(!dim.equals(&dimval));
        CPPUNIT_ASSERT(!dim.equals(&dimval2));
        CPPUNIT_ASSERT(dim.equals(&dimval3));
        CPPUNIT_ASSERT(!dim.equals(&dimval4));

        dim.setValue("stop");

        CPPUNIT_ASSERT(!dim.equals(&dimval));
        CPPUNIT_ASSERT(!dim.equals(&dimval2));
        CPPUNIT_ASSERT(!dim.equals(&dimval3));
        CPPUNIT_ASSERT(dim.equals(&dimval4));

        BlindsObject dim2;
        dim2.setValue("open:5");

        val = dim.createObjectValue("open:5");
        CPPUNIT_ASSERT(!dim.equals(val));
        CPPUNIT_ASSERT(dim2.equals(val));
        delete val;      

        val = dim.createObjectValue("stop");
        CPPUNIT_ASSERT(dim.equals(val));
        CPPUNIT_ASSERT(!dim2.equals(val));
        delete val;      
    }

    void testBlindsObjectWrite()
    {
        BlindsObject dim;
        dim.setValue("stop");
        dim.addChangeListener(this);

        uint8_t buf[3] = {0, 0x8b, 0};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);        
        CPPUNIT_ASSERT(dim.getValue() == "close:3");
        BlindsObjectValue dimval1("close:3");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x80;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "stop");
        BlindsObjectValue dimval2("stop");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 0x08;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(dim.getValue() == "stop");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0x04;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(dim.getValue() == "open:4");
        BlindsObjectValue dimval3("open:4");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x8f;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "close:7");
        BlindsObjectValue dimval4("close:7");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval4));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x81;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "open");
        BlindsObjectValue dimval5("open");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval5));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[1] = 0x89;
        isOnChangeCalled_m = false;
        dim.onWrite(buf, 2, src);       
        CPPUNIT_ASSERT(dim.getValue() == "close");
        BlindsObjectValue dimval6("close");
        CPPUNIT_ASSERT_EQUAL(0, dim.compare(&dimval6));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);
    }

    void testBlindsObjectUpdate()
    {
        BlindsObject dim;
        dim.addChangeListener(this);

        isOnChangeCalled_m = false;
        dim.setValue("open");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        dim.setValue("close");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        dim.setValue("close:1");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        isOnChangeCalled_m = false;
        dim.setValue("stop");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        dim.setValue("open:7");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        dim.removeChangeListener(this);

        isOnChangeCalled_m = false;
        dim.setValue("close:3");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testBlindsExportImport()
    {
        BlindsObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<BlindsObject*>(res));
        delete res;
    }
    
    void testBlindsPersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_dim");
        pConfig.SetAttribute("type", "3.008");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("close");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "close");
        res->setValue("stop");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "stop");
        res2->setValue("open:3");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "open:3");
        delete res3;
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
        CPPUNIT_ASSERT_THROW(t.setValue("0:50:11:1"), ticpp::Exception);

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

        TimeObjectValue tval3("now");
        CPPUNIT_ASSERT(tval3.toString() == "now");
        tval3.getTimeValue(&wday, &hour, &min, &sec);
        CPPUNIT_ASSERT(wday != -1);
        CPPUNIT_ASSERT(hour != -1);
        CPPUNIT_ASSERT(min != -1);
        CPPUNIT_ASSERT(sec != -1);
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
        TimeObjectValue tval1("0:0:0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 23;
        buf[3] = 10;
        buf[4] = 4;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "23:10:4");
        TimeObjectValue tval2("23:10:4");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);        
        CPPUNIT_ASSERT(t.getValue() == "23:10:4");
        TimeObjectValue tval3("23:10:4");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 20;
        buf[3] = 10;
        buf[4] = 4;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "20:10:4");
        TimeObjectValue tval4("20:10:4");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval4));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 20 | (3 << 5);
        buf[3] = 10;
        buf[4] = 4;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "20:10:4");
//      TODO: implement weekday support in TimeObjectValue
//        TimeObjectValue tval5("20:10:4");
//        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval5));
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

    void testTimePersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_time");
        pConfig.SetAttribute("type", "EIS3");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("07:25:00");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "7:25:0");
        res->setValue("23:59:59");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "23:59:59");
        res2->setValue("now");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() != "now");
        delete res3;
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
        CPPUNIT_ASSERT_THROW(t.setValue("2007-10-32-1"), ticpp::Exception);

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

        DateObjectValue tval3("now");
        CPPUNIT_ASSERT(tval3.toString() == "now");
        tval3.getDateValue(&day, &month, &year);
        CPPUNIT_ASSERT(day != -1);
        CPPUNIT_ASSERT(month != -1);
        CPPUNIT_ASSERT(year != -1);
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
        DateObjectValue tval1("2000-1-1");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 23;
        buf[3] = 10;
        buf[4] = 99;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "1999-10-23");
        DateObjectValue tval2("1999-10-23");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);        
        CPPUNIT_ASSERT(t.getValue() == "1999-10-23");
        DateObjectValue tval3("1999-10-23");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 20;
        buf[3] = 10;
        buf[4] = 7;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 5, src);       
        CPPUNIT_ASSERT(t.getValue() == "2007-10-20");
        DateObjectValue tval4("2007-10-20");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval4));
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

    void testDatePersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_date");
        pConfig.SetAttribute("type", "EIS4");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("2007-05-30");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "2007-5-30");
        res->setValue("1978-06-16");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "1978-6-16");
        res2->setValue("now");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() != "now");
        delete res3;
    }
    
    void testValueObject()
    {
        ObjectValue* val;
        ValueObject v, v2;
        v.setValue("25");
        CPPUNIT_ASSERT(v.getValue() == "25");
        v2.setValue("14.55");
        CPPUNIT_ASSERT(v2.getValue() == "14.55");

        v2.setPrecision("0.2");
        v2.setValue("14.55");
        CPPUNIT_ASSERT(v2.getValue() == "14.6");
        v2.setValue("14.451254");
        CPPUNIT_ASSERT(v2.getValue() == "14.4");
        v2.setValue("-14.451254");
        CPPUNIT_ASSERT(v2.getValue() == "-14.4");
        v2.setValue("-0.71");
        CPPUNIT_ASSERT(v2.getValue() == "-0.8");

        v2.setPrecision("3");
        v2.setValue("6");
        CPPUNIT_ASSERT(v2.getValue() == "6");
        v2.setValue("-3544.2");
        CPPUNIT_ASSERT(v2.getValue() == "-3543");

        v2.setPrecision("100");
        v2.setValue("56254");
        CPPUNIT_ASSERT(v2.getValue() == "56300");
        v2.setPrecision("0");

        v.setValue("670760.96");
        v2.setValue("-671088.64");
        CPPUNIT_ASSERT(v.getValue() == "670760.96");
        CPPUNIT_ASSERT(v2.getValue() == "-671088.64");

        CPPUNIT_ASSERT_EQUAL(670760.96, v.getFloatValue());
        CPPUNIT_ASSERT_EQUAL(-671088.64, v2.getFloatValue());

        CPPUNIT_ASSERT_THROW(v.setValue("alhfle"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(v.setValue("-671089"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(v.setValue("670761"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(v.setValue("10.1aaaa"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(v.setValue("10,5"), ticpp::Exception);

        ValueObjectValue fval("670760.96");
        CPPUNIT_ASSERT(v.equals(&fval));
        CPPUNIT_ASSERT(!v2.equals(&fval));

        val = v.createObjectValue("670760.96");
        CPPUNIT_ASSERT(v.equals(val));
        CPPUNIT_ASSERT(!v2.equals(val));
        delete val;

        ValueObjectValue fval2("-671088.64");
        CPPUNIT_ASSERT(!v.equals(&fval2));
        CPPUNIT_ASSERT(v2.equals(&fval2));

        val = v.createObjectValue("-671088.64");
        CPPUNIT_ASSERT(!v.equals(val));
        CPPUNIT_ASSERT(v2.equals(val));
        delete val;      

        v.setFloatValue(-35.24);
        CPPUNIT_ASSERT(v.getValue() == "-35.24");
        CPPUNIT_ASSERT_EQUAL(-35.24, v.getFloatValue());
    }

    void testValueObjectWrite()
    {
        ValueObject v;
        v.setValue("27.1");
        v.addChangeListener(this);

        uint8_t buf[6] = {0, 0x80, (1<<3) | ((1360 & 0x700)>>8) , (1360 & 0xff)};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        v.onWrite(buf, 4, src);        
        CPPUNIT_ASSERT(v.getValue() == "27.2");
        ValueObjectValue fval1("27.2");
        CPPUNIT_ASSERT_EQUAL(0, v.compare(&fval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = (1<<7) | (4<<3) | ((-2000 & 0x700)>>8);
        buf[3] = (-2000 & 0xff);
        isOnChangeCalled_m = false;
        v.onWrite(buf, 4, src);       
        CPPUNIT_ASSERT(v.getValue() == "-320");
        ValueObjectValue fval2("-320");
        CPPUNIT_ASSERT_EQUAL(0, v.compare(&fval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        v.onWrite(buf, 4, src);        
        CPPUNIT_ASSERT(v.getValue() == "-320");
        CPPUNIT_ASSERT_EQUAL(0, v.compare(&fval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = (1<<7) | (5<<3) | ((-1000 & 0x700)>>8);
        buf[3] = (-1000 & 0xff);
        isOnChangeCalled_m = false;
        v.onWrite(buf, 4, src);       
        CPPUNIT_ASSERT(v.getValue() == "-320");
        CPPUNIT_ASSERT_EQUAL(0, v.compare(&fval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = (1<<3) | ((1 & 0x700)>>8);
        buf[3] = (1 & 0xff);
        isOnChangeCalled_m = false;
        v.onWrite(buf, 4, src);       
        CPPUNIT_ASSERT(v.getValue() == "0.02");
        ValueObjectValue fval3("0.02");
        CPPUNIT_ASSERT_EQUAL(0, v.compare(&fval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL(0.02, v.getFloatValue());
    }

    void testValueObjectUpdate()
    {
        ValueObject v;
        v.addChangeListener(this);

        isOnChangeCalled_m = false;
        v.setValue("20.4");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        v.setValue("20.47");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        v.setValue("20.47");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        v.removeChangeListener(this);

        isOnChangeCalled_m = false;
        v.setValue("21.0");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testValueExportImport()
    {
        ValueObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<ValueObject*>(res));
        delete res;
    }

    void testValuePersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_value");
        pConfig.SetAttribute("type", "EIS5");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("21.5");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "21.5");
        res->setValue("3.1415");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "3.1415");
        res2->setValue("-2");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "-2");
        delete res3;
    }

    void testValueObject32Write()
    {
        ValueObject32 v;
        v.setValue("27.1");
        v.addChangeListener(this);

        uint8_t buf[6] = {0, 0x80, 0x40, 0x49, 0x0e, 0x56};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        v.onWrite(buf, 6, src);        
        CPPUNIT_ASSERT(v.getValue() == "3.1415");
        ValueObjectValue fval1("3.1414");
        ValueObjectValue fval2("3.1416");
        CPPUNIT_ASSERT_EQUAL(1, v.compare(&fval1));
        CPPUNIT_ASSERT_EQUAL(-1, v.compare(&fval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);
    }

    void testU8Object()
    {
        ObjectValue* val;
        U8Object t, t2;
        t.setValue("0");
        CPPUNIT_ASSERT(t.getValue() == "0");
        t2.setValue("255");
        CPPUNIT_ASSERT(t2.getValue() == "255");

        t.setValue("10");
        CPPUNIT_ASSERT(t.getValue() == "10");
        t2.setValue("240");
        CPPUNIT_ASSERT(t2.getValue() == "240");

        CPPUNIT_ASSERT_EQUAL((uint32_t)10, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((uint32_t)240, t2.getIntValue());

        CPPUNIT_ASSERT_THROW(t.setValue("-1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("256"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("30000"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("25.1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("75,6"), ticpp::Exception);

        U8ObjectValue tval("10");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("10");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        U8ObjectValue tval2("240");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("240");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(100);
        CPPUNIT_ASSERT(t.getValue() == "100");
        CPPUNIT_ASSERT_EQUAL((uint32_t)100, t.getIntValue());
    }

    void testU8ObjectWrite()
    {
        U8Object t;
        t.setValue("55");
        t.addChangeListener(this);

        uint8_t buf[4] = {0, 0x80, 66};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(t.getValue() == "66");
        U8ObjectValue tval1("66");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 74;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "74");
        U8ObjectValue tval2("74");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(t.getValue() == "74");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "0");
        U8ObjectValue tval3("0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((uint32_t)0, t.getIntValue());
    }

    void testU8ObjectUpdate()
    {
        U8Object t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("168");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("170");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testU8ExportImport()
    {
        U8Object orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<U8Object*>(res));
        delete res;
    }

    void testU8Persist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_scale");
        pConfig.SetAttribute("type", "EIS6");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("255");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "255");
        res->setValue("0");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "0");
        res2->setValue("35");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "35");
        delete res3;
    }

    void testScalingObject()
    {
        ObjectValue* val;
        ScalingObject t, t2;
        t.setValue("0");
        CPPUNIT_ASSERT(t.getValue() == "0");
        t2.setValue("100");
        CPPUNIT_ASSERT(t2.getValue() == "100");

        t.setValue("10");
        CPPUNIT_ASSERT(t.getValue() == "9.8");
        t2.setValue("99.9");
        CPPUNIT_ASSERT(t2.getValue() == "99.6");

        CPPUNIT_ASSERT_EQUAL((uint32_t)25, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((uint32_t)254, t2.getIntValue());

        CPPUNIT_ASSERT_THROW(t.setValue("-1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("256"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("101"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("30000"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);

        ScalingObjectValue tval("10");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("10");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        ScalingObjectValue tval2("99.6");
        CPPUNIT_ASSERT(!t.equals(&tval2));
//        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("99.6");
        CPPUNIT_ASSERT(!t.equals(val));
//        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(255);
        CPPUNIT_ASSERT(t.getValue() == "100");
        CPPUNIT_ASSERT_EQUAL((uint32_t)255, t.getIntValue());
    }

    void testScalingObjectWrite()
    {
        ScalingObject t;
        t.setValue("55");
        t.addChangeListener(this);

        uint8_t buf[4] = {0, 0x80, 128};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(t.getValue() == "50.2");
        ScalingObjectValue tval1("50.2");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 10;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "3.92");
        ScalingObjectValue tval2("3.92");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(t.getValue() == "3.92");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "0");
        ScalingObjectValue tval3("0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((uint32_t)0, t.getIntValue());
    }

    void testScalingObjectUpdate()
    {
        ScalingObject t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("68");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("69");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("69");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("70");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testScalingExportImport()
    {
        ScalingObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<ScalingObject*>(res));
        delete res;
    }

    void testScalingPersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_scale");
        pConfig.SetAttribute("type", "5.001");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("100");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "100");
        res->setValue("0");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "0");
        res2->setValue("35");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "34.5");
        delete res3;
    }

    void testHeatingModeObject()
    {
        ObjectValue* val;
        HeatingModeObject t, t2;
        t.setValue("comfort");
        CPPUNIT_ASSERT(t.getValue() == "comfort");
        t2.setValue("frost");
        CPPUNIT_ASSERT(t2.getValue() == "frost");

        CPPUNIT_ASSERT_EQUAL((uint32_t)1, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((uint32_t)4, t2.getIntValue());

        t.setValue("standby");
        CPPUNIT_ASSERT(t.getValue() == "standby");
        t2.setValue("night");
        CPPUNIT_ASSERT(t2.getValue() == "night");

        CPPUNIT_ASSERT_EQUAL((uint32_t)2, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((uint32_t)3, t2.getIntValue());

        CPPUNIT_ASSERT_THROW(t.setValue("-1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("256"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("4"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("75,6"), ticpp::Exception);

        HeatingModeObjectValue tval("standby");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("standby");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        HeatingModeObjectValue tval2("night");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("night");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(1);
        CPPUNIT_ASSERT(t.getValue() == "comfort");
        CPPUNIT_ASSERT_EQUAL((uint32_t)1, t.getIntValue());
    }

    void testHeatingModeObjectWrite()
    {
        HeatingModeObject t;
        t.setValue("frost");
        t.addChangeListener(this);

        uint8_t buf[4] = {0, 0x80, 1};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(t.getValue() == "comfort");
        HeatingModeObjectValue tval1("comfort");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 2;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "standby");
        HeatingModeObjectValue tval2("standby");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(t.getValue() == "standby");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 3;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "night");
        HeatingModeObjectValue tval3("night");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 4;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "frost");
        HeatingModeObjectValue tval4("frost");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval4));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((uint32_t)4, t.getIntValue());
    }

    void testHeatingModeObjectUpdate()
    {
        HeatingModeObject t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("comfort");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("standby");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("standby");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("night");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testHeatingModeExportImport()
    {
        HeatingModeObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<HeatingModeObject*>(res));
        delete res;
    }

    void testHeatingModePersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_heat");
        pConfig.SetAttribute("type", "heat-mode");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("comfort");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "comfort");
        res->setValue("standby");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "standby");
        res2->setValue("night");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "night");
        res3->setValue("frost");
        delete res3;

        Object *res4 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res4->getValue() == "frost");
        delete res4;
    }

    void testU16Object()
    {
        ObjectValue* val;
        U16Object t, t2;
        t.setValue("0");
        CPPUNIT_ASSERT(t.getValue() == "0");
        t2.setValue("65535");
        CPPUNIT_ASSERT(t2.getValue() == "65535");

        t.setValue("10");
        CPPUNIT_ASSERT(t.getValue() == "10");
        t2.setValue("65530");
        CPPUNIT_ASSERT(t2.getValue() == "65530");

        CPPUNIT_ASSERT_EQUAL((uint32_t)10, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((uint32_t)65530, t2.getIntValue());

        CPPUNIT_ASSERT_THROW(t.setValue("-1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("65536"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("70000"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("25.1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("75,6"), ticpp::Exception);

        U16ObjectValue tval("10");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("10");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        U16ObjectValue tval2("65530");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("65530");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(100);
        CPPUNIT_ASSERT(t.getValue() == "100");
        CPPUNIT_ASSERT_EQUAL((uint32_t)100, t.getIntValue());
    }

    void testU16ObjectWrite()
    {
        U16Object t;
        t.setValue("55");
        t.addChangeListener(this);

        uint8_t buf[4] = {0, 0x80, 39, 16};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 4, src);        
        CPPUNIT_ASSERT(t.getValue() == "10000");
        U16ObjectValue tval1("10000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 255;
        buf[3] = 254;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 4, src);       
        CPPUNIT_ASSERT(t.getValue() == "65534");
        U16ObjectValue tval2("65534");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 4, src);        
        CPPUNIT_ASSERT(t.getValue() == "65534");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0;
        buf[3] = 0;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 4, src);       
        CPPUNIT_ASSERT(t.getValue() == "0");
        U16ObjectValue tval3("0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((uint32_t)0, t.getIntValue());
    }

    void testU16ObjectUpdate()
    {
        U16Object t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("168");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("170");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testU16ExportImport()
    {
        U16Object orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<U16Object*>(res));
        delete res;
    }

    void testU16Persist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_scale");
        pConfig.SetAttribute("type", "7.xxx");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("65535");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "65535");
        res->setValue("0");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "0");
        res2->setValue("50000");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "50000");
        delete res3;
    }

    void testU32Object()
    {
        ObjectValue* val;
        U32Object t, t2;
        t.setValue("0");
        CPPUNIT_ASSERT(t.getValue() == "0");
        t2.setValue("4294967295");
        CPPUNIT_ASSERT(t2.getValue() == "4294967295");

        t.setValue("10");
        CPPUNIT_ASSERT(t.getValue() == "10");
        t2.setValue("4000000000");
        CPPUNIT_ASSERT(t2.getValue() == "4000000000");

        CPPUNIT_ASSERT_EQUAL((uint32_t)10, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((uint32_t)4000000000UL, t2.getIntValue());

//        CPPUNIT_ASSERT_THROW(t.setValue("-1"), ticpp::Exception); // Some versions of libstdc++ return 4294967295 when extracting "-1" from a stream into an unsigned int.
        CPPUNIT_ASSERT_THROW(t.setValue("4294967296"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("5000000000"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("25.1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("75,6"), ticpp::Exception);

        U32ObjectValue tval("10");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("10");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        U32ObjectValue tval2("4000000000");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("4000000000");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(3000000000UL);
        CPPUNIT_ASSERT(t.getValue() == "3000000000");
        CPPUNIT_ASSERT_EQUAL((uint32_t)3000000000UL, t.getIntValue());
    }

    void testU32ObjectWrite()
    {
        U32Object t;
        t.setValue("2000000000");
        t.addChangeListener(this);

        uint8_t buf[6] = {0, 0x80, 0xB2, 0xD0, 0x5E, 0x00};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 6, src);        
        CPPUNIT_ASSERT(t.getValue() == "3000000000");
        U32ObjectValue tval1("3000000000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 0;
        buf[3] = 0;
        buf[4] = 3;
        buf[5] = 232;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 6, src);       
        CPPUNIT_ASSERT(t.getValue() == "1000");
        U32ObjectValue tval2("1000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 6, src);        
        CPPUNIT_ASSERT(t.getValue() == "1000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0;
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 0;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 6, src);       
        CPPUNIT_ASSERT(t.getValue() == "0");
        U32ObjectValue tval3("0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((uint32_t)0, t.getIntValue());
    }

    void testU32ObjectUpdate()
    {
        U32Object t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("4000168");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("4000169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("4000169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("4000170");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testU32ExportImport()
    {
        U32Object orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<U32Object*>(res));
        delete res;
    }

    void testU32Persist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_scale");
        pConfig.SetAttribute("type", "12.xxx");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("4294967295");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "4294967295");
        res->setValue("0");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "0");
        res2->setValue("4000000000");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "4000000000");
        delete res3;
    }

    void testS8Object()
    {
        ObjectValue* val;
        S8Object t, t2;
        t.setValue("-128");
        CPPUNIT_ASSERT(t.getValue() == "-128");
        t2.setValue("127");
        CPPUNIT_ASSERT(t2.getValue() == "127");

        t.setValue("-10");
        CPPUNIT_ASSERT(t.getValue() == "-10");
        t2.setValue("120");
        CPPUNIT_ASSERT(t2.getValue() == "120");

        CPPUNIT_ASSERT_EQUAL((int32_t)-10, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((int32_t)120, t2.getIntValue());

        CPPUNIT_ASSERT_THROW(t.setValue("-130"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("128"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("30000"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("-25.1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("75,6"), ticpp::Exception);

        S8ObjectValue tval("-10");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("-10");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        S8ObjectValue tval2("120");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("120");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(0);
        CPPUNIT_ASSERT(t.getValue() == "0");
        CPPUNIT_ASSERT_EQUAL((int32_t)0, t.getIntValue());
    }

    void testS8ObjectWrite()
    {
        S8Object t;
        t.setValue("55");
        t.addChangeListener(this);

        uint8_t buf[4] = {0, 0x80, 128};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(t.getValue() == "-128");
        S8ObjectValue tval1("-128");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 127;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "127");
        S8ObjectValue tval2("127");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);        
        CPPUNIT_ASSERT(t.getValue() == "127");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 3, src);       
        CPPUNIT_ASSERT(t.getValue() == "0");
        S8ObjectValue tval3("0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((int32_t)0, t.getIntValue());
    }

    void testS8ObjectUpdate()
    {
        S8Object t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("118");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("-118");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("-118");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("119");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testS8ExportImport()
    {
        S8Object orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<S8Object*>(res));
        delete res;
    }

    void testS8Persist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_scale");
        pConfig.SetAttribute("type", "6.xxx");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("127");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "127");
        res->setValue("0");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "0");
        res2->setValue("-35");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "-35");
        delete res3;
    }

    void testS16Object()
    {
        ObjectValue* val;
        S16Object t, t2;
        t.setValue("0");
        CPPUNIT_ASSERT(t.getValue() == "0");
        t2.setValue("32767");
        CPPUNIT_ASSERT(t2.getValue() == "32767");

        t.setValue("10");
        CPPUNIT_ASSERT(t.getValue() == "10");
        t2.setValue("-32768");
        CPPUNIT_ASSERT(t2.getValue() == "-32768");

        CPPUNIT_ASSERT_EQUAL((int32_t)10, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((int32_t)-32768, t2.getIntValue());

        CPPUNIT_ASSERT_THROW(t.setValue("-32769"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("32768"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("70000"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("25.1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("75,6"), ticpp::Exception);

        S16ObjectValue tval("10");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("10");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        S16ObjectValue tval2("-32768");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("-32768");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(100);
        CPPUNIT_ASSERT(t.getValue() == "100");
        CPPUNIT_ASSERT_EQUAL((int32_t)100, t.getIntValue());
    }

    void testS16ObjectWrite()
    {
        S16Object t;
        t.setValue("55");
        t.addChangeListener(this);

        uint8_t buf[4] = {0, 0x80, 39, 16};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 4, src);        
        CPPUNIT_ASSERT(t.getValue() == "10000");
        S16ObjectValue tval1("10000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 255;
        buf[3] = 254;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 4, src);       
        CPPUNIT_ASSERT(t.getValue() == "-2");
        S16ObjectValue tval2("-2");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 4, src);        
        CPPUNIT_ASSERT(t.getValue() == "-2");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0;
        buf[3] = 0;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 4, src);       
        CPPUNIT_ASSERT(t.getValue() == "0");
        S16ObjectValue tval3("0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((int32_t)0, t.getIntValue());
    }

    void testS16ObjectUpdate()
    {
        S16Object t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("30169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("-30169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("-30169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("30170");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testS16ExportImport()
    {
        S16Object orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<S16Object*>(res));
        delete res;
    }

    void testS16Persist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_scale");
        pConfig.SetAttribute("type", "8.xxx");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("32767");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "32767");
        res->setValue("0");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "0");
        res2->setValue("-32700");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "-32700");
        delete res3;
    }

    void testS32Object()
    {
        ObjectValue* val;
        S32Object t, t2;
        t.setValue("0");
        CPPUNIT_ASSERT(t.getValue() == "0");
        t2.setValue("2147483647");
        CPPUNIT_ASSERT(t2.getValue() == "2147483647");

        t.setValue("10");
        CPPUNIT_ASSERT(t.getValue() == "10");
        t2.setValue("-2147483648");
        CPPUNIT_ASSERT(t2.getValue() == "-2147483648");

        CPPUNIT_ASSERT_EQUAL((int32_t)10, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((int32_t)-2147483648UL, t2.getIntValue());

        CPPUNIT_ASSERT_THROW(t.setValue("-2147483649"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("2147483648"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("5000000000"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("25.1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("75,6"), ticpp::Exception);

        S32ObjectValue tval("10");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("10");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        S32ObjectValue tval2("-2147483648");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("-2147483648");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(-2000000000UL);
        CPPUNIT_ASSERT(t.getValue() == "-2000000000");
        CPPUNIT_ASSERT_EQUAL((int32_t)-2000000000UL, t.getIntValue());
    }

    void testS32ObjectWrite()
    {
        S32Object t;
        t.setValue("-2000000000");
        t.addChangeListener(this);

        uint8_t buf[6] = {0, 0x80, 0x77, 0x35, 0x94, 0x00};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 6, src);        
        CPPUNIT_ASSERT(t.getValue() == "2000000000");
        S32ObjectValue tval1("2000000000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 0x88;
        buf[3] = 0xCA;
        buf[4] = 0x6C;
        buf[5] = 0x00;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 6, src);       
        CPPUNIT_ASSERT(t.getValue() == "-2000000000");
        S32ObjectValue tval2("-2000000000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 6, src);        
        CPPUNIT_ASSERT(t.getValue() == "-2000000000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0;
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 0;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 6, src);       
        CPPUNIT_ASSERT(t.getValue() == "0");
        S32ObjectValue tval3("0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((int32_t)0, t.getIntValue());
    }

    void testS32ObjectUpdate()
    {
        S32Object t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("-2000000169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("2000000169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("2000000169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("2000000170");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testS32ExportImport()
    {
        S32Object orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<S32Object*>(res));
        delete res;
    }

    void testS32Persist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_scale");
        pConfig.SetAttribute("type", "13.xxx");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("2000000000");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "2000000000");
        res->setValue("0");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "0");
        res2->setValue("-2147483648");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "-2147483648");
        delete res3;
    }

    void testS64Object()
    {
        ObjectValue* val;
        S64Object t, t2;
        t.setValue("0");
        CPPUNIT_ASSERT(t.getValue() == "0");
        t2.setValue("2147483649");
        CPPUNIT_ASSERT(t2.getValue() == "2147483649");

        t.setValue("10");
        CPPUNIT_ASSERT(t.getValue() == "10");
        t2.setValue("-4147483648");
        CPPUNIT_ASSERT(t2.getValue() == "-4147483648");

        CPPUNIT_ASSERT_EQUAL((int64_t)10, t.getIntValue());
        CPPUNIT_ASSERT_EQUAL((int64_t)-4147483648ULL, t2.getIntValue());

        CPPUNIT_ASSERT_THROW(t.setValue("−9223372036854775809"), ticpp::Exception); // −9223372036854775808 to 9223372036854775807
        CPPUNIT_ASSERT_THROW(t.setValue("9223372036854775808"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("10000000000000000000"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("akmgfbf"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("25.1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(t.setValue("75,6"), ticpp::Exception);

        S64ObjectValue tval("10");
        CPPUNIT_ASSERT(t.equals(&tval));
        CPPUNIT_ASSERT(!t2.equals(&tval));

        val = t.createObjectValue("10");
        CPPUNIT_ASSERT(t.equals(val));
        CPPUNIT_ASSERT(!t2.equals(val));
        delete val;      

        S64ObjectValue tval2("-4147483648");
        CPPUNIT_ASSERT(!t.equals(&tval2));
        CPPUNIT_ASSERT(t2.equals(&tval2));

        val = t.createObjectValue("-4147483648");
        CPPUNIT_ASSERT(!t.equals(val));
        CPPUNIT_ASSERT(t2.equals(val));
        delete val;      

        t.setIntValue(-2000000000LL);
        CPPUNIT_ASSERT(t.getValue() == "-2000000000");
        CPPUNIT_ASSERT_EQUAL((int64_t)-2000000000LL, t.getIntValue());
    }

    void testS64ObjectWrite()
    {
        S64Object t;
        t.setValue("-2000000000");
        t.addChangeListener(this);

        uint8_t buf[10] = {0, 0x80, 0, 0, 0, 0, 0x77, 0x35, 0x94, 0x00};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 10, src);        
        CPPUNIT_ASSERT(t.getValue() == "2000000000");
        S64ObjectValue tval1("2000000000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[2] = 0xff;
        buf[3] = 0xff;
        buf[4] = 0xff;
        buf[5] = 0xff;
        buf[6] = 0x88;
        buf[7] = 0xCA;
        buf[8] = 0x6C;
        buf[9] = 0x00;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 10, src);       
        CPPUNIT_ASSERT(t.getValue() == "-2000000000");
        S64ObjectValue tval2("-2000000000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.onWrite(buf, 10, src);        
        CPPUNIT_ASSERT(t.getValue() == "-2000000000");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[2] = 0;
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;
        buf[8] = 0;
        buf[9] = 0;
        isOnChangeCalled_m = false;
        t.onWrite(buf, 10, src);       
        CPPUNIT_ASSERT(t.getValue() == "0");
        S64ObjectValue tval3("0");
        CPPUNIT_ASSERT_EQUAL(0, t.compare(&tval3));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        CPPUNIT_ASSERT_EQUAL((int64_t)0, t.getIntValue());
    }

    void testS64ObjectUpdate()
    {
        S64Object t;
        t.addChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("-2000000169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("2000000169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        t.setValue("2000000169");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        t.removeChangeListener(this);

        isOnChangeCalled_m = false;
        t.setValue("2000000170");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testS64ExportImport()
    {
        S64Object orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<S64Object*>(res));
        delete res;
    }

    void testS64Persist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_scale");
        pConfig.SetAttribute("type", "29.xxx");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("5000000000");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "5000000000");
        res->setValue("0");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "0");
        res2->setValue("-6147483648");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "-6147483648");
        delete res3;
    }

    void testString14Object()
    {
        ObjectValue* val;
        String14Object s, s2, s3;
        s.setValue("test");
        CPPUNIT_ASSERT(s.getValue() == "test");
        s2.setValue("Hi there!");
        CPPUNIT_ASSERT(s2.getValue() == "Hi there!");

        s.setValue("Some text !?=+");
        CPPUNIT_ASSERT(s.getValue() == "Some text !?=+");
        s2.setValue("AnotherMessage");
        CPPUNIT_ASSERT(s2.getValue() == "AnotherMessage");

        CPPUNIT_ASSERT(s.getValue() == "Some text !?=+");
        CPPUNIT_ASSERT(s2.getValue() == "AnotherMessage");

        s3.setValue("é µ ë \x82");
        CPPUNIT_ASSERT_THROW(s.setValue("more than 14 characters"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(s.setValue("just a bit more"), ticpp::Exception);

        String14ObjectValue sval("Some text !?=+");
        CPPUNIT_ASSERT(s.equals(&sval));
        CPPUNIT_ASSERT(!s2.equals(&sval));

        val = s.createObjectValue("Some text !?=+");
        CPPUNIT_ASSERT(s.equals(val));
        CPPUNIT_ASSERT(!s2.equals(val));
        delete val;      

        String14ObjectValue sval2("AnotherMessage");
        CPPUNIT_ASSERT(!s.equals(&sval2));
        CPPUNIT_ASSERT(s2.equals(&sval2));

        val = s.createObjectValue("AnotherMessage");
        CPPUNIT_ASSERT(!s.equals(val));
        CPPUNIT_ASSERT(s2.equals(val));
        delete val;      

        s.setStringValue("A test \n value");
        CPPUNIT_ASSERT(s.getValue() == "A test \n value");
    }

    void testString14ObjectWrite()
    {
        String14Object s;
        s.setValue("something");
        s.addChangeListener(this);

        uint8_t buf[16] = {0, 0x80, 0x45, 0x49, 0x42, 0x20, 0x69, 0x73, 0x20, 0x4F, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 16, src);        
        CPPUNIT_ASSERT(s.getValue() == "EIB is OK");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[6] = 0x49;
        buf[7] = 0x53;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 16, src);       
        CPPUNIT_ASSERT(s.getValue() == "EIB IS OK");
        String14ObjectValue sval1("EIB IS OK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.onWrite(buf, 16, src);        
        CPPUNIT_ASSERT(s.getValue() == "EIB IS OK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[8] = 0x0a;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 16, src);       
        CPPUNIT_ASSERT(s.getValue() == "EIB IS\nOK");
        String14ObjectValue sval2("EIB IS\nOK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);
    }

    void testString14ObjectUpdate()
    {
        String14Object s;
        s.addChangeListener(this);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK?");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK!");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK!");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        s.removeChangeListener(this);

        isOnChangeCalled_m = false;
        s.setValue("something else");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testString14ExportImport()
    {
        String14Object orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<String14Object*>(res));
        delete res;
    }

    void testString14Persist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_string");
        pConfig.SetAttribute("type", "16.001");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("EIB is OK");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "EIB is OK");
        res->setValue("Test \r\n ?é=+");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "Test \r\n ?é=+");
        res2->setValue("14  characters");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "14  characters");
        res3->setValue("");
        delete res3;

        Object *res4 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res4->getValue() == "");
        delete res4;
    }

    void testString14AsciiObject()
    {
        ObjectValue* val;
        String14AsciiObject s, s2;
        s.setValue("test");
        CPPUNIT_ASSERT(s.getValue() == "test");
        s2.setValue("Hi there!");
        CPPUNIT_ASSERT(s2.getValue() == "Hi there!");

        s.setValue("Some text !?=+");
        CPPUNIT_ASSERT(s.getValue() == "Some text !?=+");
        s2.setValue("AnotherMessage");
        CPPUNIT_ASSERT(s2.getValue() == "AnotherMessage");

        CPPUNIT_ASSERT(s.getValue() == "Some text !?=+");
        CPPUNIT_ASSERT(s2.getValue() == "AnotherMessage");

        CPPUNIT_ASSERT_THROW(s.setValue("illegal 'é'"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(s.setValue("256µs"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(s.setValue("noël"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(s.setValue("more than 14 characters"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(s.setValue("just a bit more"), ticpp::Exception);

        String14AsciiObjectValue sval("Some text !?=+");
        CPPUNIT_ASSERT(s.equals(&sval));
        CPPUNIT_ASSERT(!s2.equals(&sval));

        val = s.createObjectValue("Some text !?=+");
        CPPUNIT_ASSERT(s.equals(val));
        CPPUNIT_ASSERT(!s2.equals(val));
        delete val;

        String14AsciiObjectValue sval2("AnotherMessage");
        CPPUNIT_ASSERT(!s.equals(&sval2));
        CPPUNIT_ASSERT(s2.equals(&sval2));

        val = s.createObjectValue("AnotherMessage");
        CPPUNIT_ASSERT(!s.equals(val));
        CPPUNIT_ASSERT(s2.equals(val));
        delete val;

        s.setStringValue("A test \n value");
        CPPUNIT_ASSERT(s.getValue() == "A test \n value");
    }

    void testString14AsciiObjectWrite()
    {
        String14AsciiObject s;
        s.setValue("something");
        s.addChangeListener(this);

        uint8_t buf[16] = {0, 0x80, 0x45, 0x49, 0x42, 0x20, 0x69, 0x73, 0x20, 0x4F, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 16, src);
        CPPUNIT_ASSERT(s.getValue() == "EIB is OK");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[6] = 0x49;
        buf[7] = 0x53;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 16, src);
        CPPUNIT_ASSERT(s.getValue() == "EIB IS OK");
        String14AsciiObjectValue sval1("EIB IS OK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.onWrite(buf, 16, src);
        CPPUNIT_ASSERT(s.getValue() == "EIB IS OK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[8] = 0x0a;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 16, src);
        CPPUNIT_ASSERT(s.getValue() == "EIB IS\nOK");
        String14AsciiObjectValue sval2("EIB IS\nOK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);
    }

    void testString14AsciiObjectUpdate()
    {
        String14AsciiObject s;
        s.addChangeListener(this);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK?");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK!");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK!");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        s.removeChangeListener(this);

        isOnChangeCalled_m = false;
        s.setValue("something else");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testString14AsciiExportImport()
    {
        String14AsciiObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<String14AsciiObject*>(res));
        delete res;
    }

    void testString14AsciiPersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);

        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_string");
        pConfig.SetAttribute("type", "EIS15");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("EIB is OK");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "EIB is OK");
        res->setValue("Test \r\n ?!=+");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "Test \r\n ?!=+");
        res2->setValue("14  characters");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "14  characters");
        res3->setValue("");
        delete res3;

        Object *res4 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res4->getValue() == "");
        delete res4;
    }

    void testStringObject()
    {
        ObjectValue* val;
        StringObject s, s2;
        s.setValue("test");
        CPPUNIT_ASSERT(s.getValue() == "test");
        s2.setValue("Hi there! This is more than 14 characters");
        CPPUNIT_ASSERT(s2.getValue() == "Hi there! This is more than 14 characters");

        s.setValue("Some text !?=+");
        CPPUNIT_ASSERT(s.getValue() == "Some text !?=+");
        s2.setValue("AnotherMessage with special chars 'éµë' ");
        CPPUNIT_ASSERT(s2.getValue() == "AnotherMessage with special chars 'éµë' ");

        CPPUNIT_ASSERT(s.getValue() == "Some text !?=+");
        CPPUNIT_ASSERT(s2.getValue() == "AnotherMessage with special chars 'éµë' ");

        String14ObjectValue sval("Some text !?=+");
        CPPUNIT_ASSERT(s.equals(&sval));
        CPPUNIT_ASSERT(!s2.equals(&sval));

        val = s.createObjectValue("Some text !?=+");
        CPPUNIT_ASSERT(s.equals(val));
        CPPUNIT_ASSERT(!s2.equals(val));
        delete val;      

        StringObjectValue sval2("AnotherMessage with special chars 'éµë' ");
        CPPUNIT_ASSERT(!s.equals(&sval2));
        CPPUNIT_ASSERT(s2.equals(&sval2));

        val = s.createObjectValue("AnotherMessage with special chars 'éµë' ");
        CPPUNIT_ASSERT(!s.equals(val));
        CPPUNIT_ASSERT(s2.equals(val));
        delete val;      

        s.setStringValue("A test \n value");
        CPPUNIT_ASSERT(s.getValue() == "A test \n value");
    }

    void testStringObjectWrite()
    {
        StringObject s;
        s.setValue("something bigger");
        s.addChangeListener(this);

        uint8_t buf[32] = {0, 0x80, 0x45, 0x49, 0x42, 0x20, 0x69, 0x73, 0x20, 0x4F, 0x4B, 0x20, 0x45, 0x49, 0x42, 0x20, 0x69, 0x73, 0x20, 0x4F, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 32, src);        
        CPPUNIT_ASSERT(s.getValue() == "EIB is OK EIB is OK");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        buf[6] = 0x49;
        buf[7] = 0x53;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 32, src);       
        CPPUNIT_ASSERT(s.getValue() == "EIB IS OK EIB is OK");
        StringObjectValue sval1("EIB IS OK EIB is OK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.onWrite(buf, 32, src);        
        CPPUNIT_ASSERT(s.getValue() == "EIB IS OK EIB is OK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        buf[8] = 0x0a;
        isOnChangeCalled_m = false;
        s.onWrite(buf, 32, src);       
        CPPUNIT_ASSERT(s.getValue() == "EIB IS\nOK EIB is OK");
        StringObjectValue sval2("EIB IS\nOK EIB is OK");
        CPPUNIT_ASSERT_EQUAL(0, s.compare(&sval2));
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);
    }

    void testStringObjectUpdate()
    {
        StringObject s;
        s.addChangeListener(this);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK?");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK!");
        CPPUNIT_ASSERT(isOnChangeCalled_m == true);

        isOnChangeCalled_m = false;
        s.setValue("EIB is OK!");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);

        s.removeChangeListener(this);

        isOnChangeCalled_m = false;
        s.setValue("something else");
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
    }

    void testStringExportImport()
    {
        StringObject orig;
        Object *res;
        ticpp::Element pConfig;

        orig.setID("test");
        orig.exportXml(&pConfig);
        res = Object::create(&pConfig);
        CPPUNIT_ASSERT(strcmp(res->getID(), orig.getID()) == 0);
        CPPUNIT_ASSERT(dynamic_cast<StringObject*>(res));
        delete res;
    }

    void testStringPersist()
    {
        system ("rm -rf /tmp/linknx_unittest");
        system ("mkdir /tmp/linknx_unittest");
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);
        
        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_string");
        pConfig.SetAttribute("type", "28.001");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("EIB is OK EIB is OK");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "EIB is OK EIB is OK");
        res->setValue("Test \r\n ?!=+");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "Test \r\n ?!=+");
        res2->setValue("16    characters");
        delete res2;

        Object *res3 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res3->getValue() == "16    characters");
        res3->setValue("");
        delete res3;

        Object *res4 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res4->getValue() == "");
        delete res4;
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION( ObjectTest );
