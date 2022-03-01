LIBNAME="gtk+-3.0"

if pkg-config --exists $LIBNAME; then
	CFLAGS="$CFLAGS $(pkg-config --cflags $LIBNAME)"
	LIBS="$LIBS $(pkg-config --libs $LIBNAME)"
else
	echo "GTK not found"
	exit 1
fi

