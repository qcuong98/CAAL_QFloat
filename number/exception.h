#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <iostream>
struct Exception {
    const char *detail;
    Exception(const char *);
};
std::ostream &operator<<(std::ostream &, const Exception &e);
#endif
