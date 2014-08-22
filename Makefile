CC = gcc
CC_FLAGS = -Wall
LD = gcc
LD_FLAGS = -Wall -framework Carbon

TARGETS = lswin movewin
OBJECTS = lswin.o movewin.o winutils.o

all: $(TARGETS)

lswin: winutils.o lswin.o
	$(LD) $(LD_FLAGS) -o lswin winutils.o lswin.o

movewin: winutils.o movewin.o
	$(LD) $(LD_FLAGS) -o movewin winutils.o movewin.o

winutils.o: winutils.h winutils.c
	$(CC) $(CC_FLAGS) -c winutils.c

lswin.o: lswin.c
	$(CC) $(CC_FLAGS) -c lswin.c

movewin.o: movewin.c
	$(CC) $(CC_FLAGS) -c movewin.c

clean:
	@rm -f $(TARGETS) $(OBJECTS) core
