#ifndef FMSETTING_H
#define FMSETTING_H

#include <QObject>
#include "debugobejct.h"

class FMSetting : public DebugObejct
{
    Q_OBJECT
public:
    explicit FMSetting(QObject *parent = 0);
    ~FMSetting();

signals:

public slots:
};

#endif // FMSETTING_H
