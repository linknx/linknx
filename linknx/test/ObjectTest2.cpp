#include <cppunit/extensions/HelperMacros.h>
#include <cstdlib>
#include "objectcontroller.h"
#include "services.h"

class ObjectTest2 : public CppUnit::TestFixture, public ChangeListener
{
    CPPUNIT_TEST_SUITE( ObjectTest2 );
    CPPUNIT_TEST( testSwitchingEnableObject );
    CPPUNIT_TEST( testSwitchingEnablePersist );
    CPPUNIT_TEST( testSwitchCtrlEnableObject );
    CPPUNIT_TEST( testSwitchCtrlEnablePersist );
    CPPUNIT_TEST( testValueObjectWriteNoPrecision );
//    CPPUNIT_TEST(  );
    
    CPPUNIT_TEST_SUITE_END();

private:
    bool isOnChangeCalled_m;
    
    void cleanTestDir()
    {
        CPPUNIT_ASSERT(system("rm -rf /tmp/linknx_unittest") != -1);
        CPPUNIT_ASSERT(system("mkdir /tmp/linknx_unittest") != -1);
    }
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

    void testSwitchingEnableObject()
    {
        const std::string on = "enable";
        const std::string off = "disable";
        const std::string zero = "0";
        const std::string one = "1";
        ObjectValue* val;
        SwitchingObjectImpl<SwitchingImplObjectValue<3> > sw, sw2;
        sw.setValue("enable");
        CPPUNIT_ASSERT(sw.getValue() == "enable");
        sw.setValue("1");
        CPPUNIT_ASSERT(sw.getValue() == "enable");

        sw2.setValue("disable");
        CPPUNIT_ASSERT(sw2.getValue() == "disable");
        sw2.setValue("0");
        CPPUNIT_ASSERT(sw2.getValue() == "disable");

        CPPUNIT_ASSERT(sw.getBoolValue() == true);
        CPPUNIT_ASSERT(sw2.getBoolValue() == false);

        SwitchingImplObjectValue<3> swval(on);
        CPPUNIT_ASSERT(sw.equals(&swval));
        CPPUNIT_ASSERT(!sw2.equals(&swval));

        val = sw.createObjectValue(one);
        CPPUNIT_ASSERT(sw.equals(val));
        CPPUNIT_ASSERT(!sw2.equals(val));
        delete val;

        SwitchingImplObjectValue<3> swval2(zero);
        CPPUNIT_ASSERT(!sw.equals(&swval2));
        CPPUNIT_ASSERT(sw2.equals(&swval2));

        val = sw.createObjectValue(off);
        CPPUNIT_ASSERT(!sw.equals(val));
        CPPUNIT_ASSERT(sw2.equals(val));
        delete val;

        sw.setBoolValue(false);
        CPPUNIT_ASSERT(sw.getValue() == "disable");
        sw2.setBoolValue(true);
        CPPUNIT_ASSERT(sw2.getValue() == "enable");
    }
    
