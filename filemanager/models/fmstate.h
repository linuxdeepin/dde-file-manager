#ifndef FMSTATE_H
#define FMSTATE_H

#include <QObject>
#include "utils/debugobejct.h"

class FMState : public DebugObejct
{
    Q_OBJECT
public:
    explicit FMState(QObject *parent = 0);
    ~FMState();

signals:

public slots:
};

#endif // FMSTATE_H
