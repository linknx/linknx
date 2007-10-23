#include <cppunit/extensions/HelperMacros.h>
#include "timermanager.h"

class TestablePeriodicTask : public PeriodicTask
{
public:
    TestablePeriodicTask(ChangeListener* cl) : PeriodicTask(cl) {};
    time_t callFindNext(time_t start, TimeSpec* next) { return findNext(start, next); };
};

class PeriodicTaskTest : public CppUnit::TestFixture, public ChangeListener
{
    CPPUNIT_TEST_SUITE( PeriodicTaskTest );
    CPPUNIT_TEST( testFindNextHour );
//    CPPUNIT_TEST(  );
    CPPUNIT_TEST_SUITE_END();

private:
    TestablePeriodicTask *task_m;
    bool isOnChangeCalled_m;
    time_t timeref1_m, timeref2_m,timeref3_m;
public:
    void setUp()
    {
        task_m = new TestablePeriodicTask(this);
        isOnChangeCalled_m = false;
        struct tm timeinfo;
        timeinfo.tm_hour = 19;
        timeinfo.tm_min = 40;
        timeinfo.tm_sec = 0;
        timeinfo.tm_mday = 1;
        timeinfo.tm_mon = 0;
        timeinfo.tm_year = 107;
        timeref1_m = mktime(&timeinfo);
        timeinfo.tm_sec = 40;
        timeref2_m = mktime(&timeinfo);
        timeinfo.tm_min = 50;
        timeref3_m = mktime(&timeinfo);
    }

    void tearDown()
    {
        delete task_m;
    }

    void onChange(Object* obj)
    {
        isOnChangeCalled_m = true;
    }

    void testFindNextHour()
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

CPPUNIT_TEST_SUITE_REGISTRATION( PeriodicTaskTest );
