#include "dfmsidebaropticalitemhandler.h"

#include "app/define.h"

#include "singleton.h"
#include "app/filesignalmanager.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "interfaces/dfmsidebaritem.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "disomaster.h"

DFM_BEGIN_NAMESPACE

DFMSideBarItem *DFMSideBarOpticalItemHandler::createItem(const DUrl &url)
{
    QString displayName = getDisplayNameForDevice(url.path());
    QString iconName("drive-harddisk-symbolic");

    DFMSideBarItem * item = new DFMSideBarItem(QIcon::fromTheme(iconName), displayName, url);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    item->setData(SIDEBAR_ID_OPTICAL, DFMSideBarItem::ItemUseRegisteredHandlerRole);

    return item;
}

DFMSideBarOpticalItemHandler::DFMSideBarOpticalItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

void DFMSideBarOpticalItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    if (ISOMaster->currentDevice() == item->url().path()) {
        return;
    }

    DUrl burnurl = DUrl::fromBurnFile(QString("%1/" BURN_SEG_ONDISC "/").arg(item->url().path()));

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    wnd->cd(burnurl); // don't `setChecked` here, wait for a signal.
}

QMenu *DFMSideBarOpticalItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    QString udiskspath(item->url().path());
    udiskspath.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
    QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspath));
    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
    DUrl burnurl = DUrl::fromBurnFile(QString("%1/" BURN_SEG_ONDISC "/").arg(item->url().path()));
    DUrl devIdUrl = DUrl(blk->mountPoints().size() ? blk->mountPoints().front() : "");
    devIdUrl.setQuery(blk->device());

    QMenu *ret = new QMenu();

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());

    ret->addAction(QObject::tr("Open in new window"), [burnurl]() {
        WindowManager::instance()->showNewWindow(burnurl, true);
    });

    ret->addAction(QObject::tr("Open in new tab"), [wnd, burnurl]() {
        wnd->openNewTab(burnurl);
    })->setDisabled(shouldDisable);

    ret->addSeparator();
        if (drv->mediaAvailable() && !drv->opticalBlank() && blk->mountPoints().empty()) {
        ret->addAction(QObject::tr("Mount"), [udiskspath] {
            QScopedPointer<DBlockDevice> b(DDiskManager::createBlockDevice(udiskspath));
            b->mount({});
        });
    }

    if (drv->mediaAvailable()) {
        ret->addAction(QObject::tr("Unmount"), [udiskspath] {
            QScopedPointer<DBlockDevice> b(DDiskManager::createBlockDevice(udiskspath));
            QScopedPointer<DDiskDevice> d(DDiskManager::createDiskDevice(b->drive()));
            if (d->ejectable()) {
                b->unmount({});
                d->eject({});
            }
        });
    }

    if (drv->media().contains("_rw")) {
        ret->addAction(QObject::tr("Erase"), [this, devIdUrl]() {
            AppController::instance()->actionOpticalBlank(dMakeEventPointer<DFMUrlBaseEvent>(this, devIdUrl));
        });
    }
    ret->addSeparator();

    ret->addAction(QObject::tr("Disk info"), [devIdUrl]() {
        Singleton<FileSignalManager>::instance()->requestShowPropertyDialog(DFMUrlListBaseEvent(nullptr, {devIdUrl}));
    });

    return ret;
}

QString DFMSideBarOpticalItemHandler::getDisplayNameForDevice(const QString &dev)
{
    QString udiskspath(dev);
    udiskspath.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
    QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspath));
    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
    QHash<QString, QString> mediamap = {
        {"optical_cd",             "aCD-ROM"},
        {"optical_cd_r",           "bCD-R"},
        {"optical_cd_rw",          "cCD-RW"},
        {"optical_dvd",            "dDVD-ROM"},
        {"optical_dvd_r",          "eDVD-R"},
        {"optical_dvd_rw",         "fDVD-RW"},
        {"optical_dvd_ram",        "gDVD-RAM"},
        {"optical_dvd_plus_r",     "hDVD+R"},
        {"optical_dvd_plus_rw",    "iDVD+RW"},
        {"optical_dvd_plus_r_dl",  "jDVD+R/DL"},
        {"optical_dvd_plus_rw_dl", "kDVD+RW/DL"},
        {"optical_bd",             "lBluray"},
        {"optical_bd_r",           "mBluray-R"},
        {"optical_bd_rw",          "nBluray-RE"},
        {"optical_hddvd",          "oHD-DVD"},
        {"optical_hddvd_r",        "pHD-DVD-R"},
        {"optical_hddvd_rw",       "qHD-DVD-RW"},
        {"optical_mo",             "rMO"},
        {"optical_mrw",            "s"},
        {"optical_mrw_w",          "t"}
    };
    if (drv->mediaAvailable()) {
        if (blk->idLabel().length()) {
            return blk->idLabel();
        } else {
            if (drv->opticalBlank()) {
                return tr("Blank %1 disc").arg(mediamap[drv->media()]);
            } else {
                return tr("%1 disc").arg(mediamap[drv->media()]);
            }
        }
    } else {
        QStringList mc = drv->mediaCompatibility();
        std::sort(mc.begin(), mc.end(), [&mediamap](const QString &a, const QString &b) {
            return mediamap[a].at(0) < mediamap[b].at(0);
        });
        while (mc.size() && mc.back().startsWith("optical_mrw")) {
            mc.pop_back();
        }
        Q_ASSERT(!mc.empty());
        return tr("%1 drive").arg(mediamap[mc.back()].mid(1));
    }
}

DFM_END_NAMESPACE
