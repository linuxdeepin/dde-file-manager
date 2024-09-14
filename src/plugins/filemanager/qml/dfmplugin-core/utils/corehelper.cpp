// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "corehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-gui/panel.h>

#include <dfm-framework/event/event.h>
#include <dfm-framework/lifecycle/lifecycle.h>

#include <QDir>
#include <QProcess>
#include <QQuickWindow>

Q_DECLARE_METATYPE(QList<QUrl> *)

DPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGUI_USE_NAMESPACE

static constexpr char kDefaultPlugin[] { "dfmplugin-core" };
static constexpr char kDefaultId[] { "filewindow" };

CoreHelper &CoreHelper::instance()
{
    static CoreHelper ins;
    return ins;
}

void CoreHelper::cd(quint64 windowId, const QUrl &url)
{
    Q_ASSERT(url.isValid());
    auto handle = FMQuickWindowIns->findWindowById(windowId);

    if (!handle) {
        fmWarning() << "Invalid window id: " << windowId;
        return;
    }

    fmInfo() << "cd to " << url;
    handle->setCurrentUrl(url);

    QUrl titleUrl { url };
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ titleUrl }, &urls);

    if (ok && !urls.isEmpty())
        titleUrl = urls.first();

    auto fileInfo = InfoFactory::create<FileInfo>(titleUrl);
    if (fileInfo) {
        handle->window()->setTitle(fileInfo->displayOf(DisPlayInfoType::kFileDisplayName));
    } else {
        handle->window()->setTitle({});
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

    FMQuickWindowIns->resetPreviousActivedWindowId();
    WindowHandle handle { openNew ? createNewWindow(url)
                                  : findExistsWindow(url) };
    if (!handle) {
        fmCritical() << "Create window failed for: " << url << FMQuickWindowIns->lastError();
        return;
    }

    FMQuickWindowIns->showWindow(handle);
}

void CoreHelper::cacheDefaultWindow()
{
    auto window { FMQuickWindowIns->createWindow({}, kDefaultPlugin, kDefaultId) };
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
                 << "State: " << plugin->pluginState();
    }
}

WindowHandle CoreHelper::defaultWindow()
{
    const auto &idList { FMQuickWindowIns->windowIdList() };
    if (idList.size() == 1) {
        auto handle { FMQuickWindowIns->findWindowById(idList.first()) };
        if (handle && !handle->window()->isVisible())
            return handle;
    }

    return {};
}

WindowHandle CoreHelper::createNewWindow(const QUrl &url)
{
    fmInfo() << "Create new window for: " << url;
    return FMQuickWindowIns->createWindow(url, kDefaultPlugin, kDefaultId);
}

WindowHandle CoreHelper::findExistsWindow(const QUrl &url)
{
    auto window { FMQuickWindowIns->findWindowByUrl(url) };

    if (window) {
        fmInfo() << "Find exists window for: " << url << ",for window:" << window->windId();
        return window;
    }

    fmWarning() << "Cannot find exists window for:" << url;
    auto oldWindow { defaultWindow() };
    if (oldWindow) {
        fmInfo() << "Close cached default window";
        oldWindow->setProperty("_dfm_isDefaultWindow", true);
        oldWindow->window()->close();
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

    QQuickWindow *window = qobject_cast<QQuickWindow *>(watched);
    if (!window)
        return ret;

    // real signal emit
    WindowHandle handle = FMQuickWindowIns->findWindowById(window->winId());
    if (!handle)
        return ret;

    // for bug-203703:
    // When hot-launching or opening a new window(not first),
    // we need all components to be displayed at the same time
    // when the window is showed
    if (type == QEvent::Show) {
        qsizetype windowCount { FMQuickWindowIns->windowIdList().size() };
        qsizetype lazyCount { DPF_NAMESPACE::LifeCycle::lazyLoadList().size() };
        if (windowCount > 1 || lazyCount == 0) {
            fmDebug("Show full window, win count %d, lazy count %d", windowCount, lazyCount);
            window->removeEventFilter(this);
            QMetaObject::invokeMethod(handle, "aboutToOpen", Qt::DirectConnection);
        }
        return ret;
    }

    if (type == QEvent::Paint) {
        fmDebug() << "Show empty window";
        window->removeEventFilter(this);
        QMetaObject::invokeMethod(handle, "aboutToOpen", Qt::QueuedConnection);
        return ret;
    }

    return ret;
}

CoreHelper::CoreHelper(QObject *parent)
    : QObject(parent)
{
}
