#include <malloc.h>
#include <iostream>
#include <string>
#include "QFloat.h"

std::istream &operator>>(std::istream &is, QFloat &a) {
    std::string tmp;
    is >> tmp;
    a = Dec2QFloat(tmp.c_str());
    return is;
}

std::ostream &operator<<(std::ostream &os, const QFloat &a) {
    char *tmp = QFloat2Dec(a);
    os << tmp;

    free(tmp);
    return os;
}

int main() {
    using namespace std;
    QFloat a = Dec2QFloat("1e32");
    cout << a << endl;
}
