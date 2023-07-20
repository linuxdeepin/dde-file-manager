// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reportlogworker.h"
#include "datas/blockmountreportdata.h"
#include "datas/smbreportdata.h"
#include "datas/vaultreportdata.h"
#include "datas/searchreportdata.h"
#include "datas/sidebarreportdata.h"
#include "datas/filemenureportdata.h"
#include "datas/appstartupreportdata.h"
#include "datas/reportdatainterface.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/private/devicehelper.h>

#include <dfm-mount/dblockdevice.h>

#include <QUrl>
#include <QDebug>
#include <QApplication>
#include <QJsonDocument>

DPUTILS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

ReportLogWorker::ReportLogWorker(QObject *parent)
    : QObject(parent)
{

}

ReportLogWorker::~ReportLogWorker()
{
    qDeleteAll(logDataObj.begin(), logDataObj.end());
    logDataObj.clear();

    if (logLibrary.isLoaded())
        logLibrary.unload();
}

bool ReportLogWorker::init()
{
    QList<ReportDataInterface *> datas {
        new BlockMountReportData,
        new SmbReportData,
        new SidebarReportData,
        new SearchReportData,
        new VaultReportData,
        new FileMenuReportData,
        new AppStartupReportData
    };

    std::for_each(datas.cbegin(), datas.cend(), [this](ReportDataInterface *dat) { registerLogData(dat->type(), dat); });

    logLibrary.setFileName("deepin-event-log");
    if (!logLibrary.load()) {
        qWarning() << "Report log plugin load log library failed!";
        return false;
    } else {
        qInfo() << "Report log plugin load log library success.";
    }

    initEventLogFunc = reinterpret_cast<InitEventLog>(logLibrary.resolve("Initialize"));
    writeEventLogFunc = reinterpret_cast<WriteEventLog>(logLibrary.resolve("WriteEventLog"));

    if (!initEventLogFunc || !writeEventLogFunc) {
        qWarning() << "Log library init failed!";
        return false;
    }

    if (!initEventLogFunc(QApplication::applicationName().toStdString(), false)) {
        qWarning() << "Log library init function call failed!";
        return false;
    }

    return true;
}

void ReportLogWorker::commitLog(const QString &type, const QVariantMap &args)
{
    ReportDataInterface *interface = logDataObj.value(type, nullptr);
    if (!interface) {
        qInfo() << "Error: Log data object is not registed.";
        return;
    }
    QJsonObject jsonObject = interface->prepareData(args);

    const QStringList &keys = commonData.keys();
    foreach (const QString &key, keys) {
        jsonObject.insert(key, commonData.value(key));   //add common data for each log commit
    }

    commit(jsonObject.toVariantHash());
}

void ReportLogWorker::handleMenuData(const QString &name, const QList<QUrl> &urlList)
{
    QVariantMap data {};
    data.insert("item_name", name);

    QString location("");
    QStringList types {};

    if (urlList.count() > 0) {
        location = "File";

        for (auto url : urlList) {
            auto info = InfoFactory::create<FileInfo>(url);
            if (info) {
                QString type = info->nameOf(NameInfoType::kMimeTypeName);
                if (!types.contains(type))
                    types << type;
            }
        }

    } else {
        location = "Workspace";
    }

    data.insert("location", location);
    data.insert("type", types);

    commitLog("FileMenu", data);
}

void ReportLogWorker::handleBlockMountData(const QString &id, bool result)
{
    if (id.isEmpty()) {
        qWarning() << "Can't report empty devices' operation";
        return;
    }

    QVariantMap rec {};
    if (result) {
        BlockDevAutoPtr device = DeviceHelper::createBlockDevice(id);

        if (device.isNull()) {
            qWarning() << "Can't report unexist devices' operation";
            return;
        }

        rec.insert("fileSystem", device->fileSystem());
        rec.insert("standardSize", device->sizeTotal());
        rec.insert("mountResult", result);
    } else {
        rec.insert("fileSystem", "unknown");
        rec.insert("standardSize", 0);
        rec.insert("mountResult", result);
    }

    commitLog("BlockMount", rec);
}

void ReportLogWorker::handleMountNetworkResult(bool ret, dfmmount::DeviceError err, const QString &msg)
{
    using namespace dfmmount;

    QVariantMap data;
    data.insert("result", ret);

    if (!ret) {
        switch (err) {
        case DeviceError::kUserErrorUserCancelled:
            data.insert("errorId", SmbReportData::kUserCancelError);
            data.insert("errorSysMsg", msg);
            data.insert("errorUiMsg", "User cancel mount dialog.");
            break;
        case DeviceError::kUDisksErrorNotMounted:
        case DeviceError::kGIOErrorNotMounted:
        case DeviceError::kUserErrorNotMounted:
            data.insert("errorId", SmbReportData::kNotMount);
            data.insert("errorSysMsg", msg);
            data.insert("errorUiMsg", msg);
            break;
        default:
            data.insert("errorId", SmbReportData::kMountError);
            data.insert("errorSysMsg", msg);
            data.insert("errorUiMsg", msg);
            break;
        }
    }

    commitLog("Smb", data);
}

bool ReportLogWorker::registerLogData(const QString &type, ReportDataInterface *dataObj)
{
    if (logDataObj.contains(type))
        return false;

    logDataObj.insert(type, dataObj);
    return true;
}

void ReportLogWorker::commit(const QVariant &args)
{
    if (args.isNull() || !args.isValid())
        return;
    const QJsonObject &dataObj = QJsonObject::fromVariantHash(args.toHash());
    QJsonDocument doc(dataObj);
    const QByteArray &sendData = doc.toJson(QJsonDocument::Compact);
    writeEventLogFunc(sendData.data());
}
