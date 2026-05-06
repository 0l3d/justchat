CC      = cc
SOURCES = justchat.c
OBJECTS = $(SOURCES:.c=.o)
TARGET  = justchat.
VERSION = $(shell git describe --tags --always || echo "v0.1")

CFLAGS  = -O2 -march=native
STATICLDFLAGS = -static -lwebsockets -lssl -lcrypto -lm -lz -lcap -lzstd
LDFLAGS       = -lwebsockets 

.PHONY: all static clean dist-static dist-dynamic

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
	strip -s $(TARGET)
	@echo "Dynamic build finished."

static: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(STATICLDFLAGS)
	strip -s $(TARGET)
	@echo "Static build finished."

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

dist-static: static
	tar -czvf $(TARGET)-static-$(VERSION).tar.gz $(TARGET) LICENSE.md page/

dist-dynamic: all
	tar -czvf $(TARGET)-dynamic-$(VERSION).tar.gz $(TARGET) LICENSE.md page/

clean:
	rm -f $(OBJECTS) $(TARGET) *.tar.gz
