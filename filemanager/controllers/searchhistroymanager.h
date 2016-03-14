#ifndef SEARCHHISTROYMANAGER_H
#define SEARCHHISTROYMANAGER_H

#include <QObject>
#include "basemanager.h"

class SearchHistroyManager : public BaseManager
{
    Q_OBJECT
public:
    explicit SearchHistroyManager(QObject *parent = 0);
    ~SearchHistroyManager();

signals:

public slots:
};

#endif // SEARCHHISTROYMANAGER_H
