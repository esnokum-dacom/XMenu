CC      = gcc
TARGET  = XMenu 
SRCS    = main.c info.c player.c draw.c src/modules/sigr1.c
LIBS    = -lX11 -lXinerama -lImlib2 -lfontconfig $(shell pkg-config --cflags --libs xft)
CFLAGS  = -Wall -Wextra -DASSETS_DIR=\"$(HOME)/.cache/XMenu/src\"
BINDIR  = $(HOME)/.local/bin
SRCDIR  = $(HOME)/.cache/XMenu/src
TDIR    = $(HOME)/.cache/XMenu

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

install: $(TARGET)
	mkdir -p $(BINDIR) && mkdir -p $(TDIR) && mkdir -p $(SRCDIR)
	cp $(TARGET) $(BINDIR)/$(TARGET)
	cp src/* $(SRCDIR)/

clean:
	rm -f $(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)
