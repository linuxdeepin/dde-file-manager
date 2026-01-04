// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspacehelper.h"
#include "views/detailspacewidget.h"
#include "utils/detailmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <QTimer>

using namespace dfmplugin_detailspace;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::AnimationConfig;

DFMBASE_USE_NAMESPACE

QMap<quint64, DetailSpaceWidget *> DetailSpaceHelper::kDetailSpaceMap {};

DetailSpaceWidget *DetailSpaceHelper::findDetailSpaceByWindowId(quint64 windowId)
{
    if (!kDetailSpaceMap.contains(windowId))
        return nullptr;

    return kDetailSpaceMap[windowId];
}

quint64 DetailSpaceHelper::findWindowIdByDetailSpace(DetailSpaceWidget *widget)
{
    return kDetailSpaceMap.key(widget, 0);
}

void DetailSpaceHelper::addDetailSpace(quint64 windowId)
{
    QMutexLocker locker(&DetailSpaceHelper::mutex());
    if (!kDetailSpaceMap.contains(windowId)) {
        DetailSpaceWidget *detailSpaceWidget = new DetailSpaceWidget;
        auto window = FMWindowsIns.findWindowById(windowId);
        window->installDetailView(detailSpaceWidget);
        kDetailSpaceMap.insert(windowId, detailSpaceWidget);
    }
}

void DetailSpaceHelper::removeDetailSpace(quint64 windowId)
{
    QMutexLocker locker(&DetailSpaceHelper::mutex());
    if (kDetailSpaceMap.contains(windowId)) {
        DetailSpaceWidget *widget = kDetailSpaceMap.take(windowId);
        widget->deleteLater();
    }
}

void DetailSpaceHelper::showDetailView(quint64 windowId, bool checked, bool userAction)
{
    // Find the detail space widget for the given window ID
    DetailSpaceWidget *widget = findDetailSpaceByWindowId(windowId);
    auto window = FMWindowsIns.findWindowById(windowId);
    if (!window)
        return;

    if (checked) {
        bool useAnimation = true;
        // If widget doesn't exist, create a new one
        if (!widget) {
            addDetailSpace(windowId);
            widget = findDetailSpaceByWindowId(windowId);
            if (!widget) {
                fmCritical() << "Can't find the detail space!";
                return;
            }
            useAnimation = false;
        }

        // Get the saved width from window
        int targetWidth = window->detailViewWidth();

        // Notify workspace about width change
        updateWorkspaceWidth(windowId, widget, true, targetWidth);

        // Show the detailspace with animation (window handles visibility and animation internally)
        QVariantHash options;
        options[DFMBASE_NAMESPACE::DetailSpaceOptions::kAnimated] = useAnimation;
        options[DFMBASE_NAMESPACE::DetailSpaceOptions::kUserAction] = userAction;
        window->showDetailSpace(options);

        // IMPORTANT: Update content AFTER detailspace becomes visible
        // Use QTimer to ensure the widget is fully visible before loading content
        QTimer::singleShot(0, widget, [widget, window, windowId]() {
            // Refresh with current selection (same logic as handleViewSelectionChanged)
            const QList<QUrl> &urls = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", windowId).value<QList<QUrl>>();
            if (urls.isEmpty()) {
                setDetailViewByUrl(widget, window->currentUrl());
            } else {
                setDetailViewByUrl(widget, urls.first());
            }
        });

    } else if (widget) {
        // Handle hiding the detail view
        int currentWidth = widget->width();

        // Notify workspace about width change
        updateWorkspaceWidth(windowId, widget, false, currentWidth);

        // Hide the detailspace with animation
        QVariantHash options;
        options[DFMBASE_NAMESPACE::DetailSpaceOptions::kAnimated] = true;
        options[DFMBASE_NAMESPACE::DetailSpaceOptions::kUserAction] = userAction;
        window->hideDetailSpace(options);
    }
}

void DetailSpaceHelper::setDetailViewSelectFileUrl(quint64 windowId, const QUrl &url)
{
    DetailSpaceWidget *w = findDetailSpaceByWindowId(windowId);
    if (w && url.isValid())
        setDetailViewByUrl(w, url);
}

void DetailSpaceHelper::setDetailViewByUrl(DetailSpaceWidget *w, const QUrl &url)
{
    if (w) {
        if (!w->isVisible()) {
            fmDebug() << "Widget not visible, skipping content update";
            return;
        }

        // Simply set the URL - DetailView::setUrl() will handle all widget creation
        // including extension widgets, following Single Responsibility Principle
        w->setCurrentUrl(url);
    } else {
        fmWarning() << "Cannot set detail view content - widget is null";
    }
}

QMutex &DetailSpaceHelper::mutex()
{
    static QMutex m;
    return m;
}

void DetailSpaceHelper::updateWorkspaceWidth(quint64 windowId, DetailSpaceWidget *widget, bool show, int targetWidth)
{
    int width = show ? -targetWidth : widget->width();
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_AboutToChangeViewWidth",
                         windowId, width);
}
