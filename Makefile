run: clean build
	while true; do ./a.out && sleep 9; done

build:
	g++ -lz -lssl -luWS -Ofast -march=native -fno-stack-protector -fno-pie a.cpp
	strip a.out

clean:
	rm a.out
