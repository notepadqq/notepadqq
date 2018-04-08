#ifndef STATS_H
#define STATS_H

#include <QJsonObject>

class Stats
{
public:
    static void init();

private:

    static void send(const QJsonObject &data);
};

#endif // STATS_H
