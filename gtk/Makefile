Objects := objs/app.o objs/patch.o
Depend  := $(shell pkg-config --libs --cflags gtk4 gstreamer-1.0)

main: ${Objects}
	gcc ${Objects} ${Depend} -o run

objs/app.o: src/app.cpp
	gcc -c src/app.cpp -o objs/app.o ${Depend}

objs/patch.o: bin_patches/patch.cpp
	gcc -c bin_patches/patch.cpp -o objs/patch.o
