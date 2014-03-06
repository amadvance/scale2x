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
	configure.windows-x86 configure.windows-x64 autogen.sh \
	doc/scalex.d doc/scalex.1 doc/scalex.txt \
	doc/scalerx.d doc/scalerx.1 doc/scalerx.txt \
	test/test.lst \
	contrib/readme.txt \
	contrib/java/Scale2xImageFilter.java \
	contrib/sdl/scale2x.c \
	contrib/visualc/scale2x.cpp \
	contrib/freebasic/scale2x8BPP.bas \
	contrib/freebasic/scale2x8BPP.exe \
	contrib/ogl2/gpuPeteOGL2-rev3.slv \
	contrib/ogl2/gpuPeteOGL2.slf \
	contrib/ogl2/gpuPeteOGL2.slv \
	contrib/ogl2/sl_scale2x.txt \
	contrib/ogl2/gpuPeteOGL2-rev3.slf \
	contrib/ps/scale2x.fx \
	contrib/ps/scale2x_ps14.fx \
	contrib/ps/scaling.inc \
	contrib/perl/GD-Image-Scale2x-0.02.tar.gz \
	contrib/java2/ImageScale2x.java \
	contrib/java2/ImageScale3x.java \
	contrib/java2/RawScale2x.java \
	contrib/java2/RawScale3x.java

man_MANS = doc/scalex.1 doc/scalerx.1

scalex_SOURCES = \
	scalex.c scalebit.c scalebit.h getopt.c portable.h file.c file.h scale2x.c scale3x.c

scalerx_SOURCES = \
	scalerx.c getopt.c portable.h file.c file.h pixel.c pixel.h

clean-local:
	rm -f tx*.png tr*.png tt*.png
	rm -f check.lst checkrx.lst checkx.lst template.png

# Docs

%.1 : %.d
	advd2 man < $(srcdir)/$< > $@

%.txt : %.d
	advd2 txt < $(srcdir)/$< | todos > $@

# Archives

DIST_WINDOWS = \
	scalex.exe scalerx.exe

DIST_WINDOWS_DOC = \
	doc/scalex.txt doc/scalerx.txt

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

distwindows-x86: $(DIST_WINDOWS)
	rm -f $(PACKAGE)-$(VERSION)-windows-x86.zip
	mkdir tmp
	cp $(DIST_WINDOWS) tmp
	utod < README > tmp/readme.txt
	utod < AUTHORS > tmp/authors.txt
	utod < HISTORY > tmp/history.txt
	utod < COPYING > tmp/copying.txt
	cp $(DIST_WINDOWS_DOC) tmp
	mkdir tmp/example
	cp $(DIST_WINDOWS_EXAMPLE) tmp/example
	find tmp \( -name "*.txt" \) -type f -exec utod {} \;
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-windows-x86.zip *
	rm -r tmp

TEST = 0g01 0g02 0g04 0g16 2c08 2c16 3p01 3p02 3p04 3p08 4a08 4a16 6a08 6a16
EXAMPLE = mslug2-1 test-1

