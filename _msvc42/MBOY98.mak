# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=MBOY98 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to MBOY98 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "MBOY98 - Win32 Release" && "$(CFG)" != "MBOY98 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "MBOY98.mak" CFG="MBOY98 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MBOY98 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MBOY98 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "MBOY98 - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MB_REL"
# PROP BASE Intermediate_Dir "MB_REL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MB_REL"
# PROP Intermediate_Dir "MB_REL"
# PROP Target_Dir ""
OUTDIR=.\MB_REL
INTDIR=.\MB_REL

ALL : "$(OUTDIR)\MBOY98.exe" "$(OUTDIR)\MBOY98.bsc"

CLEAN : 
	-@erase "$(INTDIR)\a_dummy.obj"
	-@erase "$(INTDIR)\a_dummy.sbr"
	-@erase "$(INTDIR)\a_sdl2.obj"
	-@erase "$(INTDIR)\a_sdl2.sbr"
	-@erase "$(INTDIR)\a_wvout.obj"
	-@erase "$(INTDIR)\a_wvout.sbr"
	-@erase "$(INTDIR)\audio.obj"
	-@erase "$(INTDIR)\audio.sbr"
	-@erase "$(INTDIR)\backends.obj"
	-@erase "$(INTDIR)\backends.sbr"
	-@erase "$(INTDIR)\cpu.obj"
	-@erase "$(INTDIR)\cpu.sbr"
	-@erase "$(INTDIR)\cpu_ops.obj"
	-@erase "$(INTDIR)\cpu_ops.sbr"
	-@erase "$(INTDIR)\i_sdl2.obj"
	-@erase "$(INTDIR)\i_sdl2.sbr"
	-@erase "$(INTDIR)\i_win32.obj"
	-@erase "$(INTDIR)\i_win32.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\main_imp.obj"
	-@erase "$(INTDIR)\main_imp.sbr"
	-@erase "$(INTDIR)\mem.obj"
	-@erase "$(INTDIR)\mem.sbr"
	-@erase "$(INTDIR)\ringbuf.obj"
	-@erase "$(INTDIR)\ringbuf.sbr"
	-@erase "$(INTDIR)\sys.obj"
	-@erase "$(INTDIR)\sys.sbr"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\trace.sbr"
	-@erase "$(INTDIR)\v_gdi.obj"
	-@erase "$(INTDIR)\v_gdi.sbr"
	-@erase "$(INTDIR)\v_sdl2.obj"
	-@erase "$(INTDIR)\v_sdl2.sbr"
	-@erase "$(INTDIR)\video.obj"
	-@erase "$(INTDIR)\video.sbr"
	-@erase "$(OUTDIR)\MBOY98.bsc"
	-@erase "$(OUTDIR)\MBOY98.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /w /W0 /GX /O2 /Ob2 /I "./include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "VIDEO_GDI" /D "AUDIO_WAVEOUT" /D "INPUT_WIN32" /FR /YX /c
CPP_PROJ=/nologo /G5 /ML /w /W0 /GX /O2 /Ob2 /I "./include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "VIDEO_GDI" /D "AUDIO_WAVEOUT" /D "INPUT_WIN32"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/MBOY98.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\MB_REL/
CPP_SBRS=.\MB_REL/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /D "NDEBUG" /win32
# SUBTRACT MTL /nologo
MTL_PROJ=/D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/MBOY98.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\a_dummy.sbr" \
	"$(INTDIR)\a_sdl2.sbr" \
	"$(INTDIR)\a_wvout.sbr" \
	"$(INTDIR)\audio.sbr" \
	"$(INTDIR)\backends.sbr" \
	"$(INTDIR)\cpu.sbr" \
	"$(INTDIR)\cpu_ops.sbr" \
	"$(INTDIR)\i_sdl2.sbr" \
	"$(INTDIR)\i_win32.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\main_imp.sbr" \
	"$(INTDIR)\mem.sbr" \
	"$(INTDIR)\ringbuf.sbr" \
	"$(INTDIR)\sys.sbr" \
	"$(INTDIR)\trace.sbr" \
	"$(INTDIR)\v_gdi.sbr" \
	"$(INTDIR)\v_sdl2.sbr" \
	"$(INTDIR)\video.sbr"

