#include "dfmglobal.h"

QString DFMGlobal::organizationName()
{
    return QMAKE_ORGANIZATION_NAME;
}

QString DFMGlobal::applicationName()
{
    return QMAKE_TARGET;
}

QString DFMGlobal::applicationVersion()
{
    return QMAKE_VERSION;
}

DFMGlobal::DFMGlobal()
{

}
