#include <cppunit/extensions/HelperMacros.h>
#include <cstdlib>
#include "objectcontroller.h"
#include "services.h"
#include "unistd.h"

/** Non regression test for bug described in issue 7.
 * See https://github.com/linknx/linknx/issues/7. */
class Issue7Test : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( Issue7Test );

    CPPUNIT_TEST( testUnicodeWithString14 );
    
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
    }

    void tearDown()
    {
        Services::reset();
    }

    void testUnicodeWithString14()
    {
        const std::string value = "àbcdéfghîjklmn";

		String14Object stringObject;
		stringObject.setValue(value);	
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( Issue7Test );
