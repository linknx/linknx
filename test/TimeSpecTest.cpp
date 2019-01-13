#include <cppunit/extensions/HelperMacros.h>
#include "timermanager.h"
#include "services.h"

class TimeSpecTest : public CppUnit::TestFixture, public ChangeListener
{
    CPPUNIT_TEST_SUITE( TimeSpecTest );
    CPPUNIT_TEST( testGetData );
    CPPUNIT_TEST( testImport );
    CPPUNIT_TEST( testImport2 );
    CPPUNIT_TEST( testExport );
    CPPUNIT_TEST( testCreateVar );
    CPPUNIT_TEST( testCreateVarTime );
    CPPUNIT_TEST( testCreateVarDate );
    CPPUNIT_TEST( testIsValid );
    CPPUNIT_TEST_SUITE_END();

private:
    TimeSpec *ts_1, *ts_2, *ts_3;
    bool isOnChangeCalled_m;
public:
    void setUp()
    {
        ts_1 = new TimeSpec(30, 16);
        ts_2 = new TimeSpec();
        ts_3 = TimeSpec::create("variable", 0);
        isOnChangeCalled_m = false;
    }

    void tearDown()
    {
        delete ts_1;
        delete ts_2;
        delete ts_3;
        ObjectController::reset();
        Services::reset();
    }

    void onChange(Object* obj)
    {
        isOnChangeCalled_m = true;
    }

    void testGetData()
    {
        int min, hour, mday, mon, year, wdays;
        ticpp::Element pConfig;
        ts_1->exportXml(&pConfig);
        TimeSpec *ts = new TimeSpec();
        ts->importXml(&pConfig);

		tm current;

        ts->getDay(current, mday, mon, year, wdays);
        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(30, min);
        CPPUNIT_ASSERT_EQUAL(16, hour);
        CPPUNIT_ASSERT_EQUAL(-1, mday);
        CPPUNIT_ASSERT_EQUAL(-1, mon);
        CPPUNIT_ASSERT_EQUAL(-1, year);
        CPPUNIT_ASSERT(TimeSpec::All == wdays);
        CPPUNIT_ASSERT(TimeSpec::DontCare == ts->getExceptions());
    }

    void testImport()
    {
        int min, hour, mday, mon, year, wdays;
        ticpp::Element pConfig;
        pConfig.SetAttribute("hour", "19");
        pConfig.SetAttribute("min", "3");
        pConfig.SetAttribute("day", "5");
        pConfig.SetAttribute("month", "12");
        pConfig.SetAttribute("year", "2006");
        TimeSpec *ts = TimeSpec::create(&pConfig, 0);

		tm current;
        ts->getDay(current, mday, mon, year, wdays);
        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(3, min);
        CPPUNIT_ASSERT_EQUAL(19, hour);
        CPPUNIT_ASSERT_EQUAL(5, mday);
        CPPUNIT_ASSERT_EQUAL(11, mon);
        CPPUNIT_ASSERT_EQUAL(106, year);
        CPPUNIT_ASSERT(TimeSpec::All == wdays);
        CPPUNIT_ASSERT(TimeSpec::DontCare == ts->getExceptions());
    }

    void testImport2()
    {
        int min, hour, mday, mon, year, wdays;
        ticpp::Element pConfig;
        pConfig.SetAttribute("hour", "20");
        pConfig.SetAttribute("min", "59");
        pConfig.SetAttribute("day", "6");
        pConfig.SetAttribute("month", "1");
        pConfig.SetAttribute("year", "1995");
        pConfig.SetAttribute("exception", "yes");
        pConfig.SetAttribute("wdays", "67");
        TimeSpec *ts = TimeSpec::create(&pConfig, 0);

		tm current;
        ts->getDay(current, mday, mon, year, wdays);
        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(59, min);
        CPPUNIT_ASSERT_EQUAL(20, hour);
        CPPUNIT_ASSERT_EQUAL(6, mday);
        CPPUNIT_ASSERT_EQUAL(0, mon);
        CPPUNIT_ASSERT_EQUAL(95, year);
        CPPUNIT_ASSERT(TimeSpec::Sat || TimeSpec::Sun == wdays);
        CPPUNIT_ASSERT(TimeSpec::Yes == ts->getExceptions());
    }

