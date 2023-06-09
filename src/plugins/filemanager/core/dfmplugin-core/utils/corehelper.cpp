// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "corehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/event/event.h>

#include <dfm-framework/lifecycle/lifecycle.h>

#include <QDir>
#include <QProcess>

Q_DECLARE_METATYPE(QList<QUrl> *)

DPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

CoreHelper &CoreHelper::instance()
{
    static CoreHelper ins;
    return ins;
}

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
    FMWindowsIns.resetPreviousActivedWindowId();

    auto window { FMWindowsIns.createWindow(url, opt.isValid() ? opt.toBool() : true) };
    if (!window) {
        qWarning() << "Create window failed for: " << url;
        return;
    }

    FMWindowsIns.showWindow(window);
}

void CoreHelper::cacheDefaultWindow()
{
    auto window { FMWindowsIns.createWindow({}) };
    if (!window) {
        qWarning() << "cache window failed";
        return;
    }
    window->removeEventFilter(this);
    // cache all UI components
    QMetaObject::invokeMethod(window, "aboutToOpen", Qt::DirectConnection);
}

bool CoreHelper::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(qApp->applicationName() == "dde-file-manager");

    // Purpose does not filter any events
    constexpr bool ret { false };

    // The purpose is to initialize the UI,
    // so no attention is paid to other events
    auto type { event->type() };
    if (type != QEvent::Paint && type != QEvent::Show)
        return ret;

    FileManagerWindow *window = qobject_cast<FileManagerWindow *>(watched);
    if (!window)
        return ret;

    // for bug-203703:
    // When hot-launching or opening a new window(not first),
    // we need all components to be displayed at the same time
    // when the window is showed
    if (type == QEvent::Show) {
        int windowCount { FMWindowsIns.windowIdList().size() };
        int lazyCount { DPF_NAMESPACE::LifeCycle::lazyLoadList().size() };
        if (windowCount > 1 || lazyCount == 0) {
            qDebug("Show full window, win count %d, lazy count %d", windowCount, lazyCount);
            window->removeEventFilter(this);
            QMetaObject::invokeMethod(window, "aboutToOpen", Qt::DirectConnection);
        }
        return ret;
    }

    if (type == QEvent::Paint) {
        qDebug() << "Show empty window";
        window->removeEventFilter(this);
        QMetaObject::invokeMethod(window, "aboutToOpen", Qt::QueuedConnection);
        return ret;
    }

    return ret;
}

CoreHelper::CoreHelper(QObject *parent)
    : QObject(parent)
{
    // createWindow may return an existing window, which does not need to be processed again aboutToOpen
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this, [this](quint64 id) {
        auto window { FMWindowsIns.findWindowById(id) };
        if (window)
            window->installEventFilter(this);
    });
}
