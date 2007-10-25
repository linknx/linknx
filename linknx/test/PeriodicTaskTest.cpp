#include <cppunit/extensions/HelperMacros.h>
#include "timermanager.h"
#include "services.h"

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
    CPPUNIT_TEST( testFindNextHourAndWeekday );
    CPPUNIT_TEST( testFindNextHourAndWeekday2 );
    CPPUNIT_TEST( testFindNextHourAndMonthDay );
    CPPUNIT_TEST( testFindNextHourAndMonthLastDay );
    CPPUNIT_TEST( testFindNextHourAndMonth );
    CPPUNIT_TEST( testFindNextOnLastSchedule );
    CPPUNIT_TEST( testFindNextHourNoMinute );
    CPPUNIT_TEST( testFindNextHourNoMinute2 );
    CPPUNIT_TEST( testFindNextHourAndWeekdayNotException );
    CPPUNIT_TEST( testFindNextHourAndWeekdayOnlyException );
//    CPPUNIT_TEST(  );
    
    CPPUNIT_TEST_SUITE_END();

private:
    TestablePeriodicTask *task_m;
    bool isOnChangeCalled_m;
    time_t timeref1_m, timeref2_m, timeref3_m;
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
        timeinfo.tm_mday = 5;
        timeref2_m = mktime(&timeinfo);
        timeinfo.tm_hour = 20;
        timeinfo.tm_min = 58;
        timeref3_m = mktime(&timeinfo);
    }

    void tearDown()
    {
        delete task_m;
        Services::reset();
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

    void testFindNextHourAndWeekday()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16, TimeSpec::Wed);

        next = task_m->callFindNext(timeref1_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_wday);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(10, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_wday);

    }

    void testFindNextHourAndWeekday2()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16, TimeSpec::Mon | TimeSpec::Wed | TimeSpec::Sat | TimeSpec::Sun);

        next = task_m->callFindNext(timeref2_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(6, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);
        CPPUNIT_ASSERT_EQUAL(6, timeinfo->tm_wday);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(7, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_wday);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(8, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_wday);

    }

    void testFindNextHourAndMonthDay()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16, 1, -1, -1);

        next = task_m->callFindNext(timeref2_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(2, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

    }

    void testFindNextHourAndMonthLastDay()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16, 31, -1, -1);

        next = task_m->callFindNext(timeref2_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(31, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(31, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(2, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

    }

    void testFindNextHourAndMonth()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16, -1, 6, -1);

        next = task_m->callFindNext(timeref2_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(5, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(2, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(5, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

    }

    void testFindNextOnLastSchedule()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16, 10, 6, 2007);

        next = task_m->callFindNext(timeref2_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(10, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(5, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next == 0);
    }

    void testFindNextHourNoMinute()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(-1, 20);

        next = task_m->callFindNext(timeref1_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(20, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(20, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

    }

    void testFindNextHourNoMinute2()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(-1, 20);

        next = task_m->callFindNext(timeref3_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(59, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(20, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(5, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(20, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(6, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(20, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(6, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(2, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(20, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(6, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);

    }

    void testFindNextHourAndWeekdayNotException()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16, TimeSpec::Wed, TimeSpec::No);
        DaySpec* ds = new DaySpec();
        ds->mday_m = 3;
        ds->mon_m = 0;
        ds->year_m = 107;
        Services::instance()->getExceptionDays()->addDay(ds);        

        next = task_m->callFindNext(timeref1_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(10, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_wday);

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(17, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(0, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_wday);

    }

    void testFindNextHourAndWeekdayOnlyException()
    {
        time_t next;
        struct tm * timeinfo;
        TimeSpec ts1(30, 16, TimeSpec::Wed, TimeSpec::Yes);
        DaySpec* ds = new DaySpec();
        ds->mday_m = 14;
        ds->mon_m = 1;
        ds->year_m = 107;
        Services::instance()->getExceptionDays()->addDay(ds);        

        next = task_m->callFindNext(timeref1_m, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(14, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_mon);
        CPPUNIT_ASSERT_EQUAL(107, timeinfo->tm_year);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_wday);

        DaySpec* ds2 = new DaySpec();
        ds2->mday_m = 1;
        ds2->mon_m = 3;
        Services::instance()->getExceptionDays()->addDay(ds2);        

        next = task_m->callFindNext(next, &ts1);

        CPPUNIT_ASSERT(next != 0);
        timeinfo = localtime(&next);
        
        CPPUNIT_ASSERT_EQUAL(30, timeinfo->tm_min);
        CPPUNIT_ASSERT_EQUAL(16, timeinfo->tm_hour);
        CPPUNIT_ASSERT_EQUAL(1, timeinfo->tm_mday);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_mon);
        CPPUNIT_ASSERT(timeinfo->tm_year > 107);
        CPPUNIT_ASSERT_EQUAL(3, timeinfo->tm_wday);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION( PeriodicTaskTest );
