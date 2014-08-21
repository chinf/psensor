dist_doc_DATA = faq.html faq.txt TODO.txt TODO.html

if ASCIIDOC
faq.html: faq.txt ../www/style.css
	asciidoc --theme style -a themedir=`pwd`/../www -n -a toc faq.txt

TODO.html: faq.txt ../www/style.css
	asciidoc --theme style -a themedir=`pwd`/../www -n -a toc TODO.txt
endif
