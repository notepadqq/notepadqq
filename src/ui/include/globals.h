#ifndef GLOBALS_H
#define GLOBALS_H

#include <QString>
#include <QtPromise>

#include <functional>

void print(QString string);
void println(QString string);
void printerr(QString string);
void printerrln(QString string);


namespace PForResult {
    enum Enum {
        Continue,
        Break
    };
}

/**
 * @brief Performs a Promise-based for loop. Each iteration starts only after the promise at the previous iteration is fulfilled.
 * @param start start index (inclusive)
 * @param end end index (exclusive, must be > start)
 * @param iteration function providing the current index, a break promise and a continue promise. The break and continue promises
 *                  can be returned in order to resp. break or continue the loop.
 * @return A promise which is fulfilled when the loop terminates.
 */
QtPromise::QPromise<PForResult::Enum> pFor(int start, int end, std::function<QtPromise::QPromise<PForResult::Enum>(int i, QtPromise::QPromise<PForResult::Enum> _break, QtPromise::QPromise<PForResult::Enum> _continue)> iteration);

#endif // GLOBALS_H