"$(OUTDIR)\MBOY98.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/MBOY98.pdb" /machine:I386 /out:"$(OUTDIR)/MBOY98.exe" 
LINK32_OBJS= \
	"$(INTDIR)\a_dummy.obj" \
	"$(INTDIR)\a_sdl2.obj" \
	"$(INTDIR)\a_wvout.obj" \
	"$(INTDIR)\audio.obj" \
	"$(INTDIR)\backends.obj" \
	"$(INTDIR)\cpu.obj" \
	"$(INTDIR)\cpu_ops.obj" \
	"$(INTDIR)\i_sdl2.obj" \
	"$(INTDIR)\i_win32.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\main_imp.obj" \
	"$(INTDIR)\mem.obj" \
	"$(INTDIR)\ringbuf.obj" \
	"$(INTDIR)\sys.obj" \
	"$(INTDIR)\trace.obj" \
	"$(INTDIR)\v_gdi.obj" \
	"$(INTDIR)\v_sdl2.obj" \
	"$(INTDIR)\video.obj"

"$(OUTDIR)\MBOY98.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MB_DEBUG"
# PROP BASE Intermediate_Dir "MB_DEBUG"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MB_DEBUG"
# PROP Intermediate_Dir "MB_DEBUG"
# PROP Target_Dir ""
OUTDIR=.\MB_DEBUG
INTDIR=.\MB_DEBUG

ALL : "$(OUTDIR)\MBOY98.exe"

CLEAN : 
	-@erase "$(INTDIR)\a_dummy.obj"
	-@erase "$(INTDIR)\a_sdl2.obj"
	-@erase "$(INTDIR)\a_wvout.obj"
	-@erase "$(INTDIR)\audio.obj"
	-@erase "$(INTDIR)\backends.obj"
	-@erase "$(INTDIR)\cpu.obj"
	-@erase "$(INTDIR)\cpu_ops.obj"
	-@erase "$(INTDIR)\i_sdl2.obj"
	-@erase "$(INTDIR)\i_win32.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main_imp.obj"
	-@erase "$(INTDIR)\mem.obj"
	-@erase "$(INTDIR)\ringbuf.obj"
	-@erase "$(INTDIR)\sys.obj"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\v_gdi.obj"
	-@erase "$(INTDIR)\v_sdl2.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\video.obj"
	-@erase "$(OUTDIR)\MBOY98.exe"
	-@erase "$(OUTDIR)\MBOY98.ilk"
	-@erase "$(OUTDIR)\MBOY98.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /w /W0 /Gm /GX /Zi /O2 /Ob2 /I "./include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "VIDEO_GDI" /D "AUDIO_WAVEOUT" /D "INPUT_WIN32" /YX /c
CPP_PROJ=/nologo /MLd /w /W0 /Gm /GX /Zi /O2 /Ob2 /I "./include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "VIDEO_GDI" /D "AUDIO_WAVEOUT" /D "INPUT_WIN32"\
 /Fp"$(INTDIR)/MBOY98.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\MB_DEBUG/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/MBOY98.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib winmm.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/MBOY98.pdb" /debug /machine:I386 /out:"$(OUTDIR)/MBOY98.exe" 
LINK32_OBJS= \
	"$(INTDIR)\a_dummy.obj" \
	"$(INTDIR)\a_sdl2.obj" \
	"$(INTDIR)\a_wvout.obj" \
	"$(INTDIR)\audio.obj" \
	"$(INTDIR)\backends.obj" \
	"$(INTDIR)\cpu.obj" \
	"$(INTDIR)\cpu_ops.obj" \
	"$(INTDIR)\i_sdl2.obj" \
	"$(INTDIR)\i_win32.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\main_imp.obj" \
	"$(INTDIR)\mem.obj" \
	"$(INTDIR)\ringbuf.obj" \
	"$(INTDIR)\sys.obj" \
	"$(INTDIR)\trace.obj" \
	"$(INTDIR)\v_gdi.obj" \
	"$(INTDIR)\v_sdl2.obj" \
	"$(INTDIR)\video.obj"

