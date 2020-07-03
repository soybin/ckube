PREFIX = /usr/local

ckube: ckube.c
	$(CC) ckube.c -o ckube -w -lm -lncursesw -std=c99
	./ckube

.PHONY: install
install: ckube
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	sudo cp $< $(DESTDIR)$(PREFIX)/bin/ckube

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/ckube
