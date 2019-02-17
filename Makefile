run: clean build
	while true; do ./a.out && sleep 9; done

build:
	gcc -lssl -Ofast -march=native a.c
	strip a.out

clean:
	rm -f a.out
