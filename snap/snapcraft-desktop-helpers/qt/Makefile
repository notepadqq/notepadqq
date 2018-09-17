#!/usr/bin/make -f

FLAVOR ?= qt5
SRC_DIR ?= .

DEST_LAUNCHER = desktop-launch
FLAVOR_FILE = flavor-select

arch_triplet := $(shell dpkg-architecture -q DEB_TARGET_MULTIARCH)

define QT5_CONF
./usr/lib/$(arch_triplet)/qt5/bin
./usr/lib/$(arch_triplet)
endef
export QT5_CONF

define QT4_CONF
./usr/lib/$(arch_triplet)/qt4/bin
./usr/lib/$(arch_triplet)
endef
export QT4_CONF


build: $(DEST_LAUNCHER)

clean:
	rm -f $(DEST_LAUNCHER)
	rm -rf snappy-qt*.conf
	rm -f $(FLAVOR_FILE)

desktop-launch:
	@echo "#!/bin/bash" > $(DEST_LAUNCHER)
	@cat $(SRC_DIR)/init >> $(DEST_LAUNCHER)
	@cat $(SRC_DIR)/runtime-exports >> $(DEST_LAUNCHER)
	@cat $(SRC_DIR)/desktop-exports >> $(DEST_LAUNCHER)
	@cat $(SRC_DIR)/launcher-specific >> $(DEST_LAUNCHER)
	@cat $(SRC_DIR)/mark-and-exec >> $(DEST_LAUNCHER)
	@echo "$$QT5_CONF" > snappy-qt5.conf
	@echo "$$QT4_CONF" > snappy-qt4.conf
	@echo "USE_$(FLAVOR)=true" >> $(FLAVOR_FILE)
		
install: desktop-launch
	install -D -m755 $(DEST_LAUNCHER) "$(DESTDIR)"/bin/$(DEST_LAUNCHER)
	install -D -m644 snappy-qt5.conf \
		"$(DESTDIR)"/etc/xdg/qtchooser/snappy-qt5.conf
	install -D -m644 snappy-qt4.conf \
		"$(DESTDIR)"/etc/xdg/qtchooser/snappy-qt4.conf
	install -D -m644 $(FLAVOR_FILE) "$(DESTDIR)"/

