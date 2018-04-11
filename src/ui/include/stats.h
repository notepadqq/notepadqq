#ifndef STATS_H
#define STATS_H

#include <QJsonObject>

class Stats
{
public:
    /**
     * @brief Update/start timers. You can safely call this method multiple times.
     */
    static void init();

private:

    /**
     * @brief Send the specified JSON object to the remote server.
     * @param data
     */
    static void remoteApiSend(const QJsonObject &data);

    /**
     * @brief Check if it's time to send the stats. If it is, send them.
     */
    static void check();

    static void askUserPermission();

    static bool isTimeToSendStats();

    static qint64 currentUnixTimestamp();

    static bool m_longTimerRunning;

    /**
     * @brief The first time the user opens nqq we don't want to bother him with
     *        the Stats permission dialog. For this reason, we want to defer it
     *        to the second run. This variable is set to true and *stays true*
     *        iff the user never opened nqq before.
     */
    static bool m_isFirstNotepadqqRun;
};

#endif // STATS_H
