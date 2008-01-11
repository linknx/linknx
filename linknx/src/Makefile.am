bin_PROGRAMS = linknx
AM_CPPFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/ticpp $(PTH_CPPFLAGS) $(LIBCURL_CPPFLAGS)
linknx_LDADD=../ticpp/libticpp.a $(PTH_LDFLAGS) $(PTH_LIBS) $(LIBCURL) -lm
linknx_SOURCES=linknx.cpp ruleserver.cpp objectcontroller.cpp common.c eibclient.c threads.cpp timermanager.cpp  persistentstorage.cpp xmlserver.cpp smsgateway.cpp emailgateway.cpp knxconnection.cpp services.cpp common.h ruleserver.h objectcontroller.h threads.h timermanager.h persistentstorage.h xmlserver.h smsgateway.h emailgateway.h knxconnection.h services.h
