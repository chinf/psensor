SUBDIRS = doc \
	po \
	src \
	icons/hicolor/scalable \
	icons/hicolor/14x14 \
	icons/hicolor/16x16 \
	icons/hicolor/22x22 \
	icons/hicolor/24x24 \
	icons/hicolor/32x32 \
	icons/hicolor/48x48 \
	icons/hicolor/64x64 \
	icons/ubuntu-mono-dark/status/22 \
	icons/ubuntu-mono-light/status/22 \
	icons/ubuntu-mono-dark/apps/22 \
	www \
	tests

dist_doc_DATA = README COPYING NEWS AUTHORS INSTALL

autostartdir = $(sysconfdir)/xdg/autostart
autostart_DATA = psensor.desktop

desktopdir = $(datadir)/applications
desktop_DATA = psensor.desktop

EXTRA_DIST = $(desktop_DATA) $(autostart_DATA)


