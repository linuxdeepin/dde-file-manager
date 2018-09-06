#include "define.h"

#include "dialogs/dialogmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "deviceinfo/udisklistener.h"

Q_GLOBAL_STATIC(DialogManager, gsDialogManager)
Q_GLOBAL_STATIC(GvfsMountManager, gsGvfsMountManager)
Q_GLOBAL_STATIC(UDiskListener, gsUDiskListener)

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
