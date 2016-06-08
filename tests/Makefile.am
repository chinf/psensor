check-local: checkpatch.pl
	find $(top_srcdir)/src -name \*.c -exec $(srcdir)/checkpatch.pl --ignore SPLIT_STRING --show-types -q --no-tree  -emacs -f {} \;
	find $(top_srcdir)/src -name \*.h -exec $(srcdir)/checkpatch.pl --ignore SPLIT_STRING --show-types -q --no-tree  -emacs -f {} \;

DEFS = -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\" @DEFS@

EXTRA_DIST = checkpatch.pl \
	spelling.txt \
	test-cppcheck.sh \
	test-io-dir-list.sh

check_PROGRAMS = test-io-dir-list \
	test-psensor-type-to-unit-str \
	test-psensor-value-to-str \
	test-url-encode \
	test-url-normalize

AM_CPPFLAGS = -Wall -Werror

LIBS += ../src/lib/libpsensor.a \
	$(SENSORS_LIBS)

if ATASMART
LIBS += $(ATASMART_LIBS)
endif

if GTOP
LIBS += $(GTOP_LIBS)
endif

test_io_dir_list_SOURCES = test_io_dir_list.c
test_psensor_type_to_unit_str_SOURCES = test_psensor_type_to_unit_str.c
test_psensor_type_to_unit_str_CFLAGS = -I$(top_srcdir)/src/lib
test_psensor_value_to_str_SOURCES = test_psensor_value_to_str.c
test_psensor_value_to_str_CFLAGS = -I$(top_srcdir)/src/lib
test_url_encode_SOURCES = test_url_encode.c
test_url_normalize_SOURCES = test_url_normalize.c

TESTS = test-io-dir-list.sh \
	test-psensor-type-to-unit-str \
	test-psensor-value-to-str \
	test-url-encode \
	test-url-normalize

if CPPCHECK
TESTS += test-cppcheck.sh
endif
