// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewfileoperation.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <QList>
#include <QUrl>
#include <QProcess>

Q_DECLARE_METATYPE(bool *)

Q_DECLARE_LOGGING_CATEGORY(logLibFilePreview)
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;
PreviewFileOperation::PreviewFileOperation(QObject *parent)
    : QObject(parent)
{
}

bool PreviewFileOperation::openFileHandle(quint64 winID, const QUrl &url)
{
    QList<QUrl> urls { url };
    LocalFileHandler fileHandler;
    bool ok = fileHandler.openFiles(urls);
    if (!ok) {
        GlobalEventType lastEvent = fileHandler.lastEventType();
        if (lastEvent != GlobalEventType::kUnknowType) {
            qCWarning(logLibFilePreview) << "Open file failed with unknown event type";
        } else {
            // deal open file with custom dialog
            QStringList args;
            args << "-o" << url.path();
            QString cmd("dde-file-manager");
            QProcess::startDetached(cmd, args);
            ok = true;
        }
    }

    return ok;
}
