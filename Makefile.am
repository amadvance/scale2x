## Process this file with automake to produce Makefile.in

bin_PROGRAMS = scale2x scale3x

EXTRA_DIST = \
	README AUTHORS COPYING HISTORY \
	example/mslug2-1.png \
	example/mslug2-1x.png \
	example/mslug2-2.png \
	example/mslug2-2x.png \
	example/mslug2-3.png \
	example/mslug2-3x.png \
	example/mslug2-4.png \
	example/mslug2-4x.png \
	example/test-1.png \
	example/test-1x.png \
	example/test-2.png \
	example/test-2x.png \
	example/test-3.png \
	example/test-3x.png \
	example/test-4.png \
	example/test-4x.png \
	example/make-example \
	configure.msdos configure.windows autogen.sh noautomake.sh \
	doc/scale2x.d doc/scale2x.1 doc/scale2x.txt doc/scale2x.html \
	doc/history.d doc/history.txt doc/history.html \
	doc/readme.d doc/readme.txt doc/readme.html \
	doc/authors.d doc/authors.txt doc/authors.html \
	contrib/readme.txt \
	contrib/java/Scale2xImageFilter.java \
	contrib/sdl/scale2x.c \
	contrib/visualc/scale2x.cpp \
	doc/scale3x.1

man_MANS = doc/scale2x.1 doc/scale3x.1

scale2x_SOURCES = \
	scale2x.c portable.c portable.h scale2x.h

scale3x_SOURCES = \
	scale3x.c portable.c portable.h scale3x.h

clean-local:
	rm -f *~
	rm -f README AUTHORS HISTORY doc/copying.txt

# Docs

%.1 : %.d
	advd2 man < $(srcdir)/$< > $@

%.txt : %.d
	advd2 txt < $(srcdir)/$< | todos > $@

%.html : %.d
	advd2 html < $(srcdir)/$< > $@

# Archives

README: doc/readme.txt
	cat $< | fromdos > $@

AUTHORS: doc/authors.txt
	cat $< | fromdos > $@

HISTORY: doc/history.txt
	cat $< | fromdos > $@

doc/copying.txt: COPYING
	cat $< | todos > $@

DIST_WINDOWS = \
	scale2x.exe scale3x.exe \
	support/zlib.dll support/libpng.dll \
	doc/copying.txt \
	doc/scale2x.txt doc/readme.txt doc/authors.txt doc/history.txt

DIST_WINDOWS_DOC = \
	doc/scale2x.html doc/readme.html doc/authors.html doc/history.html

DIST_WINDOWS_EXAMPLE = \
	example/mslug2-1.png \
	example/mslug2-1s.png \
	example/mslug2-2.png \
	example/mslug2-2s.png \
	example/mslug2-3.png \
	example/test-1.png \
	example/test-1s.png \
	example/test-2.png \
	example/test-2s.png \
	example/test-3.png

distwindows: $(DIST_WINDOWS)
	rm -f $(PACKAGE)-$(VERSION)-windows-pentium.zip
	mkdir tmp
	cp $(DIST_WINDOWS) tmp
	mkdir tmp/doc
	cp $(DIST_WINDOWS_DOC) tmp/doc
	mkdir tmp/example
	cp $(DIST_WINDOWS_EXAMPLE) tmp/example
	find tmp \( -name "*.txt" \) -type f -exec utod {} \;
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-windows-pentium.zip *
	rm -r tmp

