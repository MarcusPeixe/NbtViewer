LIBS = -lz
CC = gcc

DEPS = src/*

INC = include/
INCS = include/*

nbt_viewer: $(DEPS) $(INCS)
	$(CC) -o nbt_viewer -Wall $(DEPS) $(LIBS) -I $(INC)