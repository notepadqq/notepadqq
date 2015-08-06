
all-local: Makefile.qmake
	$(MAKE) -f $^ $(AM_MAKEFLAGS) all

install-exec-local: Makefile.qmake
	$(MAKE) -f $^ $(AM_MAKEFLAGS) install \
		INSTALL_ROOT=$(DESTDIR) \
		PREFIX=$(PREFIX)

clean-local: Makefile.qmake
	$(MAKE) -f $^ $(AM_MAKEFLAGS) clean

distclean-local: Makefile.qmake
	$(MAKE) -f $^ $(AM_MAKEFLAGS) distclean

Makefile.qmake: notepadqq.pro
	$(QMAKE) -makefile -o $@ $^

clobber: maintainer-clean
	rm -f Makefile.in aclocal.m4 compile configure install-sh missing

