noinst_LTLIBRARIES	= libevent.la
libevent_la_SOURCES 	= buffer.c devpoll.c epoll.c epoll_sub.c evbuffer.c event.c kqueue.c poll.c rtsig.c select.c signal.c log.c
include_HEADERS 	= event.h
INCLUDES 		= -I$(top_srcdir)/lib/event/compat -I$(top_srcdir)/lib/event


noinst_HEADERS		= event-internal.h evsignal.h log.h compat/sys/_time.h \
			compat/sys/queue.h compat/sys/tree.h
EXTRA_DIST = err.c \
	WIN32-Code/config.h WIN32-Code/misc.c \
	WIN32-Code/win32.c WIN32-Code/misc.h \
	WIN32-Prj/libevent.dsp \
	WIN32-Prj/libevent.dsw 


include $(top_srcdir)/autotools/rules.mk


