/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#ifndef QTPROMISE_TESTS_AUTO_SHARED_SENDER_H
#define QTPROMISE_TESTS_AUTO_SHARED_SENDER_H

#include <QtCore/QObject>

class Object : public QObject
{
    Q_OBJECT

public:
    bool hasConnections() const { return m_connections > 0; }

Q_SIGNALS:
    void noArgSignal();
    void oneArgSignal(const QString& v);
    void twoArgsSignal(int v1, const QString& v0);

protected:
    int m_connections = 0;
    void connectNotify(const QMetaMethod&) Q_DECL_OVERRIDE { ++m_connections; }
    void disconnectNotify(const QMetaMethod&) Q_DECL_OVERRIDE { --m_connections; }
};

#endif // ifndef QTPROMISE_TESTS_AUTO_SHARED_SENDER_H
