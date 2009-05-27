## Process this file with automake to produce Makefile.in

bin_PROGRAMS = cubbyd

dist_doc_DATA = \
  README.md \
  doc/client-protocol.md \
  doc/peer-protocol.md

cubbyd_SOURCES = \
  cubbyd.c \
  net.c net.h \
  prot.c prot.h \
  store.c store.h \
  $(shared_sources)

shared_sources = \
  sparr.c sparr.h \
  util.c util.h

unit_tests = \
  check-sparr.c

check_PROGRAMS = cutgen cutcheck
cutgen_SOURCES = cutgen.c util.c util.h
cutcheck_SOURCES = cutcheck.c cut.c cut.h $(unit_tests) $(shared_sources)
CLEANFILES = cutcheck.c

check: cutcheck
	$(builddir)/$<

cutcheck.c: cutgen $(unit_tests)
	$(builddir)/$< -o $@ $(^:$<=)

$(srcdir)/version.m4:
	printf `git describe` | sed 's/^v//' > $@

ACLOCAL_AMFLAGS = -I m4
