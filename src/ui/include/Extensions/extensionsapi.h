#ifndef EXTENSIONSAPI_H
#define EXTENSIONSAPI_H

#include <QObject>
#include "include/mainwindow.h"

class ExtensionsApi : public QObject
{
    Q_OBJECT

public:
    static ExtensionsApi *instance();

private:
    ExtensionsApi(QObject *parent = 0);
    ExtensionsApi(const ExtensionsApi &);
    ExtensionsApi& operator=(const ExtensionsApi &);

    static ExtensionsApi* m_instance;

signals:
    void newWindow(MainWindow *window);

public slots:
};

#endif // EXTENSIONSAPI_H
