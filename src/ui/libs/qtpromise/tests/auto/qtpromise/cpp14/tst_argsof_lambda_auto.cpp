/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include <QtPromise>
#include <QtTest>

using namespace QtPromisePrivate;

class tst_cpp14_argsof_lambda_auto : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void lambdaAutoArgs();
};

QTEST_MAIN(tst_cpp14_argsof_lambda_auto)
#include "tst_argsof_lambda_auto.moc"

void tst_cpp14_argsof_lambda_auto::lambdaAutoArgs()
{
    auto lOneArg = [](auto) {};
    auto lManyArgs = [](const auto&, auto, auto) {};
    auto lMutable = [](const auto&, auto) mutable {};

    Q_STATIC_ASSERT((ArgsOf<decltype(lOneArg)>::count == 0));
    Q_STATIC_ASSERT((ArgsOf<decltype(lManyArgs)>::count == 0));
    Q_STATIC_ASSERT((ArgsOf<decltype(lMutable)>::count == 0));
}
