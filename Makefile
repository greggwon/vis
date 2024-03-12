
LDLIBS=-lncurses
BINDIR=/usr/local/bin
CFLAGS=-g
LDFLAGS=-g

vis: vis.o

install : ${BINDIR}/vis

${BINDIR}/vis : vis
	cp vis ${BINDIR}/vis

clean:
	rm -f vis.o vis
