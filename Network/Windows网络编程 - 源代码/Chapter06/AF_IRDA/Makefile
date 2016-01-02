# Nmake macros for building Windows 32-Bit apps
TARGETOS=BOTH
#APPVER=5.0

!include <win32.mak>

objs_irenum=irenum.obj

cflags = $(cflags) /D"_WIN32_WINNT"

all: irenum.exe

# Update the resource if necessary

# Update the object file if necessary

.c.obj: 
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $*.c

# Update the executable file if necessary, and if so, add the resource back in.

irenum.exe: $(objs_irenum) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:irenum.exe $(objs_irenum) $(conlibsmt) ws2_32.lib

clean:
	del *.obj
	del *.exe
