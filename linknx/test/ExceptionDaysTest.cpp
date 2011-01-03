#include <cppunit/extensions/HelperMacros.h>
#include "timermanager.h"

class ExceptionDaysTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( ExceptionDaysTest );
    CPPUNIT_TEST( testAdd );
    CPPUNIT_TEST( testExportImport );
    CPPUNIT_TEST( testIsException );
    CPPUNIT_TEST( testIsExceptionWildcard );
    CPPUNIT_TEST( testIsExceptionWildcard2 );
    CPPUNIT_TEST_SUITE_END();

private:
    ExceptionDays *exceptiondays_m;
public:
    void setUp()
    {
        exceptiondays_m = new ExceptionDays();
    }

    void tearDown()
    {
        delete exceptiondays_m;
    }

    void testAdd()
    {
        ticpp::Element pConfig;
        pConfig.SetAttribute("day", "23");
        pConfig.SetAttribute("month", "10");
        pConfig.SetAttribute("year", "2007");
        DaySpec* ds = new DaySpec();
        ds->importXml(&pConfig);
        CPPUNIT_ASSERT_EQUAL(23, ds->mday_m);
        CPPUNIT_ASSERT_EQUAL(10-1, ds->mon_m);
        CPPUNIT_ASSERT_EQUAL(2007-1900, ds->year_m);
        
        exceptiondays_m->addDay(ds);

        ticpp::Element pExport;
        exceptiondays_m->exportXml(&pExport);

        ticpp::Iterator< ticpp::Element > child = pExport.FirstChildElement();
        CPPUNIT_ASSERT("date" == child->Value());
        CPPUNIT_ASSERT("23" == child->GetAttribute("day"));
        CPPUNIT_ASSERT("10" == child->GetAttribute("month"));
        CPPUNIT_ASSERT("2007" == child->GetAttribute("year"));
        child++;
        CPPUNIT_ASSERT(child == child.end());
    }

    void testExportImport()
    {
        DaySpec* ds = new DaySpec();
        ds->mday_m = 7;
        ds->mon_m = 4;
        ds->year_m = 107;
        exceptiondays_m->addDay(ds);

        ticpp::Element pExport;
        exceptiondays_m->exportXml(&pExport);
        
        ExceptionDays exceptiondays2;
        exceptiondays2.importXml(&pExport);
        
        time_t time;
        struct tm timeinfo;
        timeinfo.tm_hour = 12;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        timeinfo.tm_mday = 7;
        timeinfo.tm_mon = 4;
        timeinfo.tm_year = 107;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(exceptiondays2.isException(time));

        timeinfo.tm_mday = 7;
        timeinfo.tm_mon = 5;
        timeinfo.tm_year = 107;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(!exceptiondays2.isException(time));
    }

    void testIsException()
    {
        time_t time;
        ticpp::Element pConfig;
        pConfig.SetAttribute("day", "23");
        pConfig.SetAttribute("month", "10");
        pConfig.SetAttribute("year", "2007");
        DaySpec* ds = new DaySpec();
        ds->importXml(&pConfig);
        CPPUNIT_ASSERT_EQUAL(23, ds->mday_m);
        CPPUNIT_ASSERT_EQUAL(10-1, ds->mon_m);
        CPPUNIT_ASSERT_EQUAL(2007-1900, ds->year_m);
        
        exceptiondays_m->addDay(ds);

        struct tm timeinfo;
        timeinfo.tm_hour = 0;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        timeinfo.tm_mday = 23;
        timeinfo.tm_mon = 10-1;
        timeinfo.tm_year = 2007-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(exceptiondays_m->isException(time));

        timeinfo.tm_mday = 24;
        timeinfo.tm_mon = 10-1;
        timeinfo.tm_year = 2007-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(!exceptiondays_m->isException(time));

        timeinfo.tm_mday = 23;
        timeinfo.tm_mon = 9-1;
        timeinfo.tm_year = 2007-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(!exceptiondays_m->isException(time));

        timeinfo.tm_mday = 23;
        timeinfo.tm_mon = 10-1;
        timeinfo.tm_year = 2008-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(!exceptiondays_m->isException(time));

        timeinfo.tm_hour = 23;
        timeinfo.tm_min = 59;
        timeinfo.tm_sec = 59;
        timeinfo.tm_mday = 23;
        timeinfo.tm_mon = 10-1;
        timeinfo.tm_year = 2007-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(exceptiondays_m->isException(time));
    }

    void testIsExceptionWildcard()
    {
        time_t time;
        ticpp::Element pConfig;
        pConfig.SetAttribute("day", "23");
        pConfig.SetAttribute("month", "10");
        DaySpec* ds = new DaySpec();
        ds->importXml(&pConfig);
        
        exceptiondays_m->addDay(ds);

        struct tm timeinfo;
        timeinfo.tm_hour = 0;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        timeinfo.tm_mday = 23;
        timeinfo.tm_mon = 10-1;
        timeinfo.tm_year = 2007-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(exceptiondays_m->isException(time));

        timeinfo.tm_mday = 23;
        timeinfo.tm_mon = 10-1;
        timeinfo.tm_year = 2008-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(exceptiondays_m->isException(time));

        timeinfo.tm_mday = 23;
        timeinfo.tm_mon = 9-1;
        timeinfo.tm_year = 2007-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(!exceptiondays_m->isException(time));
    }

    void testIsExceptionWildcard2()
    {
        time_t time;
        ticpp::Element pConfig;
        pConfig.SetAttribute("day", "1");
        pConfig.SetAttribute("year", "2008");
        DaySpec* ds = new DaySpec();
        ds->importXml(&pConfig);
        
        exceptiondays_m->addDay(ds);

        struct tm timeinfo;
        timeinfo.tm_hour = 6;
        timeinfo.tm_min = 30;
        timeinfo.tm_sec = 0;
        timeinfo.tm_mday = 1;
        timeinfo.tm_mon = 10-1;
        timeinfo.tm_year = 2008-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(exceptiondays_m->isException(time));

        timeinfo.tm_mday = 1;
        timeinfo.tm_mon = 1-1;
        timeinfo.tm_year = 2008-1900;
        timeinfo.tm_hour = 6;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(exceptiondays_m->isException(time));

        timeinfo.tm_mday = 1;
        timeinfo.tm_mon = 1-1;
        timeinfo.tm_year = 2007-1900;
        time = mktime(&timeinfo);
        CPPUNIT_ASSERT(!exceptiondays_m->isException(time));
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION( ExceptionDaysTest );
