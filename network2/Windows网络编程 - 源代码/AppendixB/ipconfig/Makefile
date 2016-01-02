
! include <ntwin32.mak>

# define these if you want a debug version
#PDB = ipconfig.pdb
#CDEBUG= -Zi -Fd"$(PDB)"
#LDEBUG = -debug:full

LIBS = kernel32.lib libcmt.lib wsock32.lib iphlpapi.lib

.cpp.obj:
	cl $(CDEBUG) -W3 -MT  -c $<

all:ipconfig.exe 

ipconfig.exe:ipconfig.obj
	link -nodefaultlib ipconfig.obj $(LIBS) $(LDEBUG) \
#
# uncomment the line below for a debug build
#	-pdb:$(PDB)  \
	-out:ipconfig.exe

clean:
	-del *.obj

cleanall:clean
	-del *.exe
