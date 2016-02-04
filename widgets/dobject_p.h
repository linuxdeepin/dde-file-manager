#ifndef DOBJECT_P_H
#define DOBJECT_P_H

#include "dobject.h"

class DObjectPrivate
{
public:
    virtual ~DObjectPrivate();

protected:
    DObjectPrivate(DObject *qq);

    DObject *q_ptr;

    Q_DECLARE_PUBLIC(DObject)
};

#endif // DOBJECT_P_H

