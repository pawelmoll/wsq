# libwsq imported from NBIS rel 4.2.0

OBJS := wsq/cropcoeff.o wsq/decoder.o wsq/encoder.o wsq/globals.o wsq/huff.o wsq/ppi.o wsq/sd14util.o wsq/tableio.o wsq/tree.o wsq/util.o
OBJS += jpegl/decoder.o jpegl/encoder.o jpegl/huff.o jpegl/huftable.o jpegl/imgdat.o jpegl/ppi.o jpegl/sd4util.o jpegl/tableio.o jpegl/util.o
OBJS += fet/allocfet.o fet/extrfet.o fet/lkupfet.o fet/printfet.o fet/strfet.o fet/writefet.o fet/delfet.o fet/freefet.o fet/nistcom.o fet/readfet.o fet/updatfet.o
OBJS += util/computil.o util/syserr.o util/fatalerr.o
OBJS += ioutil/dataio.o ioutil/filesize.o

CPPFLAGS := -Iinclude
CFLAGS := -Wall -g -D__NBISLE__
LDFLAGS := -lm

all: png2wsq wsq2png

clean:
	rm -f $(OBJS)
	rm -f png2wsq png2wsq.o
	rm -f wsq2png wsq2png.o

png2wsq: png2wsq.o $(OBJS)
	$(CC) $^ -o $@ -lpng $(LDFLAGS)

png2wsq.o: png2wsq.c

wsq2png: wsq2png.o $(OBJS)
	$(CC) $^ -o $@ -lpng $(LDFLAGS)

wsq2png.o: wsq2png.c
