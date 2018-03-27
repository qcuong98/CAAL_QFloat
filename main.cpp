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
    using namespace std;
    QFloat a = Dec2QFloat("10000000000000000000000.1231");
    cout << a;
}
