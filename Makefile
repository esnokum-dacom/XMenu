CC      = gcc
TARGET  = XMenu 
SRCS    = main.c info.c player.c
LIBS    = -lX11 -lXinerama -lImlib2 -lfontconfig $(shell pkg-config --cflags --libs xft)
CFLAGS  = -Wall -Wextra
BINDIR  = $(HOME)/.local/bin
TDIR    = $(HOME)/.cache/XMenu

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

install: $(TARGET)
	mkdir -p $(BINDIR) mkdir -p $(TDIR)
	cp $(TARGET) $(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)
