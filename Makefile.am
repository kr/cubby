## Process this file with automake to produce Makefile.in

bin_PROGRAMS = cubbyd

dist_doc_DATA = \
  README.md \
  doc/client-protocol.md \
  doc/peer-protocol.md

cubbyd_SOURCES = \
  cubbyd.c \
  blob.c blob.h \
  bundle.c bundle.h \
  http.c http.h \
  net.c net.h \
  prot.c prot.h \
  region.c region.h \
  $(templates) \
  $(shared_sources)

templates = root.html

shared_sources = \
  dirent.c dirent.h \
  sparr.c sparr.h \
  spht.c spht.h \
  util.c util.h

unit_tests = \
  check-dirent.c \
  check-sparr.c \
  check-spht.c \
  check-util.c

EXTRA_DIST = reslink

check_PROGRAMS = cutgen cutcheck
cutgen_SOURCES = cutgen.c util.c util.h
cutcheck_SOURCES = cutcheck.c cut.c cut.h $(unit_tests) $(shared_sources)
CLEANFILES = cutcheck.c $(templates:.html=.o)

cubbyd_LDADD = $(templates:.html=.o)

check: cutcheck
	$(builddir)/$<

cutcheck.c: cutgen $(unit_tests)
	$(builddir)/$< -o $@ $(^:$<=)

$(srcdir)/version.m4:
	printf `git describe` | sed 's/^v//' > $@

%.o: %.s
	$(AS) -o $@ $^

%.s: %.html
	$(srcdir)/reslink $^ $@

ACLOCAL_AMFLAGS = -I m4
