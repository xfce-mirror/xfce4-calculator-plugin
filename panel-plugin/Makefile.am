AM_CPPFLAGS = 											\
	-I$(top_srcdir)										\
	-DG_LOG_DOMAIN=\"xfce4-calculator-plugin\"			\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

BACKEND_SRC = 								\
	eval.c								\
	eval.h								\
	lexer.c								\
	lexer.h								\
	parser.c							\
	parser.h							\
	parsetree.c							\
	parsetree.h							\
	constants.h

plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libcalculator.la

check_PROGRAMS = calctest

libcalculator_la_SOURCES =					\
	calculator.c							\
	$(BACKEND_SRC)

libcalculator_la_CFLAGS =					\
	$(LIBXFCE4UTIL_CFLAGS)					\
	$(LIBXFCE4UI_CFLAGS)					\
	$(LIBXFCE4PANEL_CFLAGS)					\
	$(PLATFORM_CFLAGS)

libcalculator_la_LDFLAGS =					\
	-avoid-version 							\
	-module 								\
	-no-undefined							\
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libcalculator_la_LIBADD =					\
	$(LIBXFCE4UTIL_LIBS)					\
	$(LIBXFCE4UI_LIBS)						\
	$(LIBXFCE4PANEL_LIBS)

calctest_SOURCES =							\
	calctest.c								\
	$(BACKEND_SRC)

calctest_CFLAGS = $(libcalculator_la_CFLAGS)
calctest_LDADD = $(libcalculator_la_LIBADD)

desktopdir = $(datadir)/xfce4/panel/plugins
desktop_in_files = calculator.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

%.desktop: %.desktop.in
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST =								\
	$(desktop_in_files)						\
	grammar.txt

CLEANFILES = $(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
