# Nmake macros for building Windows 32-Bit apps
TARGETOS=BOTH
#APPVER=5.0

!include <win32.mak>


objs=asyncselect.obj

all: asyncselect.exe

# Update the resource if necessary

# Update the object file if necessary

.cpp.obj: 
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $*.cpp

# Update the executable file if necessary, and if so, add the resource back in.

asyncselect.exe: $(objs) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:asyncselect.exe $(objs) $(conlibsmt) ws2_32.lib user32.lib gdi32.lib

clean:
        del *.obj
        del *.exe
