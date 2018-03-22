#include "exception.h"

Exception::Exception(const char *s): detail(s) {}

std::ostream& operator << (std::ostream &os, const Exception &e) {
    return os << e.detail;
}
