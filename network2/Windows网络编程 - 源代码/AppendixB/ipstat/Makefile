
! include <ntwin32.mak>

# define these if you want a debug version
#PDB = IpStat.pdb
#CDEBUG= -Zi -Fd"$(PDB)"
#LDEBUG = -debug:full

LIBS = kernel32.lib msvcrt.lib  wsock32.lib iphlpapi.lib

.cpp.obj:
	cl $(CDEBUG) -W3 -MD  -c $<

all:IpStat.exe 

IpStat.exe:IpStat.obj
	link -nodefaultlib IpStat.obj $(LIBS) $(LDEBUG) \
#
# uncomment the line below for a debug build
#	-pdb:$(PDB)  \
	-out:IpStat.exe

clean:
	-del *.obj

cleanall:clean
	-del *.exe
