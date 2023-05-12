# (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
# All Rights Reserved / Alle Rechte vorbehalten
# $Id: Makefile,v 4.112 2006/04/29 10:58:29 tommy Exp tommy $

.SUFFIXES:	.scm

RELEASE = 	4.108

NOBJECTS =      scheme.o atom.o heap.o util.o machine.o primitives.o ports.o boot.o auxmem.o net.o
BOBJECTS =      bitmap.o bitprim.o xbmread.o pngif.o pconvert.o v4lif.o
COBJECTS = 	

#################################################################
# 

### Inlab-Scheme with image support 
#OBJECTS =	$(NOBJECTS) $(BOBJECTS)
#PRIMITIVES =    primitives.c bitprim.c net.c
#LIBRARIES=	-ltiff -lpng -lz -lm

### raw Inlab-Scheme (no extensions) 
OBJECTS =	$(NOBJECTS) $(COBJECTS) 
PRIMITIVES =    primitives.c 
LIBRARIES=	-lm

#################################################################

IFS =		ifs.o
INTERNALFILES = init.scm
INCLUDES=	scheme.h fdecl.h global.h intern.h extern.h heap.h

CC      =	gcc
#CFLAGS  = 	-O -W -I. -DNDEBUG 
CFLAGS  = 	-g -W -I. -DNDEBUG 

#################################################################

all:		scheme 

ci:
		#make clean
		ci -l [a-z]*.c *.h *.scm Makefile README COPYING primcvt.awk

ever:

escheme:	scheme
		ln scheme escheme

scheme:		$(OBJECTS) $(IFS) $(LIBRARIES) 
		$(CC) -o scheme $(OBJECTS) $(IFS) $(LIBRARIES) 
		strip scheme

$(OBJECTS):	$(INCLUDES)

bitmap.o:	bitmap.h

Aprimitives.c:	primitives.c
		echo '/* AUTOMATICALLY GENERATED - DO NOT CHANGE */' > Aprimitives.c 
		cat primitives.c | awk -f primcvt.awk >> Aprimitives.c

primitives.o:	Aprimitives.c
		$(CC) $(CFLAGS) -c Aprimitives.c
		mv Aprimitives.o primitives.o

Abitprim.c:	bitprim.c
		echo '/* AUTOMATICALLY GENERATED - DO NOT CHANGE */' > Abitprim.c 
		cat bitprim.c | awk -f primcvt.awk >> Abitprim.c

Anet.c:		net.c
		echo '/* AUTOMATICALLY GENERATED - DO NOT CHANGE */' > Anet.c 
		cat net.c | awk -f primcvt.awk >> Anet.c

bitprim.o:	Abitprim.c
		$(CC) $(CFLAGS) -c Abitprim.c
		mv Abitprim.o bitprim.o

net.o:		Anet.c
		$(CC) $(CFLAGS) -c Anet.c
		mv Anet.o net.o

ifs.o:		integrate $(INTERNALFILES)
		echo '/* AUTOMATICALLY GENERATED - DO NOT CHANGE */' > Aifs.c 
		./integrate $(INTERNALFILES) >> Aifs.c
		$(CC) $(CFLAGS) -c Aifs.c
		mv Aifs.o ifs.o

integrate:	integrate.c
		$(CC) $(CFLAGS) -o integrate integrate.c

Aboot.c:	$(PRIMITIVES) boot.h Makefile
		echo '/* AUTOMATICALLY GENERATED - DO NOT CHANGE */' > Aboot.c
		echo "#include <boot.h>" >> Aboot.c
		echo "SchemeBootPrimitives() {" >> Aboot.c
		egrep -h '^PrimitiveProcedure\(' $(PRIMITIVES) | awk -f primcvt.awk >> Aboot.c
		echo "}" >> Aboot.c

boot.o:		Aboot.c	
		$(CC) -c $(CFLAGS) Aboot.c
		mv Aboot.o boot.o

clean:
		rm -f *.o
		rm -f integrate
		rm -f scheme
		rm -f escheme
		rm -rf *.dbs
		rm -f core
		rm -f core.*
		rm -f *.core
		rm -f OUTFILE
		rm -f .OUTFILE
		rm -f nohup.out
		rm -f Abitprim.c Aprimitives.c
		rm -f Aifs.c
		rm -f Aboot.c 
		rm -f Anet.c 

install:
		install -c -g bin -o bin scheme /usr/local/bin/scheme
		# ln /usr/local/bin/scheme /usr/local/bin/escheme

release:	clean	
		rm -rf ./releases/Inlab-Scheme-$(RELEASE)
		mkdir ./releases/Inlab-Scheme-$(RELEASE)
		cp [a-z]*.c *.h *.scm Makefile README COPYING primcvt.awk ./releases/Inlab-Scheme-$(RELEASE)
		cd releases; tar --owner=0 --group=0 -cvf Inlab-Scheme-$(RELEASE).tar ./Inlab-Scheme-$(RELEASE)
		cd releases; gzip Inlab-Scheme-$(RELEASE).tar
 


