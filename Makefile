movewin: movewin.c
	gcc -framework Carbon -o movewin movewin.c

clean:
	@rm -f movewin core
