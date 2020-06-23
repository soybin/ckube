marchc: march.c
	$(CC) march.c -o march -w -lm -lncurses -std=c99
	./march
