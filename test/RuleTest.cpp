#include <cppunit/extensions/HelperMacros.h>
#include "timermanager.h"
#include "services.h"
#include <iostream>

class TestableRule : public Rule
{
public:
    TestableRule() : Rule() {};
};

class RuleTest : public CppUnit::TestFixture/*, public ChangeListener*/
{
    CPPUNIT_TEST_SUITE( RuleTest );
    CPPUNIT_TEST( testIfTrueActionlist );
    
    CPPUNIT_TEST_SUITE_END();

private:
    TestableRule *rule_m;

public:
    void setUp()
    {
        rule_m = new TestableRule();
    }

    void tearDown()
    {
        delete rule_m;
    }

    /*void onChange(Object* obj)
    {
        isOnChangeCalled_m = true;
    }*/

    void testIfTrueActionList()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16);

        next = task_m->callFindNext(timeref1_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(2, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( RuleTest );
