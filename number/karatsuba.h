#ifndef KARATSUBA_H
#define KARATSUBA_H
namespace Karatsuba {
    typedef struct {
        int n;
        long long *d;
    } num;
    num karatsuba(num, num);
    void print(num);
}
#endif
