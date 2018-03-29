build:
	g++ *.cpp number/*.cpp -Wall -std=c++11 -g -fsanitize=address,undefined -fno-omit-frame-pointer
optimized:
	g++ *.cpp number/*.cpp -Wall -std=c++11 -Ofast
run:
	@make --no-print-directory
	(./a.out $(A); rm a.out)
debug:
	g++ *.cpp number/*.cpp -std=c++11 -g
