
! include <ntwin32.mak>

# define these if you want a debug version
#PDB = iprenew.pdb
#CDEBUG= -Zi -Fd"$(PDB)"
#LDEBUG = -debug:full

LIBS = kernel32.lib libcmt.lib wsock32.lib iphlpapi.lib

.cpp.obj:
	cl $(CDEBUG) -W3 -MT  -c $<

all:iprenew.exe 

iprenew.exe:iprenew.obj
	link -nodefaultlib iprenew.obj $(LIBS) $(LDEBUG) \
#
# uncomment the line below for a debug build
#	-pdb:$(PDB)  \
	-out:iprenew.exe

clean:
	-del *.obj

cleanall:clean
	-del *.exe
