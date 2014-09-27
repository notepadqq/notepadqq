ifndef DESTDIR
$(error DESTDIR is not set)
endif

.PHONY: all

all:
	mkdir -p "$(DESTDIR)"
	cp -r classes styles app.js index.html init.js "$(DESTDIR)"/
	mkdir -p "$(DESTDIR)"/libs
	cp -r libs/jquery "$(DESTDIR)"/libs/jquery/
	# Call CodeMirror makefile passing DESTDIR as absolute path, so we have no problems with cd-ing to libs
	$(MAKE) -C libs -f Makefile-codemirror DESTDIR="$(shell readlink -m "$(DESTDIR)")"/libs/codemirror
