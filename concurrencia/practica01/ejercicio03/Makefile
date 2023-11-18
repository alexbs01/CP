CC=gcc
CFLAGS=-Wall -g -O0
LIBS=
OBJS=array.o options.o

PROGS= array

all: $(PROGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -g

array: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) -g

clean:
	rm -f $(PROGS) *.o *~

