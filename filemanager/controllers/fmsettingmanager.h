#ifndef FMSETTINGMANAGER_H
#define FMSETTINGMANAGER_H

#include <QObject>
#include "basemanager.h"

class FMSettingManager : public BaseManager
{
    Q_OBJECT
public:
    explicit FMSettingManager(QObject *parent = 0);
    ~FMSettingManager();

signals:

public slots:
};

#endif // FMSETTINGMANAGER_H
