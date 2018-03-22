build:
	g++ *.cpp number/*.cpp -std=c++11 -O3 -fsanitize=address,undefined -fno-omit-frame-pointer
run:
	@make --no-print-directory
	(./a.out $(A); rm a.out)
debug:
	g++ *.cpp number/*.cpp -std=c++11 -g
