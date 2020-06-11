CC=gcc
default_recipe: build

.PHONY: build
build:
	mkdir -p build
	$(CC) -o build/agame \
		-I ./internal \
		-I ./include \
		-L ./src/ \
		-I/usr/local/include/SDL2 \
		-L/usr/local/lib \
		-Wl,-rpath,/usr/local/lib \
		-Wl,--enable-new-dtags \
		-lSDL2 \
		-lSDL2_image \
		-lSDL2_ttf \
		-lm \
		-static \
		main.c \
		src/entity/enemy.c \
		src/entity/entity.c \
		src/entity/light.c \
		src/entity/pew.c \
		src/entity/player.c \
		src/entity/spawner.c \
		src/event/event.c \
		src/maze.c \
		src/state.c \
		src/util.c
	cp -r res ./build/



