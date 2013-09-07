PROJ = HouaissParaBabylon

OBJS = $(INTDIR)\Common.obj                     \
       $(INTDIR)\Controller.obj                 \
       $(INTDIR)\Conversor.obj                  \
       $(INTDIR)\Decryptor.obj                  \
       $(INTDIR)\HouaissParaBabylon.obj         \
       $(INTDIR)\Shell.obj

RES = $(OUTDIR)\$(PROJ).res

TARGET = $(OUTDIR)\$(PROJ).exe $(RES)

!include "settings.mak"
