# Rules for the test code (use `make check` to execute)
if USE_B64
B64_CFLAGS=-I$(top_srcdir)/b64/include
B64_LIBS=$(top_srcdir)/b64/src/libb64.a
else
B64_CFLAGS=
B64_LIBS=
endif

AUTOMAKE_OPTIONS = subdir-objects
TESTS = testmain
check_PROGRAMS = $(TESTS)
testmain_SOURCES = ObjectControllerTest.cpp ObjectTest.cpp ObjectTest2.cpp TimeSpecTest.cpp ExceptionDaysTest.cpp TimerManagerTest.cpp PeriodicTaskTest.cpp XmlServerTest.cpp IOPortTest.cpp Issue7.cpp RuleTest.cpp testmain.cpp ../src/ruleserver.cpp ../src/objectcontroller.cpp ../src/eibclient.c ../src/threads.cpp ../src/timermanager.cpp  ../src/persistentstorage.cpp ../src/xmlserver.cpp ../src/smsgateway.cpp ../src/emailgateway.cpp ../src/knxconnection.cpp ../src/services.cpp ../src/suncalc.cpp ../src/luacondition.cpp ../src/ioport.cpp ../src/logger.cpp ../src/ruleserver.h ../src/objectcontroller.h ../src/threads.h ../src/timermanager.h ../src/persistentstorage.h ../src/xmlserver.h ../src/smsgateway.h ../src/emailgateway.h ../src/knxconnection.h ../src/services.h ../src/suncalc.h ../src/luacondition.h ../src/ioport.h ../src/logger.h
testmain_CXXFLAGS = $(CPPUNIT_CFLAGS)
AM_CPPFLAGS=-I$(top_srcdir)/src -I$(top_srcdir)/include -I$(top_srcdir)/ticpp $(B64_CFLAGS) $(PTH_CPPFLAGS) $(LIBCURL_CPPFLAGS) $(LUA_CFLAGS) $(MYSQL_CFLAGS) $(ESMTP_CFLAGS)
testmain_LDADD=../ticpp/libticpp.a $(B64_LIBS) $(PTH_LDFLAGS) $(PTH_LIBS) $(LIBCURL) $(LOG4CPP_LIBS) $(LUA_LIBS) $(MYSQL_LIBS) $(CPPUNIT_LIBS) $(ESMTP_LIBS) -ldl
