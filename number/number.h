#ifndef NUMBER_H
#define NUMBER_H
#include <iostream>

#define LL long long
const LL B               = 100000000;
const LL SIZE            = 8;
const LL M[]             = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};
const int PRECISION_SIZE = 14;
const int PRECISION      = PRECISION_SIZE * SIZE;


class Number {
    static int raw_compare(const Number &, const Number &);
    static Number raw_add(const Number &, const Number &);
    static Number raw_multiply(const Number &, const Number &);
    static Number raw_substract(const Number &, const Number &);
    static Number raw_divide(const Number &, const Number &);

    Number(int, int, long long *);
    void unpad();
    void shift(int);
    Number &invert();
    Number intpow(const Number &) const;

  public:
    int sign;
    int n;
    long long *d;

    ~Number();
    Number();
    Number(const char *);
    Number(const Number &);
    explicit Number(long long);

    Number &operator=(const Number &);

    bool operator<(const Number &) const;
    bool operator>(const Number &) const;
    bool operator<=(const Number &) const;
    bool operator>=(const Number &) const;
    bool operator==(const Number &) const;
    bool operator!=(const Number &) const;

    Number operator-() const;
    Number operator+(const Number &) const;
    Number operator-(const Number &) const;
    Number operator*(const Number &)const;
    Number operator/(const Number &) const;
    Number operator^(const Number &) const;

    Number log() const;
    Number fraction() const;
    Number floor() const;
    Number abs() const;
    Number sin() const;
    Number cos() const;
    Number tan() const;
    Number asin() const;
    Number acos() const;
    Number atan() const;
    Number exp() const;

    friend std::ostream &operator<<(std::ostream &, const Number &);
    char *to_str() const;
};
#endif
