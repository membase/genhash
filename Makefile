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
