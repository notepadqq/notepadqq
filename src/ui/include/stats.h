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

    static bool m_longTimerRunning;
};

#endif // STATS_H