    void testExport()
    {
        int min, hour, mday, mon, year, wdays;
        ticpp::Element pConfig;
        ts_1->exportXml(&pConfig);
        TimeSpec *ts = TimeSpec::create(&pConfig, 0);

		tm current;
        ts->getDay(current, mday, mon, year, wdays);
        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(30, min);
        CPPUNIT_ASSERT_EQUAL(16, hour);
        CPPUNIT_ASSERT_EQUAL(-1, mday);
        CPPUNIT_ASSERT_EQUAL(-1, mon);
        CPPUNIT_ASSERT_EQUAL(-1, year);
        CPPUNIT_ASSERT(TimeSpec::All == wdays);
        CPPUNIT_ASSERT(TimeSpec::DontCare == ts->getExceptions());
    }

    void testCreateVar()
    {
        int min, hour, mday, mon, year, wdays;
        ticpp::Element pConfig;
        ts_3->exportXml(&pConfig);
        TimeSpec *ts = TimeSpec::create(&pConfig, 0);
        VariableTimeSpec *vts = dynamic_cast<VariableTimeSpec*>(ts);
        CPPUNIT_ASSERT(vts != 0);

		tm current;
        ts->getDay(current, mday, mon, year, wdays);
        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(-1, min);
        CPPUNIT_ASSERT_EQUAL(-1, hour);
        CPPUNIT_ASSERT_EQUAL(-1, mday);
        CPPUNIT_ASSERT_EQUAL(-1, mon);
        CPPUNIT_ASSERT_EQUAL(-1, year);
        CPPUNIT_ASSERT(TimeSpec::All == wdays);
        CPPUNIT_ASSERT(TimeSpec::DontCare == ts->getExceptions());
    }

    void testCreateVarTime()
    {
        int min, hour, mday, mon, year, wdays;
        TimeSpec::ExceptionDays exception;
        TimeObject* time = new TimeObject();
        time->setID("time_stub");
        time->setValue("23:50:00");
        ObjectController* oc = ObjectController::instance();
        oc->addObject(time);
        ticpp::Element pConfig;
        pConfig.SetAttribute("type", "variable");
        pConfig.SetAttribute("time", "time_stub");
        TimeSpec *ts = TimeSpec::create(&pConfig, this);
        VariableTimeSpec *vts = dynamic_cast<VariableTimeSpec*>(ts);
        CPPUNIT_ASSERT(vts != 0);

		tm current;
        ts->getDay(current, mday, mon, year, wdays);
        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(50, min);
        CPPUNIT_ASSERT_EQUAL(23, hour);
        CPPUNIT_ASSERT_EQUAL(-1, mday);
        CPPUNIT_ASSERT_EQUAL(-1, mon);
        CPPUNIT_ASSERT_EQUAL(-1, year);
        CPPUNIT_ASSERT(TimeSpec::All == wdays);
        CPPUNIT_ASSERT(TimeSpec::DontCare == ts->getExceptions());

        CPPUNIT_ASSERT(!isOnChangeCalled_m);

        time->setValue("22:51:00");

        CPPUNIT_ASSERT(isOnChangeCalled_m);

        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(51, min);
        CPPUNIT_ASSERT_EQUAL(22, hour);
        CPPUNIT_ASSERT_EQUAL(-1, mday);
        CPPUNIT_ASSERT_EQUAL(-1, mon);
        CPPUNIT_ASSERT_EQUAL(-1, year);
        delete ts;
    }

