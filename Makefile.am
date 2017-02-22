AUTOMAKE_OPTIONS=1.9 foreign
ACLOCAL_AMFLAGS = -I m4

if CPPUNIT
CPPUNIT_DIR = test
else
CPPUNIT_DIR =
endif

EXTRA_DIST=linknx.spec

SUBDIRS= ticpp b64 src conf include $(CPPUNIT_DIR)
