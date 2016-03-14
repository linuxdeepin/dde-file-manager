#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include "basemanager.h"

class BookMarkManager : public BaseManager
{
    Q_OBJECT
public:
    explicit BookMarkManager(QObject *parent = 0);
    ~BookMarkManager();

signals:

public slots:
};

#endif // BOOKMARKMANAGER_H
