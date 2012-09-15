dist_doc_DATA = faq.html faq.txt

faq.html: faq.txt ../www/style.css
	asciidoc -v --theme style -a themedir=`pwd`/../www -n -a toc faq.txt 