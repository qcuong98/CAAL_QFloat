#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "QFloat.h"
using namespace std;

// Chuyển từ QFloat sang xâu, cơ số `base`
static string to_str(const QFloat &q, int base) {
    char *s = NULL;
    if (base == 10)
        s = QFloat2Dec(q);
    if (base == 2)
        s = QFloat2Bin(q);
    if (s == NULL)
        abort();
    string res(s);
    free(s);
    return res;
}

// Chuyển từ một xâu có cơ số `base` sang QFloat
static QFloat from_str(const string &s, int base) {
    if (s.empty())
        return QFloat();
    if (base == 10)
        return Dec2QFloat(s.data());
    if (base == 2)
        return Bin2QFloat(s.data());
    abort();
}

// Cấu trúc để đọc và xử lý một dòng test
struct Input {
    int in_base;
    int out_base;
    string op;
    string lhs, rhs;

    Input(const string &&s) {
        stringstream ss(s);
        string a, b, c, d;
        ss >> a >> b >> c >> d;
        stringstream(a) >> in_base;
        if (d.empty()) {
            if (c == "-") {
                // -a
                out_base = in_base;
                op       = c;
                lhs      = b;
                rhs      = "";
            } else {
                // Convert between bases
                stringstream(b) >> out_base;
                op  = "conv";
                lhs = c;
                rhs = "";
            }
        } else {
            // Two-operand operators
            out_base = in_base;
            op       = c;
            lhs      = b;
            rhs      = d;
        }
    }

    string output() const {
#define RETURN(EXP) \
    { return to_str(EXP, out_base); }
#define CHECK(EXP)                  \
    {                               \
        if (!(EXP))                 \
            return "Invalid input"; \
    }
#define CASE(x) if (op == x)

        CHECK(in_base == 10 || in_base == 16 || in_base == 2);
        CHECK(out_base == 10 || out_base == 16 || out_base == 2);

        QFloat L = from_str(lhs, in_base);

        CASE("conv") RETURN(L);

        QFloat R = from_str(rhs, in_base);
        CASE("-") {
            if (rhs.empty())
                RETURN(-L);
            RETURN(L - R);
        }
        CASE("+") RETURN(L + R);
        CASE("*") RETURN(L * R);
        CASE("/") RETURN(L / R);

        CHECK(false);
#undef RETURN
#undef CHECK
#undef CASE
    }
};

// Nhập input từ file argv[1],
// Xuất output ra file argv[2],
// Khi không có argv[2] thì xuất ra màn hình
int real_main(int argc, char **argv) {
    if (argc < 2)
        abort();

    ifstream in(argv[1]);
    vector<Input> inputs;
    for (string line; getline(in, line);)
        inputs.push_back(Input(move(line)));

    stringstream output;
    for (auto &input : inputs)
        output << input.output() << endl;

    if (argc >= 3) {
        ofstream(argv[2]) << output.str();
    } else {
        cout << output.str();
    }
    return 0;
}

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

int debug_main() {
    using namespace std;
    QFloat a, b;
    cin >> a >> b;
    cout << a << " op " << b << endl;
    cout << "op + = " << a + b << endl;
    cout << "op - = " << a - b << endl;
    cout << "op * = " << a * b << endl;
    cout << "op / = " << a / b << endl;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2)
        return debug_main();
    return real_main(argc, argv);
}
