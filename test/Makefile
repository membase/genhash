# arch-tag: FCD369F7-B46B-4CFB-BC55-68B7124E588D

CFLAGS=-Wall -Werror -g -I.. -g
LDFLAGS=-L.. -g -lgenhash
THELIB=../libgenhash.a

PROGS=hashtest
OBJS=hashtest.o

all: $(PROGS)

hashtest: hashtest.o
	$(CC) -o $@ hashtest.o $(LDFLAGS)

$(OBJS): $(THELIB)

clean:
	rm $(PROGS) $(OBJS)
