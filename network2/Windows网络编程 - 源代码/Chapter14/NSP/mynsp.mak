# Microsoft Developer Studio Generated NMAKE File, Based on mynsp.dsp
!IF "$(CFG)" == ""
CFG=mynsp - Win32 Debug
!MESSAGE No configuration specified. Defaulting to mynsp - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "mynsp - Win32 Release" && "$(CFG)" != "mynsp - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mynsp.mak" CFG="mynsp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mynsp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mynsp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "mynsp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\mynsp.dll"


CLEAN :
	-@erase "$(INTDIR)\mynsp.obj"
	-@erase "$(INTDIR)\nspsvc.obj"
	-@erase "$(INTDIR)\printobj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mynsp.dll"
	-@erase "$(OUTDIR)\mynsp.exp"
	-@erase "$(OUTDIR)\mynsp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MYNSP_EXPORTS" /Fp"$(INTDIR)\mynsp.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mynsp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\mynsp.pdb" /machine:I386 /def:".\mynsp.def" /out:"$(OUTDIR)\mynsp.dll" /implib:"$(OUTDIR)\mynsp.lib" 
DEF_FILE= \
	".\mynsp.def"
LINK32_OBJS= \
	"$(INTDIR)\mynsp.obj" \
	"$(INTDIR)\nspsvc.obj" \
	"$(INTDIR)\printobj.obj"

"$(OUTDIR)\mynsp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mynsp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\mynsp.dll"


CLEAN :
	-@erase "$(INTDIR)\mynsp.obj"
	-@erase "$(INTDIR)\nspsvc.obj"
	-@erase "$(INTDIR)\printobj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\mynsp.dll"
	-@erase "$(OUTDIR)\mynsp.exp"
	-@erase "$(OUTDIR)\mynsp.ilk"
	-@erase "$(OUTDIR)\mynsp.lib"
	-@erase "$(OUTDIR)\mynsp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MYNSP_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mynsp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib ole32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\mynsp.pdb" /debug /machine:I386 /def:".\mynsp.def" /out:"$(OUTDIR)\mynsp.dll" /implib:"$(OUTDIR)\mynsp.lib" /pdbtype:sept 
DEF_FILE= \
	".\mynsp.def"
LINK32_OBJS= \
	"$(INTDIR)\mynsp.obj" \
	"$(INTDIR)\nspsvc.obj" \
	"$(INTDIR)\printobj.obj"

"$(OUTDIR)\mynsp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\mynsp.dll"
   copy Debug\*.dll %SystemRoot%\System32
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("mynsp.dep")
!INCLUDE "mynsp.dep"
!ELSE 
!MESSAGE Warning: cannot find "mynsp.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "mynsp - Win32 Release" || "$(CFG)" == "mynsp - Win32 Debug"
SOURCE=.\mynsp.cpp

"$(INTDIR)\mynsp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mynsp.pch"


SOURCE=.\nspsvc.cpp

"$(INTDIR)\nspsvc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mynsp.pch"


SOURCE=.\printobj.cpp

"$(INTDIR)\printobj.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mynsp.pch"



!ENDIF 

