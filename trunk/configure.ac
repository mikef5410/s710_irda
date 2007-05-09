# Process this file with autoconf to produce a configure script.

AC_INIT(s710,0.21,dave@daveb.net)
AC_CONFIG_SRCDIR(src/libs710/serial.c)
AM_INIT_AUTOMAKE(1.6)
AM_CONFIG_HEADER(include/config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl check presence of png library
AC_ARG_WITH(png,dnl
[  --with-png[=DIR]        where to find the png library],,
  with_png=yes)

if test "$with_png" != no; then
  AC_PATH_PROG([LIBPNG_CONFIG], [libpng-config])
  if test -n "$LIBPNG_CONFIG"; then
    # should be --ldopts, but it's currently broken
    libpng_LDFLAGS=`libpng-config --ldflags | sed 's/ -l[[^ ]][[^ ]]*//g'`
  elif test -d "$with_png"; then
    libpng_LDFLAGS="-L$with_png/lib"
  fi

  _ldflags="$LDFLAGS"
  LDFLAGS="$LDFLAGS $libpng_LDFLAGS"

  AC_CHECK_LIB(png,png_create_write_struct,
    AC_DEFINE(HAVE_LIBPNG,1,[ Define if you have the png library. ]),,[-lz -lm])

  if test "$ac_cv_lib_png_png_create_write_struct" = yes ; then
    AC_EGREP_CPP([You lose],[#include "png.h"
#if PNG_LIBPNG_VER < 89
You lose.
#endif
      ],[AC_MSG_ERROR([libpng too old])])
  else
    AC_MSG_ERROR([libpng is required])
  fi

  dnl we'll use the png-specific flags later
  LDFLAGS="$_ldflags"
fi
dnl end png

dnl check presence of gd library
dnl we don't check for libfreetype and libjpeg locations - if gd requires
dnl them, the gdlib-config scipt contains all the required information
AC_ARG_WITH(gd,dnl
[  --with-gd[=DIR]         where to find Tom Boutell's gd library],,
  with_gd=yes)

if test "$with_gd" != no; then
  AC_PATH_PROG([GDLIB_CONFIG], [gdlib-config])
  if test -n "$GDLIB_CONFIG"; then
    libgd_CPPFLAGS=`gdlib-config --cflags`
    libgd_LDFLAGS=`gdlib-config --ldflags`
  elif test -d "$with_gd"; then
    libgd_CPPFLAGS="-I$with_gd/include"
    libgd_LDFLAGS="-L$with_gd/lib"
  else
    AC_CHECK_HEADER(gd.h,,[AC_MSG_ERROR([gd.h is required])])
  fi

  _cppflags="$CPPFLAGS"
  _ldflags="$LDFLAGS"
  CPPFLAGS="$CPPFLAGS $libgd_CPPFLAGS"
  LDFLAGS="$LDFLAGS $libgd_LDFLAGS"

  AC_CHECK_LIB(gd,gdImageCreate,
    [dnl found gd library
     AC_DEFINE(HAVE_LIBGD,1,[ Define if you have gd library. ])
     AC_CHECK_HEADERS(gd.h)

     LIBS="$_libs $libgd_LIBS -lpng"
     dnl png support in libgd
     AC_CHECK_LIB(gd,gdImagePng,
       [AC_DEFINE(HAVE_GD_PNG,1,[ Define if libgd supports png. ])
        libgd_LIBS="$libgd_LIBS -lpng -lz -lm"
        have_gd_png=yes],
       [AC_MSG_ERROR([libgd does not support png (required)])])
     LIBS="$_libs"

    ],[dnl gd library not found
     AC_MSG_ERROR([libgd not found])
    ])

  if test -n "$GDLIB_CONFIG"; then
    libgd_LIBS=`gdlib-config --libs`
  fi

  dnl piece it all together
  if test "$with_gd" = no; then
    CPPFLAGS="$_cppflags"
    LDFLAGS="$_ldflags"
  else
    GRAPHLIBS="$GRAPHLIBS -lgd $libgd_LIBS"
  fi

fi
dnl end gd

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memset strerror])

# Checks for libusb (version 0.1.8 or greater needed!)

AH_TEMPLATE([HAVE_LIBUSB],
	[Define if USB support is enabled via libusb])

usb_msg="disabled (http://libusb.sf.net)"
try_usb=false
AC_ARG_WITH(usb, 
	AC_HELP_STRING([--with-usb],[Compile in USB support]),
	if test x$withval = xyes; then
		try_usb=true
	fi
)
if $try_usb; then
	AC_PATH_PROG(LIBUSB_CONFIG,libusb-config)
	if test -n "${LIBUSB_CONFIG}"; then
		CPPFLAGS_save="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS `libusb-config --cflags`"
		AC_CHECK_HEADER(usb.h,
			USB_LIBS="`libusb-config --libs`"
			USB_CFLAGS="`libusb-config --cflags`"
                        LIBUSB_VER="`libusb-config --version`"
			AC_DEFINE([HAVE_LIBUSB]))
			usb_msg="enabled ($LIBUSB_VER)"
		CPPFLAGS="$CPPFLAGS_save"
	fi
