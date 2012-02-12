bin_PROGRAMS = linknx
AM_CPPFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/ticpp $(PTH_CPPFLAGS) $(LIBCURL_CPPFLAGS) $(LOG4CPP_CFLAGS) $(LUA_CFLAGS)
linknx_LDADD=../ticpp/libticpp.a $(PTH_LDFLAGS) $(PTH_LIBS) $(LIBCURL) $(LOG4CPP_LIBS) $(LUA_LIBS) -lm
linknx_SOURCES=linknx.cpp logger.cpp ruleserver.cpp objectcontroller.cpp eibclient.c threads.cpp timermanager.cpp  persistentstorage.cpp xmlserver.cpp smsgateway.cpp emailgateway.cpp knxconnection.cpp services.cpp suncalc.cpp  luacondition.cpp ioport.cpp ruleserver.h objectcontroller.h threads.h timermanager.h persistentstorage.h xmlserver.h smsgateway.h emailgateway.h knxconnection.h services.h suncalc.h luacondition.h ioport.h logger.h