    void testSwitchingEnablePersist()
    {
        cleanTestDir();
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);

        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_sw_en");
        pConfig.SetAttribute("type", "1.003");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("enable");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "enable");
        res->setValue("disable");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "disable");
        delete res2;
    }

    void testSwitchCtrlEnableObject()
    {
        const std::string on = "enable";
        const std::string off = "disable";
        const std::string nc = "no control";
        const std::string zero = "0";
        const std::string one = "1";
        const std::string neg = "-1";
        ObjectValue* val;
        SwitchingControlObject<SwitchingControlImplObjectValue<3> > sw, sw2, sw3;
        sw.setValue("enable");
        CPPUNIT_ASSERT(sw.getValue() == "enable");
        sw.setValue("1");
        CPPUNIT_ASSERT(sw.getValue() == "enable");

        sw2.setValue("disable");
        CPPUNIT_ASSERT(sw2.getValue() == "disable");
        sw2.setValue("0");
        CPPUNIT_ASSERT(sw2.getValue() == "disable");

        sw3.setValue("no control");
        CPPUNIT_ASSERT(sw3.getValue() == "no control");
        sw3.setValue("-1");
        CPPUNIT_ASSERT(sw3.getValue() == "no control");

        CPPUNIT_ASSERT(sw.getBoolValue() == true);
        CPPUNIT_ASSERT(sw.getControlValue() == true);
        CPPUNIT_ASSERT(sw2.getBoolValue() == false);
        CPPUNIT_ASSERT(sw2.getControlValue() == true);
        CPPUNIT_ASSERT(sw3.getBoolValue() == false);
        CPPUNIT_ASSERT(sw3.getControlValue() == false);

        SwitchingControlImplObjectValue<3> swval(on);
        CPPUNIT_ASSERT(sw.equals(&swval));
        CPPUNIT_ASSERT(!sw2.equals(&swval));
        CPPUNIT_ASSERT(!sw3.equals(&swval));

        val = sw.createObjectValue(one);
        CPPUNIT_ASSERT(sw.equals(val));
        CPPUNIT_ASSERT(!sw2.equals(val));
        CPPUNIT_ASSERT(!sw3.equals(val));
        delete val;      

        SwitchingControlImplObjectValue<3> swval2(zero);
        CPPUNIT_ASSERT(!sw.equals(&swval2));
        CPPUNIT_ASSERT(sw2.equals(&swval2));
        CPPUNIT_ASSERT(!sw3.equals(&swval2));

        val = sw.createObjectValue(off);
        CPPUNIT_ASSERT(!sw.equals(val));
        CPPUNIT_ASSERT(sw2.equals(val));
        CPPUNIT_ASSERT(!sw3.equals(val));
        delete val;      

        SwitchingControlImplObjectValue<3> swval3(neg);
        CPPUNIT_ASSERT(!sw.equals(&swval3));
        CPPUNIT_ASSERT(!sw2.equals(&swval3));
        CPPUNIT_ASSERT(sw3.equals(&swval3));

        val = sw.createObjectValue(nc);
        CPPUNIT_ASSERT(!sw.equals(val));
        CPPUNIT_ASSERT(!sw2.equals(val));
        CPPUNIT_ASSERT(sw3.equals(val));
        delete val;      

        sw.setBoolValue(false);
        CPPUNIT_ASSERT(sw.getValue() == "disable");
        sw2.setBoolValue(true);
        CPPUNIT_ASSERT(sw2.getValue() == "enable");
    }
    
    void testSwitchCtrlEnablePersist()
    {
        cleanTestDir();
        ticpp::Element pSvcConfig("services");
        ticpp::Element pPersistenceConfig("persistence");
        pPersistenceConfig.SetAttribute("type", "file");
        pPersistenceConfig.SetAttribute("path", "/tmp/linknx_unittest");
        pSvcConfig.LinkEndChild(&pPersistenceConfig);
        Services::instance()->importXml(&pSvcConfig);

        ticpp::Element pConfig;
        pConfig.SetAttribute("id", "test_sw_en");
        pConfig.SetAttribute("type", "2.003");
        pConfig.SetAttribute("init", "persist");

        Object *orig = Object::create(&pConfig);
        orig->setValue("enable");
        delete orig;

        Object *res = Object::create(&pConfig);
        CPPUNIT_ASSERT(res->getValue() == "enable");
        res->setValue("disable");
        delete res;

        Object *res2 = Object::create(&pConfig);
        CPPUNIT_ASSERT(res2->getValue() == "disable");
        delete res2;
    }

    void testValueObjectWriteNoPrecision()
    {
        ValueObject0 v;
        v.setValue("244.9");
        CPPUNIT_ASSERT(v.getValue() == "244.8");
        v.addChangeListener(this);

        uint8_t buf[6] = {0, 0x80, (4<<3) | ((1530 & 0x700)>>8) , (1530 & 0xff)};
        eibaddr_t src;
        isOnChangeCalled_m = false;
        v.onWrite(buf, 4, src);        
        CPPUNIT_ASSERT(v.getValue() == "244.8");
        ValueObjectValue0 fval1("244.8");
        CPPUNIT_ASSERT_EQUAL(0, v.compare(&fval1));
        CPPUNIT_ASSERT(isOnChangeCalled_m == false);
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION( ObjectTest2 );
