ifndef DESTDIR
$(error DESTDIR is not set)
endif

ABSOLUTE_DESTDIR := $(shell mkdir -p $$(dirname "$(DESTDIR)") && cd $$(dirname "$(DESTDIR)") && pwd -P)/$$(basename "$(DESTDIR)")

.PHONY: all

all:
	mkdir -p "$(DESTDIR)"
	cp -r classes styles app.js index.html init.js "$(DESTDIR)"/
	mkdir -p "$(DESTDIR)"/libs
	cp -r libs/jquery "$(DESTDIR)"/libs/jquery/
	# Call CodeMirror makefile passing DESTDIR as absolute path, so we have no problems with cd-ing to libs
	$(MAKE) -C libs -f Makefile-codemirror DESTDIR="$(ABSOLUTE_DESTDIR)"/libs/codemirror
