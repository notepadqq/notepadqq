
EXTRA_DIST = \
	doc \
	images \
	src \
	support_files \
	\
	autogen.sh \
	CONTRIBUTORS.md \
	notepadqq.pro \
	README.md


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
	rm -f $(DIST_ARCHIVES)
	if [ -d ".git" ] && [ $$(which git) ]; then git submodule deinit . ; fi

