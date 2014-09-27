ifndef DESTDIR
$(error DESTDIR is not set)
endif

.PHONY: all

all:
	mkdir -p "$(DESTDIR)"
	cp -r addon keymap lib mode theme LICENSE "$(DESTDIR)"/
	# Clean unnecessary files within "mode"
	rm -f "$(DESTDIR)"/mode/index.html
	find "$(DESTDIR)"/mode/ -type f -name 'index.html' -exec rm -f {} +