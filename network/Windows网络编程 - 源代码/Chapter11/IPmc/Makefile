# Nmake macros for building Windows 32-Bit apps
TARGETOS=BOTH
#APPVER=5.0

!include <win32.mak>

objs_ws1=mcastws1.obj

objs_ws2=mcastws2.obj

all: mcastws1.exe mcastws2.exe

# Update the resource if necessary

# Update the object file if necessary

.c.obj: 
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $*.c

# Update the executable file if necessary, and if so, add the resource back in.

mcastws1.exe: $(objs_ws1) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:mcastws1.exe $(objs_ws1) libcmt.lib oldnames.lib kernel32.lib wsock32.lib

mcastws2.exe: $(objs_ws2) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:mcastws2.exe $(objs_ws2) $(conlibsmt) ws2_32.lib


clean:
	del *.obj
	del *.exe
