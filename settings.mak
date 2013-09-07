!include <win32.mak>

lib = lib

!IF "$(NODEBUG)" == ""
INTDIR=Debug
!ELSE
INTDIR=Release
!ENDIF

OUTDIR=$(INTDIR)
OUTDIRLIB=$(INTDIR)

INCLUDE = $(INCLUDE);\WinDDK\6001.18001\inc\crt\atl71
INCLUDE = $(INCLUDE);\Tools\Libs\WTL\include

LIB = $(LIB);\WinDDK\6001.18001\lib\atl\i386

cflags = $(cflags) /DUNICODE /EHsc


all: $(OUTDIR) $(TARGET)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(INTDIR)\$(PROJ).res: $(PROJ).rc 
    $(rc) $(rcflags) $(rcvars) /fo $(INTDIR)\$(PROJ).res $(PROJ).rc

.cpp{$(INTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fp"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" $**

$(OUTDIR)\$(PROJ).exe: $(OBJS) $(RES)
	$(link) $(ldebug) $(guilibs) $(guiflags) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).exe $(OBJS) $(ELIBS) /PDB:$(OUTDIR)\$(PROJ).PDB $(RES) 

$(OUTDIRLIB)\$(PROJ).lib: $(OBJS)
	$(lib) /OUT:$(OUTDIRLIB)\$(PROJ).lib $(OBJS)

clean:
	del /q $(INTDIR)\*.*
