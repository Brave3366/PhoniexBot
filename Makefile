CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread -O2

SRCS = main.c service.c ring_buffer.c telemetry.c services.c
OBJS = $(SRCS:.c=.o)
TARGET = phoenixbot_service

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
