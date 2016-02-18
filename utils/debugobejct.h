#ifndef DEBUGOBEJCT_H
#define DEBUGOBEJCT_H

#include <QObject>
#include <QDebug>

class DebugObejct : public QObject
{

public:
    DebugObejct(QObject *parent = 0);

private:
    friend QDebug operator<<(QDebug dbg, const QObject &obj);
};

QDebug operator<<(QDebug dbg, const QObject &obj);

#endif // DEBUGOBEJCT_H
