CC?=$(CROSS_COMPILE)gcc
CFLAGS?=-ggdb \
	   	-O0 \
	   	-Wall \
	   	-Werror
LDFLAGS?=-lgcc

O=build

sh_OBJS=$(O)/sh.o

all: mkdirs $(O)/sh

mkdirs:
	mkdir -p $(O)

clean:
	rm -rf $(O)

install: $(O)/sh
	install -m0755 $(O)/sh $(DESTDIR)/sh

$(O)/sh: $(sh_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(sh_OBJS)

$(O)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
