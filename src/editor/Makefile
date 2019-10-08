ifndef DESTDIR
$(error DESTDIR is not set)
endif

# Cross-platform version of $(shell readlink -m "$(DESTDIR)")
ABSOLUTE_DESTDIR := $(shell mkdir -p $$(dirname "$(DESTDIR)") && cd $$(dirname "$(DESTDIR)") && pwd -P)/$$(basename "$(DESTDIR)")

.PHONY: all

all:
	mkdir -p "$(DESTDIR)"
	cp -r classes styles features app.js index.html init.js Languages.json "$(DESTDIR)"/
	mkdir -p "$(DESTDIR)"/libs
	cp -r libs/jquery "$(DESTDIR)"/libs/jquery/
	cp -r libs/MathJax "$(DESTDIR)"/libs/MathJax/
	cp -r libs/require.js "$(DESTDIR)"/libs/require.js/
	cp -r libs/throttle-debounce "$(DESTDIR)"/libs/throttle-debounce/
	# Call CodeMirror makefile passing DESTDIR as absolute path, so we have no problems with cd-ing to libs
	$(MAKE) -C libs -f Makefile-codemirror DESTDIR="$(ABSOLUTE_DESTDIR)"/libs/codemirror
