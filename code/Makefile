CC      = gcc
#PROF    = -pg
NOCRYPT =
# Uncomment (remove the #) to enable Cygwin support
#ACYGWIN = -DCYGWIN
# Uncomment to disable emailing of passwords in account system
# Value set if Cygwin is enabled (does not work in cygwin)
DEMAIL = -DDEMAIL
#DMTRACE = -DMTRACE

# Uncomment the two lines below if compiling on a Solaris box
#SOLARIS_FLAG = -Dsun -DSYSV
#SOLARIS_LINK = -lnsl -lsocket

#uncomment this line if you are getting a warning like this:
#interp.c:569: warning: int format, __time_t arg (arg7)
TIME = -DTIMEFORMAT

#Uncomment these lines below to remove copying of fear to ../src
ifdef ACYGWIN
   COPYFEAR = rm -f ../src/fear.exe
   COPYFEAR2 = cp fear.exe ../src
   COPYRESOLVER = rm -f ../src/resolver.exe
   COPYRESOLVER2 = cp resolver.exe ../src
   CHMOD1 = chmod g+w fear.exe
   CHMOD2 = chmod a+x fear.exe
   CHMOD3 = chmod g+w resolver.exe
   CHMOD4 = chmod a+x resolver.exe
   CLEAN = rm -f *.o fear.exe resolver.exe *~
   DEMAIL = -DEMAIL
else
   COPYFEAR = rm -f ../src/fear
   COPYFEAR2 = cp fear ../src
   COPYRESOLVER = rm -f ../src/resolver
   COPYRESOLVER2 = cp resolver ../src
   CHMOD1 = chmod g+w fear
   CHMOD2 = chmod a+x fear
   CHMOD3 = chmod g+w resolver
   CHMOD4 = chmod a+x resolver
   CLEAN = rm -f *.o fear resolver *~
endif

#Uncomment the line below if you are getting undefined crypt errors
NEED_CRYPT = -lcrypt

#For those with trigraph warnings
TRI = -Wno-trigraphs

DBUGFLG =

#DO not uncomment, it was ripped out and will not work
#IMC2 - Comment out to disable IMC2 support
#IMC = 1

#Uncomment the line below if you want a performance increase though beware
#your core files may not be as much of a benefit if you do.
#OPT_FLAG = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce
# Uncomment both OPT_FLAG and MCCP if you don't wish to use it or don't have zlib support
#OPT_FLAG = -DMCCP
#CMCCP = -lz

C_FLAGS = $(OPT_FLAG) -O -g2 -Wall $(TRI) $(TIME) $(PROF) $(NOCRYPT) $(DBUGFLG) $(SOLARIS_FLAG) $(ACYGWIN) $(DEMAIL) $(DMTRACE)
L_FLAGS = $(OPT_FLAG) $(PROF) $(SOLARIS_LINK) $(NEED_CRYPT) $(CMCCP)
#D_FLAGS : For the DNS Slave process. No need in linking all the extra libs for this.
D_FLAGS = -g2 -O $(PROF) $(SOLARIS_LINK)
          
C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c boards.c \
	  build.c clans.c comm.c comments.c const.c db.c deity.c dns.c fight.c \
	  handler.c hashstr.c interp.c ident.c magic.c makeobjs.c \
	  mapout.c misc.c mpxset.c mud_comm.c mud_prog.c player.c polymorph.c \
          requests.c reset.c save.c shops.c skills.c special.c tables.c \
          track.c update.c grub.c stat_obj.c ban.c services.c planes.c \
          imm_host.c color.c pfiles.c bank.c finger.c infochan.c \
          slay.c alias.c caste.c editor.c gboard.c ansi.c arena.c\
          asciimap.c overland.c archery.c treasure.c forge.c raferquest.c \
          roleplay.c name_gen.c cleanup.c

ifdef IMC
   C_FILES := imc.c $(C_FILES)
   IMCFILE = imc.o
   C_FLAGS := $(C_FLAGS) -DIMC -DIMCSMAUG
endif
          
O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o boards.o \
	  build.o clans.o comm.o comments.o const.o db.o dns.o deity.o fight.o \
	  handler.o hashstr.o interp.o ident.o magic.o makeobjs.o \
	  mapout.o misc.o mpxset.o mud_comm.o mud_prog.o player.o polymorph.o \
          requests.o reset.o save.o shops.o skills.o special.o tables.o \
          track.o update.o grub.o stat_obj.o ban.o services.o planes.o \
          imm_host.o color.o pfiles.o bank.o finger.o infochan.o \
          slay.o alias.o caste.o editor.o gboard.o ansi.o arena.o\
          asciimap.o overland.o archery.o treasure.o forge.o raferquest.o \
          roleplay.o name_gen.o cleanup.o
 
H_FILES = mud.h ansi.h bet.h color.h overland.h

all:
	make -s fear
	make -s dns

fear: $(O_FILES)
	rm -f fear
	$(CC) $(L_FLAGS) -o fear $(O_FILES) $(NEED_CRYPT)
	$(CHMOD1)
	$(CHMOD2)
	chmod g+w $(O_FILES)
	$(COPYFEAR)
	$(COPYFEAR2)

dns: resolver.o
	rm -f resolver
	$(CC) $(D_FLAGS) -o resolver resolver.o
	$(CHMOD3)
	$(CHMOD4)
	chmod g+w resolver.o
	$(COPYRESOLVER)
	$(COPYRESOLVER2)

.c.o: mud.h
	echo "Fear 2.x now compiling... $@";
	$(CC) -c $(C_FLAGS) $<

clean:
	$(CLEAN)
