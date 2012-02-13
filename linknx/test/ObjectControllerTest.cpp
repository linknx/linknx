#include <cppunit/extensions/HelperMacros.h>
#include "objectcontroller.h"

class ObjectControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( ObjectControllerTest );
    CPPUNIT_TEST( testAddGet );
    CPPUNIT_TEST( testAddSameID );
    CPPUNIT_TEST( testGetNotFound );
    CPPUNIT_TEST( testAddRemove );
    CPPUNIT_TEST( testWrite );
    CPPUNIT_TEST( testExportImport );
    CPPUNIT_TEST( testWriteMultipleGad );
//    CPPUNIT_TEST(  );
//    CPPUNIT_TEST(  );
    
    CPPUNIT_TEST_SUITE_END();

private:
    ObjectController* oc_m;
public:
    void setUp()
    {
        oc_m = ObjectController::instance(); 
    }

    void tearDown()
    {
        ObjectController::reset();
    }

    void testAddGet()
    {
        Object* obj1 = new SwitchingObject();
        obj1->setID("test_sw1");
        oc_m->addObject(obj1);

        Object* obj2 = new DimmingObject();
        obj2->setID("test_dim2");
        oc_m->addObject(obj2);
        
        CPPUNIT_ASSERT(oc_m->getObject("test_sw1") == obj1);
        CPPUNIT_ASSERT(oc_m->getObject("test_dim2") == obj2);

        CPPUNIT_ASSERT(dynamic_cast<SwitchingObject*>(oc_m->getObject("test_sw1")));
        CPPUNIT_ASSERT(dynamic_cast<DimmingObject*>(oc_m->getObject("test_dim2")));
    }

    void testAddSameID()
    {
        Object* obj1 = new SwitchingObject();
        obj1->setID("test_sw1");
        oc_m->addObject(obj1);

        Object* obj2 = new DimmingObject();
        obj2->setID("test_sw1");
        CPPUNIT_ASSERT_THROW(oc_m->addObject(obj2), ticpp::Exception);
        delete obj2;
    }

    void testGetNotFound()
    {
        Object* obj1 = new SwitchingObject();
        obj1->setID("test_sw1");
        oc_m->addObject(obj1);

        Object* obj2 = new DimmingObject();
        obj2->setID("test_dim2");
        oc_m->addObject(obj2);

        CPPUNIT_ASSERT_THROW(oc_m->getObject("test_sw2"), ticpp::Exception);
    }

    void testAddRemove()
    {
        Object* obj1 = new SwitchingObject();
        obj1->setID("test_sw1");
        oc_m->addObject(obj1);

        Object* obj2 = new DimmingObject();
        obj2->setID("test_dim2");
        oc_m->addObject(obj2);
        
        oc_m->removeObject(obj2);
        CPPUNIT_ASSERT(oc_m->getObject("test_sw1") == obj1);
        CPPUNIT_ASSERT_THROW(oc_m->getObject("test_dim2"), ticpp::Exception);
        obj1->decRefCount();

        oc_m->removeObject(obj1);
        CPPUNIT_ASSERT_THROW(oc_m->getObject("test_sw1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(oc_m->getObject("test_dim2"), ticpp::Exception);
    }

    void testWrite()
    {
        ticpp::Element pConfig;
        eibaddr_t src, dest;
        uint8_t buf[2] = {0, 0x81};

        pConfig.SetAttribute("id", "test_sw");
        pConfig.SetAttribute("gad", "1/1/50");
        Object *obj1 = Object::create(&pConfig);
        obj1->setValue("off");
        oc_m->addObject(obj1);

        pConfig.SetAttribute("id", "test_dim2");
        pConfig.SetAttribute("gad", "1/1/51");
        pConfig.SetAttribute("type", "EIS2");
        Object *obj2 = Object::create(&pConfig);
        obj2->setValue("down");
        oc_m->addObject(obj2);

        src = Object::ReadAddr("0.2.10");
        dest = Object::ReadGroupAddr("1/1/50");
        buf[1] = 0x81;
        oc_m->onWrite(src, dest, buf, 2);

        CPPUNIT_ASSERT(obj1->getValue() == "on");
        CPPUNIT_ASSERT(obj2->getValue() == "down");

        src = Object::ReadAddr("0.2.10");
        dest = Object::ReadGroupAddr("1/1/51");
        buf[1] = 0x8b;
        oc_m->onWrite(src, dest, buf, 2);

        CPPUNIT_ASSERT(obj1->getValue() == "on");
        CPPUNIT_ASSERT(obj2->getValue() == "up:3");
    }

    void testExportImport()
    {
        ticpp::Element pConfig;
        eibaddr_t src, dest;
        uint8_t buf[2] = {0, 0x81};

        pConfig.SetAttribute("id", "test_sw1");
        pConfig.SetAttribute("gad", "1/1/50");
        Object *obj1 = Object::create(&pConfig);
        obj1->setValue("off");
        oc_m->addObject(obj1);

        pConfig.SetAttribute("id", "test_dim2");
        pConfig.SetAttribute("gad", "1/1/51");
        pConfig.SetAttribute("type", "EIS2");
        Object *obj2 = Object::create(&pConfig);
        obj2->setValue("down");
        oc_m->addObject(obj2);

        ticpp::Element pObjects;
        oc_m->exportXml(&pObjects);
        
        ObjectController::reset();
        oc_m = ObjectController::instance(); 

        CPPUNIT_ASSERT_THROW(oc_m->getObject("test_sw1"), ticpp::Exception);
        CPPUNIT_ASSERT_THROW(oc_m->getObject("test_dim2"), ticpp::Exception);

        oc_m->importXml(&pObjects);

        CPPUNIT_ASSERT(dynamic_cast<SwitchingObject*>(oc_m->getObject("test_sw1")));
        CPPUNIT_ASSERT(dynamic_cast<DimmingObject*>(oc_m->getObject("test_dim2")));

        Object* sw1 = oc_m->getObject("test_sw1");
        Object* dim2 = oc_m->getObject("test_dim2");
        CPPUNIT_ASSERT(strcmp(sw1->getID(), "test_sw1") == 0);
        CPPUNIT_ASSERT(strcmp(dim2->getID(), "test_dim2") == 0);
        CPPUNIT_ASSERT_EQUAL(Object::ReadGroupAddr("1/1/50"), sw1->getGad());
        CPPUNIT_ASSERT_EQUAL(Object::ReadGroupAddr("1/1/51"), dim2->getGad());

    }

    void testWriteMultipleGad()
    {
        ticpp::Element pConfig;
        ticpp::Element pGadConfig;
        eibaddr_t src, dest;
        uint8_t buf[2] = {0, 0x81};

        pConfig.SetAttribute("id", "test_sw_feedback1");
        pConfig.SetAttribute("gad", "1/1/206");
        pConfig.SetText("The feedback object description");
        Object *obj2 = Object::create(&pConfig);
        obj2->setValue("off");
        oc_m->addObject(obj2);

        pConfig.SetAttribute("id", "test_sw_feedback2");
        pConfig.SetAttribute("gad", "1/1/206");
        pConfig.SetText("The feedback2 object description");
        Object *obj3 = Object::create(&pConfig);
        obj3->setValue("off");
        oc_m->addObject(obj3);

        src = Object::ReadAddr("0.2.16");
        dest = Object::ReadGroupAddr("1/1/206");
        buf[1] = 0x81;
        oc_m->onWrite(src, dest, buf, 2);

        CPPUNIT_ASSERT(obj2->getValue() == "on");
        CPPUNIT_ASSERT(obj3->getValue() == "on");


        pConfig.SetAttribute("id", "test_sw");
        pConfig.SetAttribute("gad", "1/1/6");
        pGadConfig.SetValue("listener");
        pGadConfig.SetAttribute("gad", "1/1/206");
        pConfig.LinkEndChild(&pGadConfig);
        pConfig.SetText("The object description");
        Object *obj1 = Object::create(&pConfig);
        obj1->setValue("off");
        oc_m->addObject(obj1);

        src = Object::ReadAddr("0.2.10");
        dest = Object::ReadGroupAddr("1/1/6");
        buf[1] = 0x81;
        oc_m->onWrite(src, dest, buf, 2);

        CPPUNIT_ASSERT(obj1->getValue() == "on");

        src = Object::ReadAddr("0.2.11");
        dest = Object::ReadGroupAddr("1/1/206");
        buf[1] = 0x80;
        oc_m->onWrite(src, dest, buf, 2);

        CPPUNIT_ASSERT(obj1->getValue() == "off");
        CPPUNIT_ASSERT(obj2->getValue() == "off");
        CPPUNIT_ASSERT(obj3->getValue() == "off");
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION( ObjectControllerTest );
