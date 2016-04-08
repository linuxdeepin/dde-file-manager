#ifndef FMSTATEMANAGER_H
#define FMSTATEMANAGER_H

#include "basemanager.h"

#include <QObject>

class FMStateManager : public QObject, public BaseManager
{
    Q_OBJECT
public:
    explicit FMStateManager(QObject *parent = 0);
    ~FMStateManager();

signals:

public slots:
};

#endif // FMSTATEMANAGER_H