check-local:
	rm -f check.lst checkx.lst checkrx.lst
	echo -- Generate the template.png
	$(TESTENV) ./scalerx$(EXEEXT) -g
	echo -- Test the template
	$(TESTENV) ./scalerx$(EXEEXT) -k 2 -c template.png ttx22.png | fromdos >> check.lst ; \
	$(TESTENV) ./scalerx$(EXEEXT) -k 3 -c template.png ttx33.png | fromdos >> check.lst ; \
	$(TESTENV) ./scalerx$(EXEEXT) -k 4 -c template.png ttx44.png | fromdos >> check.lst ; \
	$(TESTENV) ./scalerx$(EXEEXT) -k 2 -r k -c template.png ttk22.png | fromdos >> check.lst ; \
	$(TESTENV) ./scalerx$(EXEEXT) -k 3 -r k -c template.png ttk33.png | fromdos >> check.lst ; \
	$(TESTENV) ./scalerx$(EXEEXT) -k 4 -r k -c template.png ttk44.png | fromdos >> check.lst ; \
	echo -- Test the basn files
	for i in $(TEST); do \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2 -c $(srcdir)/test/basn$$i.png tr22$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2x3 -c $(srcdir)/test/basn$$i.png tr23$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2x4 -c $(srcdir)/test/basn$$i.png tr24$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 3 -c $(srcdir)/test/basn$$i.png tr33$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 4 -c $(srcdir)/test/basn$$i.png tr44$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2 -c $(srcdir)/test/basn$$i.png tx22$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2x3 -c $(srcdir)/test/basn$$i.png tr23$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2x4 -c $(srcdir)/test/basn$$i.png tr24$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 3 -c $(srcdir)/test/basn$$i.png tx33$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 4 -c $(srcdir)/test/basn$$i.png tx44$$i.png | fromdos >> check.lst ; \
	done
	echo -- Test the example files
	for i in $(EXAMPLE); do \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2 -c $(srcdir)/example/$$i.png tr22$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2x3 -c $(srcdir)/example/$$i.png tr23$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2x4 -c $(srcdir)/example/$$i.png tr24$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 3 -c $(srcdir)/example/$$i.png tr33$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 4 -c $(srcdir)/example/$$i.png tr44$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2 -c $(srcdir)/example/$$i.png tx22$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2x3 -c $(srcdir)/example/$$i.png tr23$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2x4 -c $(srcdir)/example/$$i.png tr24$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 3 -c $(srcdir)/example/$$i.png tx33$$i.png | fromdos >> check.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 4 -c $(srcdir)/example/$$i.png tx44$$i.png | fromdos >> check.lst ; \
	done
	cmp check.lst $(srcdir)/test/test.lst
	echo -- Compare the reference and fast implementation
	for i in $(TEST); do \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2 -c -o $(srcdir)/test/basn$$i.png tr22$$i.png | fromdos >> checkrx.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2x3 -c -o $(srcdir)/test/basn$$i.png tr23$$i.png | fromdos >> checkrx.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2x4 -c -o $(srcdir)/test/basn$$i.png tr24$$i.png | fromdos >> checkrx.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 3 -c -o $(srcdir)/test/basn$$i.png tr33$$i.png | fromdos >> checkrx.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 4 -c -o $(srcdir)/test/basn$$i.png tr43$$i.png | fromdos >> checkrx.lst ; \
	done
	for i in $(EXAMPLE); do \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2 -c -o $(srcdir)/example/$$i.png tr22$$i.png | fromdos >> checkrx.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2x3 -c -o $(srcdir)/example/$$i.png tr23$$i.png | fromdos >> checkrx.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 2x4 -c -o $(srcdir)/example/$$i.png tr24$$i.png | fromdos >> checkrx.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 3 -c -o $(srcdir)/example/$$i.png tr33$$i.png | fromdos >> checkrx.lst ; \
		$(TESTENV) ./scalerx$(EXEEXT) -k 4 -c -o $(srcdir)/example/$$i.png tr43$$i.png | fromdos >> checkrx.lst ; \
	done
	for i in $(TEST); do \
		$(TESTENV) ./scalex$(EXEEXT) -k 2 -c $(srcdir)/test/basn$$i.png tx22$$i.png | fromdos >> checkx.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2x3 -c $(srcdir)/test/basn$$i.png tx23$$i.png | fromdos >> checkx.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2x4 -c $(srcdir)/test/basn$$i.png tx24$$i.png | fromdos >> checkx.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 3 -c $(srcdir)/test/basn$$i.png tx33$$i.png | fromdos >> checkx.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 4 -c $(srcdir)/test/basn$$i.png tx44$$i.png | fromdos >> checkx.lst ; \
	done
	for i in $(EXAMPLE); do \
		$(TESTENV) ./scalex$(EXEEXT) -k 2 -c $(srcdir)/example/$$i.png tx22$$i.png | fromdos >> checkx.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2x3 -c $(srcdir)/example/$$i.png tx23$$i.png | fromdos >> checkx.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 2x4 -c $(srcdir)/example/$$i.png tx24$$i.png | fromdos >> checkx.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 3 -c $(srcdir)/example/$$i.png tx33$$i.png | fromdos >> checkx.lst ; \
		$(TESTENV) ./scalex$(EXEEXT) -k 4 -c $(srcdir)/example/$$i.png tx44$$i.png | fromdos >> checkx.lst ; \
	done
	cmp checkx.lst checkrx.lst
	echo Success!

