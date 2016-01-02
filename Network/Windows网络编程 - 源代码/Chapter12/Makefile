# Nmake macros for building Windows 32-Bit apps
TARGETOS=BOTH
APPVER=5.0

!include <win32.mak>


objs_udp=qosudp.obj provider.obj printqos.obj

objs_tcp=qostcp.obj provider.obj printqos.obj

objs_mcast=qosmcast.obj provider.obj printqos.obj

all: qosudp.exe qostcp.exe qosmcast.exe

# Update the resource if necessary

# Update the object file if necessary

.c.obj: 
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $*.c

# Update the executable file if necessary, and if so, add the resource back in.

qosudp.exe: $(objs_udp) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:qosudp.exe $(objs_udp) $(conlibsmt) ws2_32.lib

qostcp.exe: $(objs_tcp) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:qostcp.exe $(objs_tcp) $(conlibsmt) ws2_32.lib

qosmcast.exe: $(objs_mcast) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:qosmcast.exe $(objs_mcast) $(conlibsmt) ws2_32.lib

clean:
	del *.obj
	del *.exe
