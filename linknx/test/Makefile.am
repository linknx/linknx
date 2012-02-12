# Rules for the test code (use `make check` to execute)
TESTS = testmain
check_PROGRAMS = $(TESTS)
testmain_SOURCES = ObjectControllerTest.cpp ObjectTest.cpp TimeSpecTest.cpp ExceptionDaysTest.cpp TimerManagerTest.cpp PeriodicTaskTest.cpp XmlServerTest.cpp IOPortTest.cpp testmain.cpp ../src/ruleserver.cpp ../src/objectcontroller.cpp ../src/eibclient.c ../src/threads.cpp ../src/timermanager.cpp  ../src/persistentstorage.cpp ../src/xmlserver.cpp ../src/smsgateway.cpp ../src/emailgateway.cpp ../src/knxconnection.cpp ../src/services.cpp ../src/suncalc.cpp ../src/luacondition.cpp ../src/ioport.cpp ../src/logger.cpp ../src/ruleserver.h ../src/objectcontroller.h ../src/threads.h ../src/timermanager.h ../src/persistentstorage.h ../src/xmlserver.h ../src/smsgateway.h ../src/emailgateway.h ../src/knxconnection.h ../src/services.h ../src/suncalc.h ../src/luacondition.h ../src/ioport.h ../src/logger.h
testmain_CXXFLAGS = $(CPPUNIT_CFLAGS)
AM_CPPFLAGS=-I$(top_srcdir)/src -I$(top_srcdir)/include -I$(top_srcdir)/ticpp $(PTH_CPPFLAGS) $(LIBCURL_CPPFLAGS) $(LUA_CFLAGS)
testmain_LDFLAGS = $(CPPUNIT_LIBS) -ldl
testmain_LDADD=../ticpp/libticpp.a $(PTH_LDFLAGS) $(PTH_LIBS) $(LIBCURL) $(LOG4CPP_LIBS) $(LUA_LIBS)
