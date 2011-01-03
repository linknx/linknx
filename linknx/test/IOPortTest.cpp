#include <cppunit/extensions/HelperMacros.h>
#include "ioport.h"
#include "services.h"

class IOPortTest : public CppUnit::TestFixture, public ChangeListener, public TxAction
{
    CPPUNIT_TEST_SUITE( IOPortTest );
    CPPUNIT_TEST( testTxAction );
    CPPUNIT_TEST( testTxActionHex );
    CPPUNIT_TEST_SUITE_END();

private:
    bool isOnChangeCalled_m;
public:
    void setUp()
    {
//        isOnChangeCalled_m = false;
    }

    void tearDown()
    {
        IOPortManager::reset();
    }

    class StubIOPort : public IOPort
    {
    public:
        StubIOPort() : buffer(0), buflen(0) {};
        virtual ~StubIOPort() { if (buffer) delete buffer; };

        int send(const uint8_t* buf, int len) {
            if (buffer == 0)
            {
                buflen = len;
                buffer = new uint8_t[len];
                memcpy(buffer, buf, len);
                return len;
            }
            return -1;
        };
        int get(uint8_t* buf, int len, pth_event_t stop) {
            if (len < buflen)
                return -1;
            memcpy(buf, buffer, buflen);
            return buflen;
        };
        virtual bool isRxEnabled() { return true; };

        int buflen;
        uint8_t* buffer;
    };

    void onChange(Object* obj)
    {
        isOnChangeCalled_m = true;
    }

    void testTxAction()
    {
        const char *testdata = "the data content";
        StubIOPort *port = new StubIOPort();
        port->setID("testport");
        IOPortManager::instance()->addPort(port);
        ticpp::Element pActionConfig("action");
        pActionConfig.SetAttribute("type", "ioport-tx");
        pActionConfig.SetAttribute("ioport", "testport");
        pActionConfig.SetAttribute("data", testdata);
        TxAction* action = dynamic_cast<TxAction*>(Action::create(&pActionConfig));

        action->sendData(port);

        CPPUNIT_ASSERT(strlen(testdata) == port->buflen);
        for (int i=0; i<strlen(testdata); i++)
        {
            CPPUNIT_ASSERT(testdata[i] == port->buffer[i]);
        }
    }

    void testTxActionExportImport()
    {
        const char *testdata = "the data content";
        StubIOPort *port = new StubIOPort();
        port->setID("testport");
        IOPortManager::instance()->addPort(port);
        ticpp::Element pActionConfig("action");
        pActionConfig.SetAttribute("type", "ioport-tx");
        pActionConfig.SetAttribute("ioport", "testport");
        pActionConfig.SetAttribute("data", testdata);
        TxAction* action = dynamic_cast<TxAction*>(Action::create(&pActionConfig));

        ticpp::Element pConfig;
        action->exportXml(&pConfig);
        TxAction* action2 = dynamic_cast<TxAction*>(Action::create(&pConfig));

        action2->sendData(port);

        CPPUNIT_ASSERT(strlen(testdata) == port->buflen);
        for (int i=0; i<strlen(testdata); i++)
        {
            CPPUNIT_ASSERT(testdata[i] == port->buffer[i]);
        }

    }

    void testTxActionHex()
    {
        const char *testdata = "\x01\x3a\x02\x03\x04\x00\x06";
        int testdatalen = 7;
        StubIOPort *port = new StubIOPort();
        port->setID("testport");
        IOPortManager::instance()->addPort(port);
        ticpp::Element pActionConfig("action");
        pActionConfig.SetAttribute("type", "ioport-tx");
        pActionConfig.SetAttribute("ioport", "testport");
        pActionConfig.SetAttribute("hex", "true");
        pActionConfig.SetAttribute("data", "013a0203040006");
        TxAction* action = dynamic_cast<TxAction*>(Action::create(&pActionConfig));

        action->sendData(port);

        CPPUNIT_ASSERT(testdatalen == port->buflen);
        for (int i=0; i<testdatalen; i++)
        {
            CPPUNIT_ASSERT(testdata[i] == port->buffer[i]);
        }
    }

    void testTxActionHexExportImport()
    {
        const char *testdata = "\x01\x3a\x02\x03\x04\x00\x06";
        int testdatalen = 7;
        StubIOPort *port = new StubIOPort();
        port->setID("testport");
        IOPortManager::instance()->addPort(port);
        ticpp::Element pActionConfig("action");
        pActionConfig.SetAttribute("type", "ioport-tx");
        pActionConfig.SetAttribute("ioport", "testport");
        pActionConfig.SetAttribute("hex", "true");
        pActionConfig.SetAttribute("data", "013a0203040006");
        TxAction* action = dynamic_cast<TxAction*>(Action::create(&pActionConfig));

        ticpp::Element pConfig;
        action->exportXml(&pConfig);
        TxAction* action2 = dynamic_cast<TxAction*>(Action::create(&pConfig));

        action2->sendData(port);

        CPPUNIT_ASSERT(testdatalen == port->buflen);
        for (int i=0; i<testdatalen; i++)
        {
            CPPUNIT_ASSERT(testdata[i] == port->buffer[i]);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( IOPortTest );
