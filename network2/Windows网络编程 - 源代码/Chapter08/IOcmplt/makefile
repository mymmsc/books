# Nmake macros for building Windows 32-Bit apps
TARGETOS=BOTH
#APPVER=5.0

!include <win32.mak>


objs=iocmplt.obj

all: iocmplt.exe

# Update the resource if necessary

# Update the object file if necessary

.cpp.obj: 
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $*.cpp

# Update the executable file if necessary, and if so, add the resource back in.

iocmplt.exe: $(objs) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:iocmplt.exe $(objs) $(conlibsmt) ws2_32.lib

clean:
	del *.obj
	del *.exe
