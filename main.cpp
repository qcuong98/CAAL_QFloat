#include "QFloat.h"
#include <iostream>
#include <string>
#include <malloc.h>

#define BUFFER 1000

std::istream & operator >> (std::istream &is, QFloat &a) {
	std::string tmp;
	is >> tmp;
	a = Dec2QFloat(tmp.c_str());
	return is;
}

std::ostream & operator << (std::ostream &os, const QFloat &a) {
	char *tmp = QFloat2Dec(a);
	os << tmp;

	free(tmp);
	return os;
}

int main() {
	QFloat a, b;

	std::cin >> a >> b;
	std::cout << a << " + " << b << " = " << a + b << std::endl;
	return 0;
}
