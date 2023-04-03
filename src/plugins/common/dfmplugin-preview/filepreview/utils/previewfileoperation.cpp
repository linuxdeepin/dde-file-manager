// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewfileoperation.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

#include <QList>

#include <QUrl>

Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;
PreviewFileOperation::PreviewFileOperation(QObject *parent)
    : QObject(parent)
{
}

bool PreviewFileOperation::openFileHandle(quint64 winID, const QUrl &url)
{
    QList<QUrl> urls;
    urls << url;
    bool ok = true;
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, winID, urls, &ok);

    return ok;
}
