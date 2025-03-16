CC=gcc
CFLAGS=-O3 -Wall -fopenmp
LIBS=-lm -fopenmp

all: wave_gif_maker

wave_gif_maker: wave_gif_maker.c stb_image_write.h
	$(CC) $(CFLAGS) -o wave_gif_maker wave_gif_maker.c $(LIBS)

clean:
	rm -f wave_gif_maker
	rm -f frames/*.png
	rm -rf frames 