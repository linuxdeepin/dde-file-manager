// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewfileoperation.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <QList>
#include <QUrl>
#include <QProcess>
#include <QFileInfo>

Q_DECLARE_METATYPE(bool *)

Q_DECLARE_LOGGING_CATEGORY(logLibFilePreview)
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

PreviewFileOperation::PreviewFileOperation(QObject *parent)
    : QObject(parent)
{
    qCDebug(logLibFilePreview) << "PreviewFileOperation: initialized";
}

bool PreviewFileOperation::openFileHandle(quint64 winID, const QUrl &url)
{
    qCInfo(logLibFilePreview) << "PreviewFileOperation: attempting to open file:" << url.toString() << "from window ID:" << winID;

    if (!url.isValid() || !url.isLocalFile()) {
        qCWarning(logLibFilePreview) << "PreviewFileOperation: invalid URL provided:" << url.toString();
        return false;
    }

    if (!QFileInfo(url.toLocalFile()).exists()) {
        UniversalUtils::notifyMessage(QObject::tr("dde-file-manager"),
                                      tr("Failed to open %1, which may be moved or renamed").arg(url.fileName()));
        return false;
    }

    QList<QUrl> urls { url };
    LocalFileHandler fileHandler;
    bool ok = fileHandler.openFiles(urls);

    if (!ok) {
        GlobalEventType lastEvent = fileHandler.lastEventType();
        if (lastEvent != GlobalEventType::kUnknowType) {
            qCWarning(logLibFilePreview) << "PreviewFileOperation: failed to open file with event type:" << static_cast<int>(lastEvent) << "URL:" << url.toString();
        } else {
            qCInfo(logLibFilePreview) << "PreviewFileOperation: using fallback method - launching file manager for URL:" << url.toString();
            // deal open file with custom dialog
            QStringList args;
            args << "-o" << url.path();
            QString cmd("dde-file-manager");
            bool started = QProcess::startDetached(cmd, args);
            if (started) {
                qCInfo(logLibFilePreview) << "PreviewFileOperation: successfully launched file manager with arguments:" << args;
                ok = true;
            } else {
                qCCritical(logLibFilePreview) << "PreviewFileOperation: failed to start file manager process for URL:" << url.toString();
            }
        }
    } else {
        qCInfo(logLibFilePreview) << "PreviewFileOperation: successfully opened file:" << url.toString();
    }

    return ok;
}
