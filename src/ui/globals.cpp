#include "include/globals.h"

#include <QTextStream>
#include <QtPromise>

void print(QString string)
{
    static QTextStream ts(stdout);
    ts << string;
    ts.flush();
}

void println(QString string)
{
    print(string + "\n");
}

void printerr(QString string)
{
    static QTextStream ts(stderr);
    ts << string;
    ts.flush();
}

void printerrln(QString string)
{
    printerr(string + "\n");
}

QtPromise::QPromise<PForResult::Enum> pFor(int start, int end, std::function<QtPromise::QPromise<PForResult::Enum>(int, QtPromise::QPromise<PForResult::Enum>, QtPromise::QPromise<PForResult::Enum>)> iteration) {
    QtPromise::QPromise<PForResult::Enum> p = QtPromise::QPromise<PForResult::Enum>::resolve(PForResult::Continue);

    for (int i = start; i < end; i++) {
        p = p.then([=](PForResult::Enum result){
            const auto _break = QtPromise::QPromise<PForResult::Enum>::resolve(PForResult::Break);
            const auto _continue = QtPromise::QPromise<PForResult::Enum>::resolve(PForResult::Continue);
            if (result == PForResult::Break) {
                return _break; // TODO It is inefficient to transfer the "Break" message all to the end of the loop
            } else {
                return iteration(i, _break, _continue);
            }
        });
    }

    return p;
}
