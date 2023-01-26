LIBS = -lz
CC = gcc

DEPS = src/*.c

INC = include/
INCS = include/*.h

nbt_viewer: $(DEPS) $(INCS) makefile
	$(CC) -o nbt_viewer -Wall $(DEPS) $(LIBS) -I $(INC)