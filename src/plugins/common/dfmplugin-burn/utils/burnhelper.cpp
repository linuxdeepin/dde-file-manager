// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burnhelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DDialog>

#include <QObject>
#include <QDebug>
#include <QCoreApplication>
#include <QPushButton>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QApplication>

using namespace dfmplugin_burn;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

// TODO(zhangs): replace it
#define BURN_SEG_ONDISC "disc_files"
#define BURN_SEG_STAGING "staging_files"

int BurnHelper::showOpticalBlankConfirmationDialog()
{
    QString EraseDisk = QObject::tr("Are you sure you want to erase all data on the disc?");
    QStringList buttonTexts;
    buttonTexts.append(QObject::tr("Cancel", "button"));
    buttonTexts.append(QObject::tr("Erase", "button"));

    DDialog d(qApp->activeWindow());

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    d.setTitle(EraseDisk);
    d.setMessage(QObject::tr("This action cannot be undone"));
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(64, 64));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();
    int code = d.exec();

    return code;
}

int BurnHelper::showOpticalImageOpSelectionDialog()
{
    QString EraseDisk = QObject::tr("How do you want to use this disc?");
    QStringList buttonTexts;
    buttonTexts.append(QObject::tr("Cancel", "button"));
    buttonTexts.append(QObject::tr("Burn image", "button"));
    buttonTexts.append(QObject::tr("Burn files", "button"));

    DDialog d(qApp->activeWindow());

    if (!d.parentWidget())
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);

    d.setTitle(EraseDisk);
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(64, 64));
    d.addButton(buttonTexts[0], false, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonNormal);
    d.addButton(buttonTexts[2], true, DDialog::ButtonRecommend);
    d.setDefaultButton(2);
    d.getButton(2)->setFocus();
    d.moveToCenter();

    int code = d.exec();
    return code;
}

// TODO(zhangs): replace it
QUrl BurnHelper::localStagingFile(QString dev)
{
#define DISCBURN_STAGING "discburn"
    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)   // ~/.cache
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"   // ~/.cache/deepin/discburn/
                               + dev.replace('/', '_'));
}

// TODO(zhangs): repalce it
QUrl BurnHelper::localStagingFile(const QUrl &dest)
{
    if (burnDestDevice(dest).length() == 0)
        return {};

    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"
                               + burnDestDevice(dest).replace('/', '_')
                               + burnFilePath(dest));
}

QUrl BurnHelper::fromBurnFile(const QString &dev)
{
    QString path { dev + "/" BURN_SEG_STAGING "/" };
    QUrl ret;
    ret.setScheme(Global::Scheme::kBurn);
    ret.setPath(path);
    return ret;
}

QString BurnHelper::parseXorrisoErrorMessage(const QStringList &msg)
{
    QRegularExpression ovrex("While grafting '(.*)'");
    for (auto &msgs : msg) {
        auto ovrxm = ovrex.match(msgs);
        if (msgs.contains("file object exists and may not be overwritten") && ovrxm.hasMatch()) {
            return QObject::tr("%1 is a duplicate file.").arg(ovrxm.captured(1));
        }
        if (msgs.contains(QRegularExpression("Image size [0-9s]* exceeds free space on media [0-9s]*"))) {
            return QObject::tr("Insufficient disc space.");
        }
        if (msgs.contains("Lost connection to drive")) {
            return QObject::tr("Lost connection to drive.");
        }
        if (msgs.contains("servo failure")) {
            return QObject::tr("The CD/DVD drive is not ready. Try another disc.");
        }
        if (msgs.contains("Device or resource busy")) {
            return QObject::tr("The CD/DVD drive is busy. Exit the program using the drive, and insert the drive again.");
        }
        if (msgs.contains("-volid: Text too long")) {
            // something is wrong if the following return statement is reached.
            return QObject::tr("Invalid volume name");
        }
    }
    return QObject::tr("Unknown error");
}

// TODO(zhangs): repalce it
QString BurnHelper::burnDestDevice(const QUrl &url)
{
    static QRegularExpression rxp { "^(.*?)/(" BURN_SEG_ONDISC "|" BURN_SEG_STAGING ")(.*)$" };

    QRegularExpressionMatch m;
    if (url.scheme() != Global::Scheme::kBurn || !url.path().contains(rxp, &m))
        return {};
    return m.captured(1);
}

