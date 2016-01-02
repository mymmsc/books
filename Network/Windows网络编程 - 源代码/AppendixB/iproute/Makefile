
! include <ntwin32.mak>

# define these if you want a debug version
#PDB = IpRoute.pdb
#CDEBUG= -Zi -Fd"$(PDB)"
#LDEBUG = -debug:full

LIBS = kernel32.lib msvcrt.lib wsock32.lib iphlpapi.lib

.cpp.obj:
	cl $(CDEBUG) -W3 -MD  -c $<

all:IpRoute.exe 

IpRoute.exe:IpRoute.obj
	link -nodefaultlib IpRoute.obj $(LIBS) $(LDEBUG) \
#
# uncomment the line below for a debug build
#	-pdb:$(PDB)  \
	-out:IpRoute.exe

clean:
	-del *.obj

cleanall:clean
	-del *.exe
