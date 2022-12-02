// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blockmountreportdata.h"
#include "reportlog/rlog/rlog.h"

#include <QVariantMap>
#include <QDebug>
#include <QDateTime>

//#include <ddiskmanager.h>
#include <dblockdevice.h>

static constexpr int kBlockMountTid { 1000500004 };
static constexpr char kReportType[] { "BlockMount" };

QString BlockMountReportData::type() const
{
    return kReportType;
}

QJsonObject BlockMountReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap temArgs = args;
    temArgs.insert("tid", kBlockMountTid);
    temArgs.insert("opTime", QDateTime::currentDateTime().toTime_t());
    return QJsonObject::fromVariantMap(temArgs);
}

/*!
 * \brief BlockMountReportData::report
 * \param datas: { {"dev", "/dev/sdb1"}, {"result", true} }
 */
void BlockMountReportData::report(const QVariantMap &datas)
{
    /*
    const auto &dev = datas.value("dev").toString();
    if (dev.isEmpty()) {
        qDebug() << "cannot report empty devices' operation";
        return;
    }
    bool ok = datas.value("result").toBool();

    const auto &blks = DDiskManager::resolveDeviceNode(dev, {});
    if (blks.count() == 0) {
        qWarning() << "rlog: cannot resolveDevice for " << dev;
        return;
    }

    const QString &blkObjPath = blks.first();
    QScopedPointer<DBlockDevice> blk { DDiskManager::createBlockDevice(blkObjPath, nullptr) };
    if (!blk) {
        qWarning() << "rlog: cannot create block object";
        return;
    }

    QVariantMap rec {
        { "fileSystem", blk->idType() },
        { "standardSize", blk->size() },
        { "mountResult", ok }
    };

    qInfo() << "rlog: mount result: " << rec;
    RLog::instance()->commit(ReportType, rec);
    */
}
