#include <cppunit/extensions/HelperMacros.h>
#include "xmlserver.h"
extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

class XmlServerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( XmlServerTest );
    CPPUNIT_TEST( testReadEmptyMessage );
    CPPUNIT_TEST( testReadMessage );
    CPPUNIT_TEST( testReadUnterminatedMessage );
    CPPUNIT_TEST( testReadMultipleMessage );
    CPPUNIT_TEST( testReadLongMessage );
//    CPPUNIT_TEST(  );
    
    CPPUNIT_TEST_SUITE_END();

private:
    ClientConnection* cc_m;
public:
    void setUp()
    {
        cc_m = 0; 
        system ("rm -rf /tmp/linknx_unittest_tmp");
    }

    void tearDown()
    {
        if (cc_m)
            delete(cc_m);
    }

    int createMsgFd(const char *msg)
    {
        int fd = creat("/tmp/linknx_unittest_tmp", 00644);
        write(fd, msg, strlen(msg));
        close(fd);
        return open("/tmp/linknx_unittest_tmp", O_RDONLY);
    }
    
    void testReadEmptyMessage()
    {
        pth_event_t stop = pth_event(PTH_EVENT_TIME, pth_timeout(1,0));
        cc_m = new ClientConnection(NULL, createMsgFd(""));
        CPPUNIT_ASSERT_EQUAL(-1, cc_m->readmessage(stop));
    }

    void testReadMessage()
    {
        pth_event_t stop = pth_event(PTH_EVENT_TIME, pth_timeout(1,0));
        cc_m = new ClientConnection(NULL, createMsgFd("test\004"));
        CPPUNIT_ASSERT_EQUAL(1, cc_m->readmessage(stop));
        CPPUNIT_ASSERT(cc_m->msg_m == "test");
        CPPUNIT_ASSERT_EQUAL(-1, cc_m->readmessage(stop));
    }

    void testReadUnterminatedMessage()
    {
        pth_event_t stop = pth_event(PTH_EVENT_TIME, pth_timeout(1,0));
        cc_m = new ClientConnection(NULL, createMsgFd("a message without ending ascii 0x04"));
        CPPUNIT_ASSERT_EQUAL(-1, cc_m->readmessage(stop));
    }

    void testReadMultipleMessage()
    {
        pth_event_t stop = pth_event(PTH_EVENT_TIME, pth_timeout(1,0));
        cc_m = new ClientConnection(NULL, createMsgFd("test\004second message\004"));
        CPPUNIT_ASSERT_EQUAL(1, cc_m->readmessage(stop));
        CPPUNIT_ASSERT(cc_m->msg_m == "test");
        CPPUNIT_ASSERT_EQUAL(1, cc_m->readmessage(stop));
        CPPUNIT_ASSERT(cc_m->msg_m == "second message");
        CPPUNIT_ASSERT_EQUAL(-1, cc_m->readmessage(stop));
    }

    void testReadLongMessage()
    {
        const char *msg = "first part must be at least 256 bytes long, first part must be at least 256 bytes long, first part must be at least 256 bytes long, first part must be at least 256 bytes long, first part must be at least 256 bytes long, first part must be at least 256 byte, and this is second part\004";
        pth_event_t stop = pth_event(PTH_EVENT_TIME, pth_timeout(1,0));
        cc_m = new ClientConnection(NULL, createMsgFd(msg));
        CPPUNIT_ASSERT_EQUAL(1, cc_m->readmessage(stop));
        cc_m->msg_m.push_back('\004');
        CPPUNIT_ASSERT(cc_m->msg_m == msg);
        CPPUNIT_ASSERT_EQUAL(-1, cc_m->readmessage(stop));
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION( XmlServerTest );
