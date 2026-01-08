# SGD - Copyright 1995-1999 ThunderWare Research Center

PROJ   = sgd
LINK_S = dos4g

.ERASE

C_OPT   = -oo -mf -fp3 -fpi -3r -oxnt -zp1 -j -ei -wx -we /Fo=$^@  -fh=$^&.pch
CPP_OPT = -zq /Fo=$^@
#ASM_OPT = /nologo /c /Fo $^@
ASM_OPT = /c $^@

LNK_OPT = op{q} system $(LINK_S) option stack=48k
LNK     = $(PROJ).lnk

OBJS    = dbconfig.obj dbscreen.obj dbfile.obj   dbdbase.obj  dbselect.obj &
          dbcompr.obj  dbmenu.obj   dbpoke.obj   dbtzxpok.obj dbmain.obj
LIBS    = wbench32\wbench32.lib

all : $(PROJ).exe

dbconfig.obj : dbconfig.c dbconfig.h wbench32\wbench32.h dbmain.h dbdbase.h
dbscreen.obj : dbscreen.c dbscreen.h wbench32\wbench32.h dbmain.h dbconfig.h dbdbase.h
dbfile.obj   : dbfile.c dbfile.h wbench32\wbench32.h dbmain.h dbconfig.h dbdbase.h
dbdbase.obj  : dbdbase.c dbdbase.h wbench32\wbench32.h dbmain.h dbconfig.h
dbselect.obj : dbselect.c dbselect.h wbench32\wbench32.h dbmain.h dbconfig.h dbdbase.h
dbcompr.obj  : dbcompr.c dbcompr.h wbench32\wbench32.h dbmain.h dbconfig.h dbdbase.h
dbmenu.obj   : dbmenu.c dbmenu.h wbench32\wbench32.h dbmain.h dbconfig.h dbdbase.h
dbpoke.obj   : dbpoke.c dbpoke.h wbench32\wbench32.h dbmain.h dbconfig.h dbdbase.h
dbtzxpok.obj : dbtzxpok.c dbtzxpok.h wbench32\wbench32.h dbmain.h dbconfig.h
dbmain.obj   : dbmain.c dbmain.h wbench32\wbench32.h dbconfig.h dbdbase.h

$(LNK) : makefile
	@echo -- Create Linker Project File
	@echo ----------------------------------
	@%create $(LNK)
	@%append $(LNK) name $(PROJ)
	@for %i in ($(OBJS)) do @%append $(LNK) file %i
	@for %i in ($(LIBS)) do @%append $(LNK) file %i

.c.obj :
	wcc386 $(C_OPT) $[@

.cpp.obj :
	wpp386 $(CPP_OPT) $[@

.asm.obj :
	d:\program\masm\masm $(ASM_OPT) $[@

$(PROJ).exe : $(OBJS) $(LNK) $(LIBS) makefile
	@echo -- Linking SGD Executable
	@echo -----------------------------------
	@rem > result.lst
	wlink $(LNK_OPT) @$^* > result.lst

# End Of MakeFile..
