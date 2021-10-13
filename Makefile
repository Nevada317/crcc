CFLAGS+= -I.
CC?=gcc

crcc: main.o crc.o file_read.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	chmod +x $@

%.o: %.c *.h
	$(CC) -c $(CFLAGS) -o $@ $< $(LIBS)

install: crcc
	cp crcc /bin/
