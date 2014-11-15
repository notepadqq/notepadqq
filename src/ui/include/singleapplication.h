#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QLocalServer>

class SingleApplication : public QApplication
{
    Q_OBJECT
public:
    explicit SingleApplication(int &argc, char **argv);

    void startServer();

    /**
     * @brief If another instance of Notepadqq is opened, sends it
     *        the current command line arguments.
     * @return true if another instance was found and received the arguments,
     *         false otherwise.
     */
    bool attachToOtherInstance();
signals:
    void receivedArguments(const QString &workingDirectory, const QStringList &arguments);

public slots:

private:
    QLocalServer *m_localServer = nullptr;

    bool sendCommandLineArguments(QLocalSocket *socket);
    QLocalSocket *alreadyRunningInstance();
    void newConnection();
    QString socketNameForUser();
};

#endif // SINGLEAPPLICATION_H
