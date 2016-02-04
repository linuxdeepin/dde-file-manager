#include "dobject.h"
#include "dobject_p.h"


DObjectPrivate::DObjectPrivate(DObject *qq):
    q_ptr(qq)
{

}

DObjectPrivate::~DObjectPrivate()
{

}

DObject::DObject(DObjectPrivate &dd):
    d_d_ptr(&dd)
{

}

DObject::~DObject()
{

}
