check-local: checkpatch.pl
	find $(top_srcdir)/src -name \*.c -exec $(srcdir)/checkpatch.pl -q --no-tree  -emacs -f {} \;
	find $(top_srcdir)/src -name \*.h -exec $(srcdir)/checkpatch.pl -q --no-tree  -emacs -f {} \;

EXTRA_DIST = checkpatch.pl 