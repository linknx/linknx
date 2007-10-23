# Rules for the test code (use `make check` to execute)
TESTS = testmain
check_PROGRAMS = $(TESTS)
testmain_SOURCES = TimeSpecTest.cpp ExceptionDaysTest.cpp TimerManagerTest.cpp PeriodicTaskTest.cpp testmain.cpp ../src/ruleserver.cpp ../src/objectcontroller.cpp ../src/common.c ../src/eibclient.c ../src/threads.cpp ../src/timermanager.cpp  ../src/persistentstorage.cpp ../src/xmlserver.cpp ../src/smsgateway.cpp ../src/emailgateway.cpp ../src/knxconnection.cpp ../src/services.cpp ../src/common.h ../src/ruleserver.h ../src/objectcontroller.h ../src/threads.h ../src/timermanager.h ../src/persistentstorage.h ../src/xmlserver.h ../src/smsgateway.h ../src/emailgateway.h ../src/knxconnection.h ../src/services.h
testmain_CXXFLAGS = $(CPPUNIT_CFLAGS)
AM_CPPFLAGS=-I$(top_srcdir)/src -I$(top_srcdir)/include -I$(top_srcdir)/ticpp $(PTH_CPPFLAGS) $(LIBCURL_CPPFLAGS)
testmain_LDFLAGS = $(CPPUNIT_LIBS) -ldl
testmain_LDADD=../ticpp/libticpp.a $(PTH_LDFLAGS) $(PTH_LIBS) $(LIBCURL)
