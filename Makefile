all: space


WARNINGS = -Wall
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2


space: Makefile main.c ./glad/src/glad.c
	$(CC) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) main.c ./glad/src/glad.c -lglfw3 -lrt -lm -ldl

clean:
	rm -f space

# Builder will call this to install the application before running.
install:
	./space

# Builder uses this target to run your application.
run:
	./space

