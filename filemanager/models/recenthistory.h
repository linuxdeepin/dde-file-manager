#ifndef RECENTHISTORY_H
#define RECENTHISTORY_H

#include <QObject>
#include "utils/debugobejct.h"

class RecentHistory : public DebugObejct
{
    Q_OBJECT
public:
    explicit RecentHistory(QObject *parent = 0);
    ~RecentHistory();
signals:

public slots:
};

#endif // RECENTHISTORY_H
