
! include <ntwin32.mak>

# define these if you want a debug version
#PDB = IpArp.pdb
#CDEBUG= -Zi -Fd"$(PDB)"
#LDEBUG = -debug:full

LIBS = kernel32.lib msvcrt.lib wsock32.lib iphlpapi.lib

.cpp.obj:
	cl $(CDEBUG) -W3 -MD  -c $<

all:IpArp.exe 

IpArp.exe:IpArp.obj
	link -nodefaultlib IpArp.obj $(LIBS) $(LDEBUG) \
#
# uncomment the line below for a debug build
#	-pdb:$(PDB)  \
	-out:IpArp.exe

clean:
	-del *.obj

cleanall:clean
	-del *.exe