// TODO(zhangs): repalce it
QString BurnHelper::burnFilePath(const QUrl &url)
{
    static QRegularExpression rxp { "^(.*?)/(" BURN_SEG_ONDISC "|" BURN_SEG_STAGING ")(.*)$" };

    QRegularExpressionMatch m;
    if (url.scheme() != Global::Scheme::kBurn || !url.path().contains(rxp, &m))
        return {};
    return m.captured(3);
}

QList<QVariantMap> BurnHelper::discDataGroup()
{
    using namespace GlobalServerDefines;
    QList<QVariantMap> discDatas;
    QStringList discIdList;
    auto &&idList = DevProxyMng->getAllBlockIds({});
    for (const auto &id : idList) {
        if (id.startsWith("/org/freedesktop/UDisks2/block_devices/sr")) {
            auto &&data = DevProxyMng->queryBlockInfo(id);
            bool isOptical { data.value(DeviceProperty::kOptical).toBool() };
            bool isOpticalDrive { data.value(DeviceProperty::kOpticalDrive).toBool() };
            if (isOptical && isOpticalDrive)
                discDatas.push_back(data);
        }
    }

    return discDatas;
}

void BurnHelper::updateBurningStateToPersistence(const QString &id, const QString &dev, bool working)
{
    QVariantMap info;
    info[Persistence::kIdKey] = id;
    info[Persistence::kWoringKey] = working;

    if (Application::dataPersistence()) {
        Application::dataPersistence()->setValue(Persistence::kBurnStateGroup, dev, info);
        Application::dataPersistence()->sync();
    }
}

void BurnHelper::mapStagingFilesPath(const QList<QUrl> &srcList, const QList<QUrl> &targetList)
{
    if (!srcList.isEmpty() && (srcList.size() != targetList.size())) {
        fmWarning() << "Src url size != targt url size";
        return;
    }

    if (targetList.isEmpty() || !Application::dataPersistence()) {
        fmWarning() << "Target url is empty";
        return;
    }

    QString firsDestPath { targetList[0].toLocalFile() };
    static QRegularExpression reg("_dev_sr[0-9]*");
    QRegularExpressionMatch match;
    if (!firsDestPath.contains(reg, &match)) {
        fmWarning() << "Cannot map _dev_sr[0-9]";
        return;
    }
    QString dev { match.captured().replace("_", "/") };
    if (dev.isEmpty()) {
        fmWarning() << "Empty dev";
        return;
    }

    QVariantMap map { Application::dataPersistence()->value("StagingMap", dev).toMap() };
    for (int i = 0; i != srcList.size(); ++i)
        map[targetList.at(i).toLocalFile()] = srcList.at(i).path();

    Application::dataPersistence()->setValue("StagingMap", dev, map);
    Application::dataPersistence()->sync();
}

bool BurnHelper::isBurnEnabled()
{
    const auto &&ret = DConfigManager::instance()->value("org.deepin.dde.file-manager.burn", "burnEnable");
    return ret.isValid() ? ret.toBool() : true;
}

bool BurnHelper::burnIsOnLocalStaging(const QUrl &url)
{
    if (!url.path().contains("/.cache/deepin/discburn/_dev_"))
        return false;

    static QRegularExpression reg("/_dev_sr[0-9]*/");
    QRegularExpressionMatch match = reg.match(url.path());
    if (match.hasMatch())
        return true;

    return false;
}

QFileInfoList BurnHelper::localFileInfoList(const QString &path)
{
    QDir dir(path);
    if (!dir.exists() || dir.isEmpty())
        return {};

    return dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
}

QFileInfoList BurnHelper::localFileInfoListRecursive(const QString &path, QDir::Filters filters)
{
    QDir dir(path);
    if (!dir.exists() || dir.isEmpty())
        return {};

    QFileInfoList fileList { dir.entryInfoList(filters) };
    const QFileInfoList &folderList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &info : folderList) {
        const QFileInfoList &childFileList = localFileInfoListRecursive(info.absoluteFilePath(), filters);
        fileList.append(childFileList);
    }

    return fileList;
}
