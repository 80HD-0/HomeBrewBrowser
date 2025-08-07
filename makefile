CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags gtk4`
LDFLAGS = `pkg-config --libs gtk4` -lcurl
SRCS = hbb.c
OBJS = $(SRCS:.c=.o)
TARGET = builds/HomeBrewBrowser
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: all clean
