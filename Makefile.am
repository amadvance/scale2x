## Process this file with automake to produce Makefile.in

bin_PROGRAMS = scalex scalerx

EXTRA_DIST = \
	scale2x.h scale3x.h \
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
	test/basn0g01.png \
	test/basn0g02.png \
	test/basn0g04.png \
	test/basn0g08.png \
	test/basn0g16.png \
	test/basn2c08.png \
	test/basn2c16.png \
	test/basn3p01.png \
	test/basn3p02.png \
	test/basn3p04.png \
	test/basn3p08.png \
	test/basn4a08.png \
	test/basn4a16.png \
	test/basn6a08.png \
	test/basn6a16.png \
	configure.msdos configure.windows autogen.sh noautomake.sh \
	doc/scalex.d doc/scalex.1 doc/scalex.txt doc/scalex.html \
	doc/scalerx.d doc/scalerx.1 doc/scalerx.txt doc/scalerx.html \
	doc/history.d doc/history.txt doc/history.html \
	doc/readme.d doc/readme.txt doc/readme.html \
	doc/authors.d doc/authors.txt doc/authors.html \
	contrib/readme.txt \
	contrib/java/Scale2xImageFilter.java \
	contrib/sdl/scale2x.c \
	contrib/visualc/scale2x.cpp \
	check.lst

man_MANS = doc/scalex.1 doc/scalerx.1

scalex_SOURCES = \
	scalex.c scalebit.c scalebit.h getopt.c portable.h file.c file.h

scalerx_SOURCES = \
	scalerx.c getopt.c portable.h file.c file.h pixel.c pixel.h

clean-local:
	rm -f tx*.png tr*.png
	rm -f new.lst

maintainer-clean-local:
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
	scalex.exe scalerx.exe \
	support/zlib.dll support/libpng.dll \
	doc/copying.txt \
	doc/readme.txt doc/authors.txt doc/history.txt

DIST_WINDOWS_DOC = \
	doc/scalex.html doc/scalerx.html doc/readme.txt doc/authors.html doc/history.html \
	doc/scalex.txt doc/scalerx.txt doc/readme.html doc/authors.txt doc/history.txt

DIST_WINDOWS_EXAMPLE = \
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
	example/test-4x.png

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

TEST = 0g01 0g02 0g04 0g16 2c08 2c16 3p01 3p02 3p04 3p08 4a08 4a16 6a08 6a16

check-local:
	rm -f new.lst
	for i in $(TEST); do \
		./scalerx -k 2 -c $(srcdir)/test/basn$$i.png tr2x$$i.png >> new.lst ; \
		./scalerx -k 3 -c $(srcdir)/test/basn$$i.png tr3x$$i.png >> new.lst ; \
		./scalerx -k 4 -c $(srcdir)/test/basn$$i.png tr4x$$i.png >> new.lst ; \
		./scalex -k 2 -c $(srcdir)/test/basn$$i.png tx2x$$i.png >> new.lst ; \
		./scalex -k 3 -c $(srcdir)/test/basn$$i.png tx3x$$i.png >> new.lst ; \
		./scalex -k 4 -c $(srcdir)/test/basn$$i.png tx4x$$i.png >> new.lst ; \
	done
	cmp new.lst $(srcdir)/check.lst


