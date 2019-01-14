#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#ifndef USE_DBUS
#include <QLocalServer>
#endif
/**
 * @brief QApplication subclass which, using local sockets, makes sure only
 *        one instance of the application is running at the same time.
 */
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

#ifdef USE_DBUS
public slots:
    Q_SCRIPTABLE void receive(const QString& workingDirectory, const QStringList& arguments);
#else
private:
    QLocalServer *m_localServer = nullptr;

    bool sendCommandLineArguments(QLocalSocket *socket);
    QLocalSocket *alreadyRunningInstance();
    void newConnection();
    QString socketNameForUser();
#endif
};

#endif // SINGLEAPPLICATION_H
