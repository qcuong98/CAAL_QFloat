#include <malloc.h>
#include <iostream>
#include <string>
#include "QFloat.h"

#define BUFFER 1000

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
    QFloat a = Bin2QFloat("0_100000001101001_0011101110001011010110110101000001010110111000010110101100111011111000000100000000000000000000000000000000000000");
    cout << a << endl;
}
