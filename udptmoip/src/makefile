IDIR =../inc
CC=gcc -g -ggdb -O0
CFLAGS=-I$(IDIR)

ODIR=obj
LIBS=-lm


_DEPS = tmoip.h
DEPS = $(patsubst %, $(IDIR)/%, $(_DEFS))

_OBJ = udp_tmoip.o tmoip_parse.o
OBJ = $(patsubst %,$(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: %.c  $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

udptest: $(OBJ)
	$(CC) -o $@ $^ $(FLAGS) $(LIBS)


.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *.~ core $(INCDIR)/*~