fi

PROG_LIBS="$USB_LIBS $GRAPHLIBS"

AC_SUBST(USB_CFLAGS)
AC_SUBST(USB_LIBS)
AC_SUBST(PROG_LIBS)

# max valid HR and cadence can be set at config time

max_hr=206
max_cad=170
hrm_max_hr=200
hrm_rest_hr=60
filedir=/var/polar/s710/raw

AH_TEMPLATE([S710_MAX_VALID_HR],
	[Define if the default value is inadequate])
AH_TEMPLATE([S710_MAX_VALID_CAD],
	[Define if the default value is inadequate])
AH_TEMPLATE([S710_FILEDIR],
	[Define if the default value is inadequate])
AH_TEMPLATE([S710_HRM_MAX_HR],
	[Define if the default value is inadequate])
AH_TEMPLATE([S710_HRM_REST_HR],
	[Define if the default value is inadequate])

# USB read is either a bulk read (suits most 2.4 kernels?) or an 
# interrupt read (suits most 2.6 kernels?).  As of s710-0.13, the
# interrupt read is the default, but this configuration directive
# allows you to override that default and have libusb do bulk 
# reads.  You'll probably need to do this if you have a 2.4 kernel.

AH_TEMPLATE([S710_USB_BULK_READ],
	[Enable if you want libusb to do bulk reads (suits 2.4 kernels)])

AH_TEMPLATE([S710_SERIAL_ALT_INTER_CHAR_TIMER_IMP],
	[Enable if your platform does not honor termios VTIME/VMIN settings])

AC_ARG_WITH(max-hr,
	AC_HELP_STRING([--with-max-hr],
	[set a default maximum valid HR value for filtering]),
	max_hr=$withval
	AC_DEFINE_UNQUOTED([S710_MAX_VALID_HR],$withval),
	AC_DEFINE_UNQUOTED([S710_MAX_VALID_HR],$max_hr))

AC_ARG_WITH(max-cad,
	AC_HELP_STRING([--with-max-cad],
	[set a default maximum valid cadence value for filtering]),
	max_cad=$withval
	AC_DEFINE_UNQUOTED([S710_MAX_VALID_CAD],$withval),
	AC_DEFINE_UNQUOTED([S710_MAX_VALID_CAD],$max_cad))

AC_ARG_WITH(filedir,
	AC_HELP_STRING([--with-filedir],
	[set the directory where exercise files are saved]),
	filedir=$withval
	AC_DEFINE_UNQUOTED([S710_FILEDIR],"$withval"),
	AC_DEFINE_UNQUOTED([S710_FILEDIR],"$filedir"))

AC_ARG_WITH(hrm-max-hr,
	AC_HELP_STRING([--with-hrm-max-hr],
	[set a default MaxHR value for HRM output]),
	hrm_max_hr=$withval
	AC_DEFINE_UNQUOTED([S710_HRM_MAX_HR],$withval),
	AC_DEFINE_UNQUOTED([S710_HRM_MAX_HR],$hrm_max_hr))

AC_ARG_WITH(hrm-rest-hr,
	AC_HELP_STRING([--with-hrm-rest-hr],
	[set a default RestHR value for HRM output]),
	hrm_rest_hr=$withval
	AC_DEFINE_UNQUOTED([S710_HRM_REST_HR],$withval),
	AC_DEFINE_UNQUOTED([S710_HRM_REST_HR],$hrm_rest_hr))

AC_ARG_ENABLE(usb-bulk-read,
	AC_HELP_STRING([--enable-usb-bulk-read],
	[enable bulk reads instead of interrupt reads in libusb]),
	usb_msg="$usb_msg (using bulk read)"
	AC_DEFINE(S710_USB_BULK_READ))

AC_ARG_ENABLE(serial-alt-inter-char-timer-imp,
	AC_HELP_STRING([--enable-serial-alt-inter-char-timer-imp],
	[enable alternative inter-character timer implementation for serial port communication]),
	[serial_alt_inter_char_timer_imp=$enableval],
	[serial_alt_inter_char_timer_imp=no])

if test x$serial_alt_inter_char_timer_imp = xyes; then
	AC_DEFINE(S710_SERIAL_ALT_INTER_CHAR_TIMER_IMP)
fi

AC_CONFIG_FILES([Makefile \
	src/Makefile \
	src/libs710/Makefile \
	src/utils/Makefile \
	include/Makefile \
	doc/Makefile \
	test/Makefile])
AC_OUTPUT

echo "
Configuration (s710)
	USB support:             $usb_msg $usb_bulk_read_msg
	Max valid heart rate:    $max_hr
	Max valid cadence:       $max_cad
        HRM output MaxHR:        $hrm_max_hr
        HRM output RestHR:       $hrm_rest_hr
	Workout file directory:  $filedir
"
