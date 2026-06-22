/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#ifndef QTPROMISE_QPROMISEEXCEPTIONS_H
#define QTPROMISE_QPROMISEEXCEPTIONS_H

#include "qpromiseglobal.h"

#include <QtCore/QException>

namespace QtPromise {

class QPromiseCanceledException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseCanceledException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseCanceledException{*this};
    }
};

class QPromiseContextException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseContextException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseContextException{*this};
    }
};

class QPromiseConversionException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseConversionException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseConversionException{*this};
    }
};

class QPromiseTimeoutException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseTimeoutException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseTimeoutException{*this};
    }
};

class QPromiseUndefinedException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseUndefinedException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseUndefinedException{*this};
    }
};

} // namespace QtPromise

#endif // QTPROMISE_QPROMISEEXCEPTIONS_H
