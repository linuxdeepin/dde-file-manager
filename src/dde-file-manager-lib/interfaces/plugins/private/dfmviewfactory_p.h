#ifndef DFMVIEWFACTORY_P_H
#define DFMVIEWFACTORY_P_H

#include "dfmglobal.h"

#include <QMap>

DFM_BEGIN_NAMESPACE

class DFMBaseView;
class DFMViewFactoryPrivate
{
public:
    static QMap<const DFMBaseView*, int> viewToLoaderIndex;
};

DFM_END_NAMESPACE

#endif // DFMVIEWFACTORY_P_H
