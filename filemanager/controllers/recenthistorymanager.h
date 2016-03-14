#ifndef RECENTHISTORYMANAGER_H
#define RECENTHISTORYMANAGER_H

#include <QObject>
#include "basemanager.h"

class RecentHistoryManager : public BaseManager
{
    Q_OBJECT
public:
    explicit RecentHistoryManager(QObject *parent = 0);
    ~RecentHistoryManager();

signals:

public slots:
};

#endif // RECENTHISTORYMANAGER_H
