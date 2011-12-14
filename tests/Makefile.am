check-local: checkpatch.pl
	find $(top_srcdir)/src -name \*.c -exec $(srcdir)/checkpatch.pl -q --no-tree  -emacs -f {} \;
	find $(top_srcdir)/src -name \*.h -exec $(srcdir)/checkpatch.pl -q --no-tree  -emacs -f {} \;

EXTRA_DIST = checkpatch.pl 

check_PROGRAMS = test-url-encode test-url-normalize

test_url_encode_SOURCES = test_url_encode.c
test_url_encode_LDADD = ../src/lib/libpsensor.a

test_url_normalize_SOURCES = test_url_normalize.c
test_url_normalize_LDADD = ../src/lib/libpsensor.a

TESTS = test-url-encode \
	test-url-normalize