// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "corehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/event/event.h>

#include <QDir>
#include <QProcess>

Q_DECLARE_METATYPE(QList<QUrl> *)

DPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void CoreHelper::cd(quint64 windowId, const QUrl &url)
{
    Q_ASSERT(url.isValid());
    auto window = FMWindowsIns.findWindowById(windowId);

    if (!window) {
        qWarning() << "Invalid window id: " << windowId;
        return;
    }

    qInfo() << "cd to " << url;
    window->cd(url);

    QUrl titleUrl { url };
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ titleUrl }, &urls);

    if (ok && !urls.isEmpty())
        titleUrl = urls.first();

    auto fileInfo = InfoFactory::create<FileInfo>(titleUrl);
    if (fileInfo) {
        QUrl url { fileInfo->urlOf(UrlInfoType::kUrl) };
        window->setWindowTitle(fileInfo->displayOf(DisPlayInfoType::kFileDisplayName));
    } else {
        window->setWindowTitle({});
    }
}

void CoreHelper::openNewWindow(const QUrl &url, const QVariant &opt)
{
    FMWindowsIns.showWindow(url, opt.isValid() ? opt.toBool() : true);
}
