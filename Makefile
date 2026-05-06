CC      = cc
SOURCES = justchat.c
OBJECTS = $(SOURCES:.c=.o)
TARGET  = justchat

CFLAGS  = -O2
STATICLDFLAGS = -static -lwebsockets -lssl -lcrypto -lm -lz -lcap -lzstd
LDFLAGS       = -lwebsockets 

.PHONY: all static clean 

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

clean:
	rm -f $(OBJECTS) $(TARGET) *.tar.gz