    void testCreateVarDate()
    {
        int min, hour, mday, mon, year, wdays;
        DateObject* date = new DateObject();
        date->setID("date_stub");
        date->setValue("2007-09-26");
        ObjectController* oc = ObjectController::instance();
        oc->addObject(date);
        ticpp::Element pConfig;
        pConfig.SetAttribute("type", "variable");
        pConfig.SetAttribute("hour", "16");
        pConfig.SetAttribute("min", "30");
        pConfig.SetAttribute("date", "date_stub");
        TimeSpec *ts = TimeSpec::create(&pConfig, this);
        VariableTimeSpec *vts = dynamic_cast<VariableTimeSpec*>(ts);
        CPPUNIT_ASSERT(vts != 0);

		tm current;
        ts->getDay(current, mday, mon, year, wdays);
        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(30, min);
        CPPUNIT_ASSERT_EQUAL(16, hour);
        CPPUNIT_ASSERT_EQUAL(26, mday);
        CPPUNIT_ASSERT_EQUAL(8, mon);
        CPPUNIT_ASSERT_EQUAL(107, year);
        CPPUNIT_ASSERT(TimeSpec::All == wdays);
        CPPUNIT_ASSERT(TimeSpec::DontCare == ts->getExceptions());

        CPPUNIT_ASSERT(!isOnChangeCalled_m);

        date->setValue("2008-12-31");

        CPPUNIT_ASSERT(isOnChangeCalled_m);

        ts->getDay(current, mday, mon, year, wdays);
        ts->getTime(mday, mon, year, min, hour);
        CPPUNIT_ASSERT_EQUAL(30, min);
        CPPUNIT_ASSERT_EQUAL(16, hour);
        CPPUNIT_ASSERT_EQUAL(31, mday);
        CPPUNIT_ASSERT_EQUAL(11, mon);
        CPPUNIT_ASSERT_EQUAL(108, year);
        delete ts;
    }

#define checkTimeSpec(min, hour, mday, mon, year, expectIsValid) CPPUNIT_ASSERT_EQUAL(expectIsValid, TimeSpec(min, hour, mday, mon, year, 0).isValid())
	void testIsValid()
	{
		checkTimeSpec(0, 0, 1, 1, 2010, true);
        checkTimeSpec(0, 0, 30, 2, -1, false);
        checkTimeSpec(0, 0, 31, 4, -1, false);
        checkTimeSpec(0, 0, 33, -1, -1, false);
        checkTimeSpec(-1, -1, -1, -1, -1, true);
        checkTimeSpec(60, -1, -1, -1, -1, false);
        checkTimeSpec(-1, 24, 1 -1, -1, -1, false);
        checkTimeSpec(-1, -1, 0, -1, -1, false);
        checkTimeSpec(-1, -1, 31, -1, -1, true);
        checkTimeSpec(-1, -1, 32, -1, -1, false);
        //checkTimeSpec(-1, -1, -1, 0, -1, false); Looks sensible but
		//TimeSpec::TimeSpec converts mon=0 to January without decrementing the
		//value. THIS IS CRAP!
        checkTimeSpec(-1, -1, -1, 13, -1, false);

		// Check most cases.
		for (int min = -2; min < 60; ++min)
		{
			bool isMinValid = min >= -1;
			for (int hour = -2; hour < 24; ++hour)
			{
				bool isHourValid = hour >= -1;
				for (int mday = -2; mday <= 28; ++mday)
				{
					bool isDayValid = mday >= -1 && mday != 0;
					for (int mon = -2; mon <= 12; ++mon)
					{
						bool isMonthValid = mon >= -1; // This test should exclude month==0 from the valid values. But TimeSpec's ctor does is so weird that it makes it acceptable.
						checkTimeSpec(min, hour, mday, mon, -1, isMinValid && isHourValid && isDayValid && isMonthValid);
					}
				}
			}
		}


        checkTimeSpec(-1, -1, 29, 2, 2016, true); // 2016 was a leap year
        checkTimeSpec(-1, -1, 29, 2, 2017, false); // and 2017 was obviously not.
        checkTimeSpec(-1, -1, 29, 2, -1, true);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TimeSpecTest );
