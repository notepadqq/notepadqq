#ifndef JS_BINDINGS_H
#define JS_BINDINGS_H

#include <tuple>

namespace EditorNS
{

struct Cursor {
    int line;
    int column;

    bool operator == (const Cursor &x) const {
        return line == x.line && column == x.column;
    }

    bool operator < (const Cursor &x) const {
        return std::tie(line, column) < std::tie(x.line, x.column);
    }

    bool operator <= (const Cursor &x) const {
        return *this == x || *this < x;
    }

    bool operator > (const Cursor &x) const {
        return !(*this <= x);
    }

    bool operator >= (const Cursor &x) const {
        return !(*this < x);
    }
};

struct Selection {
    Cursor from;
    Cursor to;
};

struct IndentationMode {
    bool useTabs;
    int size;
};
}

#endif // JS_BINDINGS_H