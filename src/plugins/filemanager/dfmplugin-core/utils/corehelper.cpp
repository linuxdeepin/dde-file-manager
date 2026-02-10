// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "corehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

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
        fmWarning() << "Invalid window id: " << windowId;
        return;
    }

    fmInfo() << "cd to " << url;
    window->cd(url);

    if (UniversalUtils::urlEquals(url, FileUtils::trashRootUrl())) {
        window->setWindowTitle(QCoreApplication::translate("PathManager", "Trash"));
        return;
    }

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

void CoreHelper::openWindow(const QUrl &url, const QVariant &opt)
{
    // performance:
    // if a window is opened that is cached, then just activate it
    bool openNew { opt.isValid() ? opt.toBool() : true };
    auto oldWindow { defaultWindow() };
    if (openNew && oldWindow)
        openNew = false;

    FMWindowsIns.resetPreviousActivedWindowId();
    FileManagerWindow *window { openNew ? createNewWindow(url)
                                        : findExistsWindow(url) };
    if (!window) {
        fmCritical() << "Create window failed for: " << url;
        return;
    }

    FMWindowsIns.showWindow(window);
}

void CoreHelper::cacheDefaultWindow()
{
    auto window { FMWindowsIns.createWindow({}) };
    if (!window) {
        fmWarning() << "cache window failed";
        return;
    }
    window->removeEventFilter(this);
    // cache all UI components
    QMetaObject::invokeMethod(window, "aboutToOpen", Qt::DirectConnection);
}

void CoreHelper::loadPlugin(const QString &name)
{
    fmInfo() << "About to load plugin:" << name;
    auto plugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj(name) };
    if (plugin) {
        auto result { DPF_NAMESPACE::LifeCycle::loadPlugin(plugin) };
        fmInfo() << "Load result: " << result
                 << "State: " << plugin->pluginState() << "for plugin:" << name;
    } else {
        fmWarning() << "Plugin meta object not found for:" << name;
    }
}

FileManagerWindow *CoreHelper::defaultWindow()
{
    const auto &idList { FMWindowsIns.windowIdList() };
    if (idList.size() == 1) {
        auto window { FMWindowsIns.findWindowById(idList.first()) };
        if (window && window->isHidden())
            return window;
    }

    fmDebug() << "No default window available";
    return {};
}

FileManagerWindow *CoreHelper::createNewWindow(const QUrl &url)
{
    fmInfo() << "Create new window for: " << url;
    return FMWindowsIns.createWindow(url, true);
}

FileManagerWindow *CoreHelper::findExistsWindow(const QUrl &url)
{
    auto window { FMWindowsIns.createWindow(url, false) };

    if (window) {
        fmInfo() << "Find exists window for: " << url << ",for window:" << window->winId();
        return window;
    }

    fmWarning() << "Cannot find exists window for:" << url;
    auto oldWindow { defaultWindow() };
    if (oldWindow) {
        fmInfo() << "Close cached default window with ID:" << oldWindow->winId();
        oldWindow->setProperty("_dfm_isDefaultWindow", true);
        oldWindow->close();
    }
    return createNewWindow(url);
}

bool CoreHelper::eventFilter(QObject *watched, QEvent *event)
{
    auto appName = qApp->applicationName();
    Q_ASSERT(appName == "dde-file-manager" || appName == "dde-file-dialog" || appName == "dde-select-dialog-x11" || appName == "dde-select-dialog-wayland");

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
            fmDebug("Show full window, win count %d, lazy count %d", windowCount, lazyCount);
            window->removeEventFilter(this);
            QMetaObject::invokeMethod(window, "aboutToOpen", Qt::DirectConnection);
        }
        return ret;
    }

    if (type == QEvent::Paint) {
        fmDebug() << "Show empty window for ID:" << window->winId();
        window->removeEventFilter(this);
        QMetaObject::invokeMethod(window, "aboutToOpen", Qt::QueuedConnection);
        return ret;
    }

    return ret;
}

CoreHelper::CoreHelper(QObject *parent)
    : QObject(parent)
{
}
