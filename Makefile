# arch-tag: 4A4074A7-19F1-49B2-882E-09BBC7BD170F

CFLAGS=-Wall -Werror -g

OBJS=genhash.o

THELIB=libgenhash.a

all: $(THELIB)

clean:
	rm $(OBJS) $(THELIB)

genhash.o: genhash.c genhash.h

$(THELIB): $(OBJS)
	$(AR) rcs $@ $(OBJS)
	ranlib $@
