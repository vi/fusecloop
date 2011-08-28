PROGNAME=fusecloop
ARCFILES=*.c *.h *.pl Makefile configure README VERSION HELP INSTALL typescript *.cloop COPYING
PROGS=fusecloop cloopreaderdemo extract_compressed_fs create_compressed_fs
FUSECFLAGS=`pkg-config fuse --cflags`
FUSELDFLAGS=`pkg-config fuse --libs` 

CFLAGS= -Wall

all: fusecloop extract_compressed_fs create_compressed_fs

extract_compressed_fs: extract_compressed_fs.c
	${CC} ${CFLAGS} ${LDFLAGS} -lz extract_compressed_fs.c -o extract_compressed_fs

create_compressed_fs: create_compressed_fs.c
	${CC} ${CFLAGS} ${LDFLAGS} -lz create_compressed_fs.c -o create_compressed_fs

fusecloop: fusecloop.c cloopreader.o strver debug.o
	${CC} ${CFLAGS} ${LDFLAGS} -lz cloopreader.o ${FUSECFLAGS} ${FUSELDFLAGS} fusecloop.c debug.o -o fusecloop

# ${FUSECFLAGS} here for 64 bit file pointers.
cloopreader.o: cloopreader.c
	${CC} ${CFLAGS} ${FUSECFLAGS} cloopreader.c -c -o cloopreader.o

debug.o: debug.c
	${CC} ${CFLAGS} debug.c -c -o debug.o

.PHONY: check mount umount arc clean check_ test

test: check

check: mount check_ umount

check_: hw
	#sleep 1
	diff hw testmp
	cmp testmp hw
	sleep 1
	

mount: fusecloop
	touch testmp
	./fusecloop hw.cloop testmp

umount:
	fusermount -u testmp

# Convert something like 0.10.0 into "0.10.0"
strver: VERSION
	echo "\"`cat VERSION`\"" > strver

clean:
	rm -f *.exe ${PROGS} *.o .*.swp *~ *.gch *.s *.i strver testmp hw fusecloop.log
	rm -Rf arc

dist:
	mkdir -p arc
	mkdir ${PROGNAME}-`cat VERSION`
	cp ${ARCFILES} ${PROGNAME}-`cat VERSION`/
	tar -cjf arc/${PROGNAME}-`cat VERSION`.tar.bz2 ${PROGNAME}-`cat VERSION`
	rm -Rf ${PROGNAME}-`cat VERSION`/
	perl incver.pl

install:
	echo "No install yet. Copy fusecloop where you like (to /usr/local/bin/ or ~/bin/)"

hw:    
	yes '#define hw "Hello world.\n"' | dd bs=1024 count=4096 > hw
