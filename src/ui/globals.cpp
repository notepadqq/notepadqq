#include "include/globals.h"
#include <QTextStream>

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
