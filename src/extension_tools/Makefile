ifndef DESTDIR
$(error DESTDIR is not set)
endif

.PHONY: all

all:
	mkdir -p "$(DESTDIR)"
	cp -r node_modules install.js pack.js package.json readmanifest.js "$(DESTDIR)"/
