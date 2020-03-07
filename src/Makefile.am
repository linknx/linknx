bin_PROGRAMS = linknx
if USE_B64
B64_CFLAGS=-I$(top_srcdir)/b64/include
B64_LIBS=$(top_srcdir)/b64/src/libb64.a
else
B64_CFLAGS=
B64_LIBS=
endif
AM_CPPFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/ticpp $(B64_CFLAGS) $(PTH_CPPFLAGS) $(LIBCURL_CPPFLAGS) $(LOG4CPP_CFLAGS) $(LUA_CFLAGS) $(MYSQL_CFLAGS) $(ESMTP_CFLAGS)
linknx_LDADD=$(top_srcdir)/ticpp/libticpp.a $(LIBICONV) $(B64_LIBS) $(PTH_LDFLAGS) $(PTH_LIBS) $(LIBCURL) $(LOG4CPP_LIBS) $(LUA_LIBS) $(MYSQL_LIBS) $(ESMTP_LIBS) -lm
linknx_SOURCES=linknx.cpp logger.cpp ruleserver.cpp objectcontroller.cpp eibclient.c threads.cpp timermanager.cpp  persistentstorage.cpp xmlserver.cpp smsgateway.cpp emailgateway.cpp knxconnection.cpp services.cpp suncalc.cpp  luacondition.cpp ioport.cpp ruleserver.h objectcontroller.h threads.h timermanager.h persistentstorage.h xmlserver.h smsgateway.h emailgateway.h knxconnection.h services.h suncalc.h luacondition.h ioport.h logger.h