"$(OUTDIR)\MBOY98.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "MBOY98 - Win32 Release"
# Name "MBOY98 - Win32 Debug"

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\video.c
DEP_CPP_VIDEO=\
	".\./include\audio.h"\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\cpu.h"\
	".\./include\mem.h"\
	".\./include\ringbuf.h"\
	".\./include\sys.h"\
	".\./include\trace.h"\
	".\./include\video.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\video.obj" : $(SOURCE) $(DEP_CPP_VIDEO) "$(INTDIR)"

"$(INTDIR)\video.sbr" : $(SOURCE) $(DEP_CPP_VIDEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\video.obj" : $(SOURCE) $(DEP_CPP_VIDEO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\backends.c
DEP_CPP_BACKE=\
	".\./include\backends.h"\
	".\./include\bendlist.h"\
	".\./include\compat.h"\
	".\./include\trace.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\backends.obj" : $(SOURCE) $(DEP_CPP_BACKE) "$(INTDIR)"

"$(INTDIR)\backends.sbr" : $(SOURCE) $(DEP_CPP_BACKE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\backends.obj" : $(SOURCE) $(DEP_CPP_BACKE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cpu.c
DEP_CPP_CPU_C=\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\cpu.h"\
	".\./include\cpu_alu.h"\
	".\./include\cpu_help.h"\
	".\./include\mem.h"\
	".\./include\sys.h"\
	".\./include\trace.h"\
	".\./include\video.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\cpu.obj" : $(SOURCE) $(DEP_CPP_CPU_C) "$(INTDIR)"

"$(INTDIR)\cpu.sbr" : $(SOURCE) $(DEP_CPP_CPU_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\cpu.obj" : $(SOURCE) $(DEP_CPP_CPU_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cpu_ops.c
DEP_CPP_CPU_O=\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\cpu.h"\
	".\./include\cpu_alu.h"\
	".\./include\cpu_help.h"\
	".\./include\mem.h"\
	".\./include\sys.h"\
	".\./include\trace.h"\
	".\./include\video.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\cpu_ops.obj" : $(SOURCE) $(DEP_CPP_CPU_O) "$(INTDIR)"

"$(INTDIR)\cpu_ops.sbr" : $(SOURCE) $(DEP_CPP_CPU_O) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\cpu_ops.obj" : $(SOURCE) $(DEP_CPP_CPU_O) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\main.c
DEP_CPP_MAIN_=\
	".\./include\compat.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"

"$(INTDIR)\main.sbr" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\main_imp.c
DEP_CPP_MAIN_I=\
	".\./include\audio.h"\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\cpu.h"\
	".\./include\mem.h"\
	".\./include\ringbuf.h"\
	".\./include\sys.h"\
	".\./include\trace.h"\
	".\./include\video.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\main_imp.obj" : $(SOURCE) $(DEP_CPP_MAIN_I) "$(INTDIR)"

"$(INTDIR)\main_imp.sbr" : $(SOURCE) $(DEP_CPP_MAIN_I) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\main_imp.obj" : $(SOURCE) $(DEP_CPP_MAIN_I) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mem.c
DEP_CPP_MEM_C=\
	".\./include\audio.h"\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\cpu.h"\
	".\./include\mem.h"\
	".\./include\ringbuf.h"\
	".\./include\sys.h"\
	".\./include\trace.h"\
	".\./include\video.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\mem.obj" : $(SOURCE) $(DEP_CPP_MEM_C) "$(INTDIR)"

"$(INTDIR)\mem.sbr" : $(SOURCE) $(DEP_CPP_MEM_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\mem.obj" : $(SOURCE) $(DEP_CPP_MEM_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ringbuf.c
DEP_CPP_RINGB=\
	".\./include\compat.h"\
	".\./include\ringbuf.h"\
	".\./include\trace.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\ringbuf.obj" : $(SOURCE) $(DEP_CPP_RINGB) "$(INTDIR)"

"$(INTDIR)\ringbuf.sbr" : $(SOURCE) $(DEP_CPP_RINGB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\ringbuf.obj" : $(SOURCE) $(DEP_CPP_RINGB) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sys.c
DEP_CPP_SYS_C=\
	".\./include\audio.h"\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\cpu.h"\
	".\./include\mem.h"\
	".\./include\ringbuf.h"\
	".\./include\sys.h"\
	".\./include\trace.h"\
	".\./include\video.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\sys.obj" : $(SOURCE) $(DEP_CPP_SYS_C) "$(INTDIR)"

"$(INTDIR)\sys.sbr" : $(SOURCE) $(DEP_CPP_SYS_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\sys.obj" : $(SOURCE) $(DEP_CPP_SYS_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\trace.c
DEP_CPP_TRACE=\
	".\./include\compat.h"\
	".\./include\trace.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\trace.obj" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"

"$(INTDIR)\trace.sbr" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\trace.obj" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\audio.c
DEP_CPP_AUDIO=\
	".\./include\audio.h"\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\mem.h"\
	".\./include\ringbuf.h"\
	".\./include\sys.h"\
	".\./include\trace.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


"$(INTDIR)\audio.obj" : $(SOURCE) $(DEP_CPP_AUDIO) "$(INTDIR)"

"$(INTDIR)\audio.sbr" : $(SOURCE) $(DEP_CPP_AUDIO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\audio.obj" : $(SOURCE) $(DEP_CPP_AUDIO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\video.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\backends.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\bendlist.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\compat.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\cpu.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\cpu_alu.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\cpu_help.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\mem.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\ringbuf.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\sys.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\trace.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\include\audio.h

!IF  "$(CFG)" == "MBOY98 - Win32 Release"

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\audio\a_dummy.c
DEP_CPP_A_DUM=\
	".\./include\backends.h"\
	".\./include\compat.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\a_dummy.obj" : $(SOURCE) $(DEP_CPP_A_DUM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\a_dummy.sbr" : $(SOURCE) $(DEP_CPP_A_DUM) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\a_dummy.obj" : $(SOURCE) $(DEP_CPP_A_DUM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\audio\a_sdl2.c
DEP_CPP_A_SDL=\
	".\./include\audio.h"\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\mem.h"\
	".\./include\ringbuf.h"\
	".\./include\trace.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\a_sdl2.obj" : $(SOURCE) $(DEP_CPP_A_SDL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\a_sdl2.sbr" : $(SOURCE) $(DEP_CPP_A_SDL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\a_sdl2.obj" : $(SOURCE) $(DEP_CPP_A_SDL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\audio\a_wvout.c
DEP_CPP_A_WVO=\
	".\./include\audio.h"\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\mem.h"\
	".\./include\ringbuf.h"\
	".\./include\trace.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\a_wvout.obj" : $(SOURCE) $(DEP_CPP_A_WVO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\a_wvout.sbr" : $(SOURCE) $(DEP_CPP_A_WVO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\a_wvout.obj" : $(SOURCE) $(DEP_CPP_A_WVO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\input\i_sdl2.c
DEP_CPP_I_SDL=\
	".\./include\backends.h"\
	".\./include\compat.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\i_sdl2.obj" : $(SOURCE) $(DEP_CPP_I_SDL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\i_sdl2.sbr" : $(SOURCE) $(DEP_CPP_I_SDL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\i_sdl2.obj" : $(SOURCE) $(DEP_CPP_I_SDL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\input\i_win32.c
DEP_CPP_I_WIN=\
	".\./include\backends.h"\
	".\./include\compat.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\i_win32.obj" : $(SOURCE) $(DEP_CPP_I_WIN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\i_win32.sbr" : $(SOURCE) $(DEP_CPP_I_WIN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\i_win32.obj" : $(SOURCE) $(DEP_CPP_I_WIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\video\v_gdi.c
DEP_CPP_V_GDI=\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\trace.h"\
	".\./include\video.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\v_gdi.obj" : $(SOURCE) $(DEP_CPP_V_GDI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\v_gdi.sbr" : $(SOURCE) $(DEP_CPP_V_GDI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\v_gdi.obj" : $(SOURCE) $(DEP_CPP_V_GDI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\video\v_sdl2.c
DEP_CPP_V_SDL=\
	".\./include\backends.h"\
	".\./include\compat.h"\
	".\./include\video.h"\
	

!IF  "$(CFG)" == "MBOY98 - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\v_sdl2.obj" : $(SOURCE) $(DEP_CPP_V_SDL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\v_sdl2.sbr" : $(SOURCE) $(DEP_CPP_V_SDL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "MBOY98 - Win32 Debug"


"$(INTDIR)\v_sdl2.obj" : $(SOURCE) $(DEP_CPP_V_SDL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
