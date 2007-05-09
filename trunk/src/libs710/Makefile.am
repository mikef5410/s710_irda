lib_LTLIBRARIES = libs710.la

libs710_la_SOURCES = add_attr.c \
	alpha_map.c \
	axes.c \
	bike.c \
	byte_map.c \
	comm.c \
	crc.c \
	driver.c \
	exercise.c \
	files.c \
	filter.c \
	free_attr.c \
	glob.c \
	histogram.c \
	logo.c \
	merge_attr.c \
	overview.c \
	packet.c \
	parse_attr.c \
	plot.c \
	print_attr.c \
	reminder.c \
	serial.c \
	time.c \
	transfer.c \
	usb.c \
	user.c \
	watch.c \
	workout_util.c \
	workout_merge.c \
	workout_print.c \
	workout_read.c \
	workout_write.c

# Updating version info:
#
# version is c:r:a (current:revision:age)
#
# Any change at all => c:r+1:a
# Interfaces changed/added/removed => c+1:0:a
# Interfaces added => c+1:0:a+1
# Interfaces removed => c+1:0:0

libs710_la_LDFLAGS = \
	-version-info 9:1:0

INCLUDES = -I$(top_srcdir)/include
CFLAGS   = @CFLAGS@ $(USB_CFLAGS) -Wall
#LDFLAGS  = $(USB_LIBS)
