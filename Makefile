CC = gcc
CC_FLAGS = -Wall -framework Carbon

TARGETS = lswin movewin

all: $(TARGETS)

lswin: lswin.c
	$(CC) $(CC_FLAGS) -o lswin lswin.c

movewin: movewin.c
	$(CC) $(CC_FLAGS) -o movewin movewin.c

clean:
	@rm -f $(TARGETS) core
