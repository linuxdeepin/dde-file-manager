// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void startAuditLogForEraseDisc(const QVariantMap &info, bool result);

    void startPutFilesToDisc(const QString &dev, const QList<QUrl> &urls);
    void startRemoveFilesFromDisc(const QString &dev, const QList<QUrl> &urls);
    void startRenameFileFromDisc(const QString &dev, const QUrl &src, const QUrl &dest);

private:
    void initBurnJobConnect(AbstractBurnJob *job);
    void initDumpJobConnect(DumpISOImageJob *job);
    bool deleteStagingDir(const QUrl &url);

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
