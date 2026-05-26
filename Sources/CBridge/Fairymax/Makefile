srcdir = .

CC?=gcc
CFLAGS?= -O2 -s
INI_F?=$(DESTDIR)/usr/share/games/fairymax/fmax.ini
INI_Q?=$(DESTDIR)/usr/share/games/fairymax/qmax.ini


ALL= fairymax shamax maxqi fairymax.6.gz

all: ${ALL}

fairymax: fairymax.c
	$(CC) $(CFLAGS) -DINI_FILE=\"${INI_F}\" fairymax.c -o fairymax

shamax: fairymax.c
	$(CC) $(CFLAGS) -DINI_FILE=\"${INI_F}\" -DSHATRANJ fairymax.c -o shamax

maxqi: maxqi.c
	$(CC) $(CFLAGS) -DINI_FILE=\"${INI_Q}\" maxqi.c -o maxqi

install: ${ALL} ${srcdir}/data/*
	cp -u ${srcdir}/fairymax $(DESTDIR)/usr/games
	cp -u ${srcdir}/shamax $(DESTDIR)/usr/games
	cp -u ${srcdir}/maxqi $(DESTDIR)/usr/games
	install -d -m0755 $(DESTDIR)/usr/share/games/fairymax
	cp -u ${srcdir}/data/* $(DESTDIR)/usr/share/games/fairymax
	install -d -m0755 $(DESTDIR)/usr/share/man/man6
	cp -u ${srcdir}/fairymax.6.gz $(DESTDIR)/usr/share/man/man6

fairymax.6.gz: fairymax.pod
	pod2man -s 6 fairymax.pod > fmax.man
	cp fmax.man fairymax.6
	rm -f fairymax.6.gz
	gzip fairymax.6

clean:
	rm -f ${ALL}

dist-clean:
	rm -f ${ALL} *~ data/*~ *.man md5sums

dist:
	install -d -m0755 Fairy-Max
	install -d -m0755 Fairy-Max/data
	rm -f fairymax.tar fairymax.tar.gz
	cp fairymax.c maxqi.c fairymax.pod Makefile README changelog copyright Fairy-Max
	cp data/* Fairy-Max/data
	(md5sum Fairy-Max/* Fairy-Max/data/* > Fairy-Max/md5sums) || true
	tar -cvvf fairymax.tar Fairy-Max
	gzip fairymax.tar
	rm Fairy-Max/data/*
	rmdir Fairy-Max/data
	rm Fairy-Max/*
	rmdir Fairy-Max

uninstall:
	rm -f $(DESTDIR)/usr/share/games/fairymax/*
	rmdir $(DESTDIR)/usr/share/games/fairymax
	rm -f $(DESTDIR)/usr/share/man/man6/fairymax.6.gz
	rm -f $(DESTDIR)/usr/games/fairymax
	rm -f $(DESTDIR)/usr/games/shamax
	rm -f $(DESTDIR)/usr/games/maxqi

