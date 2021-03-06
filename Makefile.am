## Process this file with automake to produce Makefile.in

bin_PROGRAMS = cubbyd

AM_CFLAGS = $(CUBBY_CFLAGS) $(GCOV_CFLAGS)

dist_doc_DATA = \
  README.md \
  doc/client-protocol.md \
  doc/peer-protocol.md

cubbyd_SOURCES = \
  cubbyd.c \
  http.c http.h \
  net.c net.h \
  $(templates) \
  $(shared_sources)

templates = root.html

shared_sources = \
  arr.c arr.h \
  blob.c blob.h \
  bundle.c bundle.h \
  cpkt.c cpkt.h \
  dirent.c dirent.h \
  error_codes.h \
  heap.c heap.h \
  key.c key.h \
  manager.c manager.h \
  node.c node.h \
  peer.c peer.h \
  prot.c prot.h \
  region.c region.h \
  sha512.c sha512.h \
  sparr.c sparr.h \
  spht.c spht.h \
  util.c util.h

unit_tests = \
  check-arr.c \
  check-bundle.c \
  check-cpkt.c \
  check-dirent.c \
  check-heap.c \
  check-key.c \
  check-manager.c \
  check-node.c \
  check-peer.c \
  check-region.c \
  check-sha512.c \
  check-sparr.c \
  check-spht.c \
  check-util.c

EXTRA_DIST = reslink

check_PROGRAMS = cutgen cutcheck
cutgen_SOURCES = cutgen.c
cutcheck_SOURCES = cutcheck.c cut.c cut.h $(unit_tests) $(shared_sources)
MOSTLYCLEANFILES = cutcheck.c $(templates:.html=.o)

mostlyclean-local:
	rm -rf *.gcda *.gcno

clean-local:
	rm -rf *.gcov

cubbyd_LDADD = $(templates:.html=.o)

check: cutcheck
	$(builddir)/$<
	$(srcdir)/check.py

cutcheck.c: cutgen $(unit_tests)
	$(builddir)/$< -o $@ $(^:$<=)

$(srcdir)/version.m4: | update-version-if-necessary
	printf `git describe` | sed 's/^v//' > $@

update-version-if-necessary:
	@$(srcdir)/check-version $(srcdir)/version.m4

%.o: %.s
	$(AS) -o $@ $^

%.s: %.html
	$(srcdir)/reslink $^ $@

.PHONY: update-version-if-necessary

ACLOCAL_AMFLAGS = -I m4
