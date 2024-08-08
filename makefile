# makefile for GNU make (automatically generated by makemake)


OS := $(shell uname)

ifeq ($(strip $(OS)),AmigaOS)
	AMIGADATE = $(shell c:date LFORMAT %d.%m.%Y)
#	YEAR = $(shell c:date LFORMAT %Y)
	RM = delete
else
	AMIGADATE = $(shell date +"%-d.%-m.%Y")
#	YEAR = $(shell date +"%Y")
	RM = rm -f -r
endif

# "make DEBUG=" to disable
DEBUG = -D_DEBUG_
# "make MENUCLASS=" to disable
MENUCLASS = -D_MENUCLASS_


EXE    = AmiFTP
OBJDIR = o/
CFLAGS = -Wall -Werror=implicit-function-declaration -D__USE_INLINE__ -gstabs -D__AMIGADATE__=\"$(AMIGADATE)\" $(MENUCLASS) $(DEBUG)
#LFLAGS = -lauto
LFLAGS = 
CC = ppc-amigaos-gcc
STRIP = ppc-amigaos-strip
CATCOMP = APPDIR:FlexCat

# target 'all' (default target)
all : $(EXE)

$(OBJDIR)ADT.o : ADT.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)ADT.o ADT.c

$(OBJDIR)aguide.o : aguide.c AmiFTP.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)aguide.o aguide.c

$(OBJDIR)AmiFTP.o : AmiFTP.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)AmiFTP.o AmiFTP.c

$(OBJDIR)AmiFTP_Cat.o : AmiFTP_Cat.c AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)AmiFTP_Cat.o AmiFTP_Cat.c

$(OBJDIR)cache.o : cache.c AmiFTP.h gui.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)cache.o cache.c

$(OBJDIR)change_dir.o : change_dir.c AmiFTP.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)change_dir.o change_dir.c

$(OBJDIR)config.o : config.c AmiFTP.h gui.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)config.o config.c

$(OBJDIR)ConnectWindow.o : ConnectWindow.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)ConnectWindow.o ConnectWindow.c

$(OBJDIR)data.o : data.c AmiFTP.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)data.o data.c

$(OBJDIR)dirlist.o : dirlist.c AmiFTP.h gui.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)dirlist.o dirlist.c

$(OBJDIR)dofuncs.o : dofuncs.c AmiFTP.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)dofuncs.o dofuncs.c

$(OBJDIR)ftp.o : ftp.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)ftp.o ftp.c

$(OBJDIR)locale.o : locale.c AmiFTP.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)locale.o locale.c

$(OBJDIR)main.o : main.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)main.o main.c

$(OBJDIR)MainWindow.o : MainWindow.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)MainWindow.o MainWindow.c

$(OBJDIR)Menu.o : Menu.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)Menu.o Menu.c

$(OBJDIR)misc.o : misc.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)misc.o misc.c

$(OBJDIR)PrefsWindow.o : PrefsWindow.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)PrefsWindow.o PrefsWindow.c

$(OBJDIR)readdir.o : readdir.c AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)readdir.o readdir.c

$(OBJDIR)rexx.o : rexx.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)rexx.o rexx.c

$(OBJDIR)SiteWindow.o : SiteWindow.c AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)SiteWindow.o SiteWindow.c

$(OBJDIR)SpeedBar.o : SpeedBar.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)SpeedBar.o SpeedBar.c

$(OBJDIR)TransferWindow.o : TransferWindow.c AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)TransferWindow.o TransferWindow.c

$(OBJDIR)bsdsocket.o : ../tcphook/bsdsocket.c
	$(CC) -c -o $(OBJDIR)bsdsocket.o ../tcphook/bsdsocket.c

$(OBJDIR)tcp.o : ../tcphook/tcp.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)tcp.o ../tcphook/tcp.c

#$(OBJDIR)version.o : version.c
#	$(CC) $(CFLAGS) -c -o $(OBJDIR)version.o version.c

$(OBJDIR)MenuClass.o : MenuClass.c AmiFTP.h gui.h AmiFTP_Cat.h
	$(CC) $(CFLAGS) -c -o $@ MenuClass.c


#OBJS = $(OBJDIR)version.o $(OBJDIR)bsdsocket.o $(OBJDIR)tcp.o $(OBJDIR)ADT.o $(OBJDIR)aguide.o $(OBJDIR)AmiFTP.o $(OBJDIR)AmiFTP_Cat.o $(OBJDIR)cache.o $(OBJDIR)change_dir.o $(OBJDIR)config.o $(OBJDIR)ConnectWindow.o $(OBJDIR)data.o $(OBJDIR)dirlist.o $(OBJDIR)dofuncs.o $(OBJDIR)ftp.o $(OBJDIR)locale.o $(OBJDIR)main.o $(OBJDIR)MainWindow.o $(OBJDIR)Menu.o $(OBJDIR)misc.o $(OBJDIR)PrefsWindow.o $(OBJDIR)readdir.o $(OBJDIR)rexx.o $(OBJDIR)SiteWindow.o $(OBJDIR)SpeedBar.o $(OBJDIR)TransferWindow.o
OBJS = $(OBJDIR)main.o $(OBJDIR)bsdsocket.o $(OBJDIR)tcp.o $(OBJDIR)ADT.o \
$(OBJDIR)aguide.o $(OBJDIR)AmiFTP.o $(OBJDIR)AmiFTP_Cat.o $(OBJDIR)cache.o \
$(OBJDIR)change_dir.o $(OBJDIR)config.o $(OBJDIR)ConnectWindow.o $(OBJDIR)data.o \
$(OBJDIR)dirlist.o $(OBJDIR)dofuncs.o $(OBJDIR)ftp.o $(OBJDIR)MainWindow.o \
$(OBJDIR)misc.o $(OBJDIR)PrefsWindow.o $(OBJDIR)readdir.o $(OBJDIR)rexx.o \
$(OBJDIR)SiteWindow.o $(OBJDIR)SpeedBar.o $(OBJDIR)TransferWindow.o

ifeq ($(MENUCLASS), )
	OBJS += $(OBJDIR)Menu.o $(OBJDIR)locale.o
else
	OBJS += $(OBJDIR)MenuClass.o
endif


AmiFTP_Cat.h: AmiFTP.cd
	-$(CATCOMP) AmiFTP.cd AmiFTP_Cat.c=C_c_amiftp.sd
	-$(CATCOMP) AmiFTP.cd AmiFTP_Cat.h=C_h_amiftp.sd


$(EXE) : $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE)_debug
	$(STRIP) $(EXE)_debug -o $(EXE)

clean:
	-$(RM) $(EXE) $(EXE)_debug
	-$(RM) $(OBJDIR)#?.o
