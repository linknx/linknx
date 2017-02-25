AUTOMAKE_OPTIONS=1.9 foreign
ACLOCAL_AMFLAGS = -I m4

if CPPUNIT
CPPUNIT_DIR = test
else
CPPUNIT_DIR =
endif

if USE_B64
B64_DIR = b64
else
B64_DIR =
endif

EXTRA_DIST=linknx.spec

SUBDIRS= ticpp $(B64_DIR) src conf include $(CPPUNIT_DIR)
