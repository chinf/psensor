check-local: checkpatch.pl
	find $(top_srcdir)/src -name \*.c -exec $(srcdir)/checkpatch.pl --ignore SPLIT_STRING --show-types -q --no-tree  -emacs -f {} \;
	find $(top_srcdir)/src -name \*.h -exec $(srcdir)/checkpatch.pl --ignore SPLIT_STRING --show-types -q --no-tree  -emacs -f {} \;

EXTRA_DIST = checkpatch.pl data test-io-dir-list.sh

check_PROGRAMS = test-io-dir-list \
	test-url-encode \
	test-url-normalize

AM_CPPFLAGS = -pedantic -Werror

test_url_encode_SOURCES = test_url_encode.c
test_url_encode_LDADD = ../src/lib/libpsensor.a

test_url_normalize_SOURCES = test_url_normalize.c
test_url_normalize_LDADD = ../src/lib/libpsensor.a

test_io_dir_list_SOURCES = test_io_dir_list.c
test_io_dir_list_LDADD = ../src/lib/libpsensor.a

TESTS = test-url-encode \
	test-url-normalize \
	test-io-dir-list.sh