#include <cppunit/extensions/HelperMacros.h>
#include "timermanager.h"

class StubTimerTask : public TimerTask
{
public:
    time_t execTime_m;
    bool isOnTimerCalled_m;
    StubTimerTask() : execTime_m(0), isOnTimerCalled_m(false) {};
    virtual void onTimer(time_t time) { isOnTimerCalled_m = true; };
    virtual void reschedule(time_t from = 0) {};
    virtual time_t getExecTime() { return execTime_m; };
    virtual void statusXml(ticpp::Element* pStatus) {};
};

class TimerManagerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( TimerManagerTest );
    CPPUNIT_TEST( testEmptyList );
    CPPUNIT_TEST( testOneTaskFuture );
    CPPUNIT_TEST( testOneTaskNow );
    CPPUNIT_TEST( testOneTaskPast );
    CPPUNIT_TEST( testTwoTasksOrdered );
    CPPUNIT_TEST( testTwoTasksReversed );
    CPPUNIT_TEST( testAddRemove );
    CPPUNIT_TEST_SUITE_END();

private:
    TimerManager *timermanager_m;
    time_t timeref1_m, timeref2_m, timeref3_m;
    StubTimerTask task1_m, task2_m;
public:
    void setUp()
    {
        timermanager_m = new TimerManager();
        struct tm timeinfo;
        timeinfo.tm_hour = 8;
        timeinfo.tm_min = 30;
        timeinfo.tm_sec = 20;
        timeinfo.tm_mday = 31;
        timeinfo.tm_mon = 9;
        timeinfo.tm_year = 107;
        timeref1_m = mktime(&timeinfo);
        timeinfo.tm_sec = 40;
        timeref2_m = mktime(&timeinfo);
        timeinfo.tm_min = 31;
        timeref3_m = mktime(&timeinfo);
    }

    void tearDown()
    {
        delete timermanager_m;
    }

    void testEmptyList()
    {
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref1_m) == TimerManager::Long);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Long);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref3_m) == TimerManager::Long);
    }

    void testOneTaskFuture()
    {
        task1_m.execTime_m = timeref1_m + 5;
        timermanager_m->addTask(&task1_m);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref1_m) == TimerManager::Short);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);

        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref1_m) == TimerManager::Short);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);
    }

    void testOneTaskNow()
    {
        task1_m.execTime_m = timeref1_m + 5;
        timermanager_m->addTask(&task1_m);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Immediate);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == true);

        task1_m.isOnTimerCalled_m = false;
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Long);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);
    }

    void testOneTaskPast()
    {
        task1_m.execTime_m = timeref1_m + 5;
        timermanager_m->addTask(&task1_m);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref3_m) == TimerManager::Immediate);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);

        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref3_m) == TimerManager::Long);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);
    }

    void testTwoTasksOrdered()
    {
        task1_m.execTime_m = timeref1_m + 5;
        task2_m.execTime_m = timeref1_m + 10;
        timermanager_m->addTask(&task1_m);
        timermanager_m->addTask(&task2_m);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Immediate);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == true);
        CPPUNIT_ASSERT(task2_m.isOnTimerCalled_m == false);

        task1_m.isOnTimerCalled_m = false;
        task2_m.isOnTimerCalled_m = false;

        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Immediate);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);
        CPPUNIT_ASSERT(task2_m.isOnTimerCalled_m == true);

        task1_m.isOnTimerCalled_m = false;
        task2_m.isOnTimerCalled_m = false;

        timermanager_m->checkTaskList(timeref2_m);

        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);
        CPPUNIT_ASSERT(task2_m.isOnTimerCalled_m == false);
    }

    void testTwoTasksReversed()
    {
        task1_m.execTime_m = timeref1_m + 5;
        task2_m.execTime_m = timeref1_m + 10;
        timermanager_m->addTask(&task2_m);
        timermanager_m->addTask(&task1_m);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Immediate);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == true);
        CPPUNIT_ASSERT(task2_m.isOnTimerCalled_m == false);

        task1_m.isOnTimerCalled_m = false;
        task2_m.isOnTimerCalled_m = false;

        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Immediate);
        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);
        CPPUNIT_ASSERT(task2_m.isOnTimerCalled_m == true);

        task1_m.isOnTimerCalled_m = false;
        task2_m.isOnTimerCalled_m = false;

        timermanager_m->checkTaskList(timeref2_m);

        CPPUNIT_ASSERT(task1_m.isOnTimerCalled_m == false);
        CPPUNIT_ASSERT(task2_m.isOnTimerCalled_m == false);
    }

    void testAddRemove()
    {
        task1_m.execTime_m = timeref1_m + 5;
        task2_m.execTime_m = timeref1_m + 10;
        timermanager_m->addTask(&task1_m);
        timermanager_m->addTask(&task2_m);
        timermanager_m->removeTask(&task1_m);
        timermanager_m->removeTask(&task2_m);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref1_m) == TimerManager::Long);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Long);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref3_m) == TimerManager::Long);

        timermanager_m->addTask(&task1_m);
        timermanager_m->addTask(&task2_m);
        timermanager_m->removeTask(&task2_m);
        timermanager_m->removeTask(&task1_m);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref1_m) == TimerManager::Long);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Long);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref3_m) == TimerManager::Long);

        timermanager_m->addTask(&task1_m);
        timermanager_m->addTask(&task2_m);
        timermanager_m->removeTask(&task1_m);
        timermanager_m->addTask(&task1_m);
        timermanager_m->removeTask(&task2_m);
        timermanager_m->removeTask(&task1_m);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref1_m) == TimerManager::Long);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref2_m) == TimerManager::Long);
        CPPUNIT_ASSERT(timermanager_m->checkTaskList(timeref3_m) == TimerManager::Long);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION( TimerManagerTest );
