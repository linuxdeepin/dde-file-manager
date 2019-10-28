#include "define.h"

#include <QMutex>

#include "dialogs/dialogmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "deviceinfo/udisklistener.h"
#include "disomaster.h"

Q_GLOBAL_STATIC(DialogManager, gsDialogManager)
Q_GLOBAL_STATIC(GvfsMountManager, gsGvfsMountManager)
Q_GLOBAL_STATIC(UDiskListener, gsUDiskListener)
Q_GLOBAL_STATIC(DISOMasterNS::DISOMaster, gsDISOMaster)

static QMutex odmutex;

DialogManager *getDialogManager(bool doConstruct)
{
    if (doConstruct) {
        if (!gsDialogManager.exists() && qApp) {
            gsDialogManager->moveToThread(qApp->thread());
        }

        return gsDialogManager();
    }
    return gsDialogManager.exists() ? gsDialogManager() : nullptr;
}

GvfsMountManager *getGvfsMountManager(bool doConstruct)
{
    if (doConstruct) {
        if (!gsGvfsMountManager.exists() && qApp) {
            gsGvfsMountManager->moveToThread(qApp->thread());
        }

        return gsGvfsMountManager();
    }
    return gsGvfsMountManager.exists() ? gsGvfsMountManager() : nullptr;
}

UDiskListener *getUDiskListener(bool doConstruct)
{
    if (doConstruct) {
        if (!gsUDiskListener.exists() && qApp) {
            gsUDiskListener->moveToThread(qApp->thread());
        }

        return gsUDiskListener();
    }
    return gsUDiskListener.exists() ? gsUDiskListener() : nullptr;
}

DISOMasterNS::DISOMaster *getDISOMaster(bool doConstruct)
{
    if (doConstruct) {
        if (!gsDISOMaster.exists() && qApp) {
            gsDISOMaster->moveToThread(qApp->thread());
        }

        return gsDISOMaster();
    }
    return gsDISOMaster.exists() ? gsDISOMaster() : nullptr;
}

QMutex *getOpticalDriveMutex()
{
    return &odmutex;
}
