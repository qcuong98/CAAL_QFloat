#ifndef NUMBER_H
#define NUMBER_H
#include <iostream>

#define LL long long
const LL B               = 100000000;
const LL SIZE            = 8;
const LL M[]             = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};
const int PRECISION_SIZE = 12;
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
    Number exp() const;
    Number chop(int) const;

    friend std::ostream &operator<<(std::ostream &, const Number &);
    char *to_str() const;
};

static const Number TWO(2);
static const Number ONE(1);
static const Number HALF("0.5");
static const Number LOG2(
    "0."
    "3010299956639811952137388947244930267681898814621085413104274611271081892744245094869272521181"
    "86172041");
static const Number LN10(
    "2."
    "3025850929940456840179914546843642076011014886287729760333279009675726096773524802359972050895"
    "9829834");
static const Number LOG2_10(
    "3."
    "3219280948873623478703194294893901758648313930245806120547563958159347766086252158501397433593"
    "701551");
static const Number LN2(
    "0."
    "6931471805599453094172321214581765680755001343602552541206800094933936219696947156058633269964"
    "18687542");
#endif
