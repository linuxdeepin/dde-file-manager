/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef BURNJOBMANAGER_H
#define BURNJOBMANAGER_H

#include "dfmplugin_burn_global.h"

#include <dfm-burn/dopticaldiscmanager.h>

#include <QObject>

namespace dfmplugin_burn {

class AbstractBurnJob;
class DumpISOImageJob;
class BurnJobManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BurnJobManager)

public:
    struct Config
    {
        QString volName;   // It's empty for the burned ISO image
        int speeds;
        DFMBURN::BurnOptions opts;
    };

public:
    static BurnJobManager *instance();

    void startEraseDisc(const QString &dev);
    void startBurnISOFiles(const QString &dev, const QUrl &stagingUrl, const Config &conf);
    void startBurnISOImage(const QString &dev, const QUrl &imageUrl, const Config &conf);
    void startBurnUDFFiles(const QString &dev, const QUrl &stagingUrl, const Config &conf);
    void startDumpISOImage(const QString &dev, const QUrl &imageUrl);

    void startAuditLogForCopyFromDisc(const QList<QUrl> &srcList, const QList<QUrl> &destList);
    void startAuditLogForBurnFiles(const QVariantMap &info, const QUrl &stagingUrl, bool result);

private:
    void initBurnJobConnect(AbstractBurnJob *job);
    void initDumpJobConnect(DumpISOImageJob *job);
    void deleteStagingDir(const QUrl &url);

private slots:
    void showOpticalJobCompletionDialog(const QString &msg, const QString &icon);
    void showOpticalJobFailureDialog(int type, const QString &err, const QStringList &details);
    void showOpticalDumpISOSuccessDialog(const QUrl &imageUrl);
    void showOpticalDumpISOFailedDialog();

private:
    explicit BurnJobManager(QObject *parent = nullptr);
};

}

#endif   // BURNJOBMANAGER_H
