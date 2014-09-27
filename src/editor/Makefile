ifndef DESTDIR
$(error DESTDIR is not set)
endif

.PHONY: all

all:
	mkdir -p "$(DESTDIR)"
	cp -r classes styles app.js index.html init.js "$(DESTDIR)"/
	mkdir -p "$(DESTDIR)"/libs
	cp -r libs/jquery "$(DESTDIR)"/libs/jquery/
	cd libs/codemirror && $(MAKE) DESTDIR=../../"$(DESTDIR)"/libs/codemirror