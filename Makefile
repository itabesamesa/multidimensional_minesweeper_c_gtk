CC=gcc # Flag for implicit rules
CFLAGS=$(shell pkg-config --cflags gtk4)
LDFLAGS=$(shell pkg-config --libs gtk4)
#flags := $(shell pkg-config --cflags gtk4)
#gtk_libs := $(shell pkg-config --libs gtk4)

BINARY=main
OBJECTS=minesweeper.o

default: $(BINARY)

$(BINARY): $(OBJECTS)

clean:
	rm -f $(BINARY) $(OBJECTS)

#a.out: main.c
#	$(CC) $(flags) -o a.out main.c $(gtk_libs)

#a.out: ${gtk_libs}
#	gcc $( pkg-config --cflags gtk4 ) -o a.out main.c ${gtk_libs}
