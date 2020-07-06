CC?=$(CROSS_COMPILE)gcc
CFLAGS?=-ggdb \
	   	-O0 \
	   	-Wall \
	   	-Werror \
		-Wno-char-subscripts \
		-Iinclude
LDFLAGS?=-lgcc

O=build

sh_OBJS=$(O)/sh.o \
		$(O)/readline.o \
		$(O)/cmd.o \
		$(O)/parse.o \
		$(O)/builtin.o \
		$(O)/job.o \
		$(O)/env.o

all: mkdirs $(O)/sh

mkdirs:
	mkdir -p $(O)

clean:
	rm -rf $(O)

install: mkdirs $(O)/sh
	install -m0755 $(O)/sh $(DESTDIR)/bin/sh

$(O)/sh: $(sh_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(sh_OBJS)

$(O)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
