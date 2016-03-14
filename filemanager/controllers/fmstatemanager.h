#ifndef FMSTATEMANAGER_H
#define FMSTATEMANAGER_H

#include <QObject>
#include "basemanager.h"

class FMStateManager : public BaseManager
{
    Q_OBJECT
public:
    explicit FMStateManager(QObject *parent = 0);
    ~FMStateManager();

signals:

public slots:
};

#endif // FMSTATEMANAGER_H
