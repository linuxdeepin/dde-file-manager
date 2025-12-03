// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspacehelper.h"
#include "views/detailspacewidget.h"
#include "utils/detailmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <QPropertyAnimation>

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

void DetailSpaceHelper::showDetailView(quint64 windowId, bool checked)
{
    // Find the detail space widget for the given window ID
    DetailSpaceWidget *widget = findDetailSpaceByWindowId(windowId);

    // Check if animations are enabled in DConfig settings
    const bool animEnable = DConfigManager::instance()->value(
                                                              kAnimationDConfName, kAnimationDetailviewEnable, true)
                                    .toBool();

    if (checked) {
        // If widget doesn't exist, create a new one
        if (!widget) {
            addDetailSpace(windowId);
            widget = findDetailSpaceByWindowId(windowId);
            if (!widget) {
                fmCritical() << "Can't find the detail space!";
                return;
            }
        }

        // Only animate if widget is not visible or width needs to change
        bool notRepeatAni = (!widget->isVisible()) || (widget->width() != widget->detailWidth());
        if (animEnable && notRepeatAni) {
            // Update workspace width before animation
            updateWorkspaceWidth(windowId, widget, true);
            // Set initial width to 0 for animation
            widget->setFixedWidth(0);
            // Start show animation
            animateDetailView(widget, true);
        }

        // Make widget visible
        widget->setVisible(true);

        // Update detail view content with current window URL
        auto window = FMWindowsIns.findWindowById(windowId);
        setDetailViewByUrl(widget, window->currentUrl());
    } else if (widget) {
        // Handle hiding the detail view
        if (animEnable) {
            // Update workspace and animate hiding if animations enabled
            updateWorkspaceWidth(windowId, widget, false);
            animateDetailView(widget, false);
        } else {
            // Just hide widget if animations disabled
            widget->setVisible(false);
        }
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

        w->setCurrentUrl(url);
        QMap<int, QWidget *> widgetMap = DetailManager::instance().createExtensionView(w->currentUrl());
        if (!widgetMap.isEmpty()) {
            QList<int> indexs = widgetMap.keys();
            for (int &index : indexs) {
                w->insterExpandControl(index, widgetMap.value(index));
            }
        }
    } else {
        fmWarning() << "Cannot set detail view content - widget is null";
    }
}

QMutex &DetailSpaceHelper::mutex()
{
    static QMutex m;
    return m;
}

void DetailSpaceHelper::animateDetailView(DetailSpaceWidget *widget, bool show)
{
    const int duration = getAnimationDuration();
    const auto curve = getAnimationCurve();
    QPropertyAnimation *ani = new QPropertyAnimation(widget, "maximumWidth");
    ani->setDuration(duration);
    ani->setEasingCurve(curve);
    ani->setStartValue(show ? 0 : widget->width());
    ani->setEndValue(show ? widget->detailWidth() : 0);

    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void DetailSpaceHelper::updateWorkspaceWidth(quint64 windowId, DetailSpaceWidget *widget, bool show)
{
    int width = show ? -widget->detailWidth() : widget->width();
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_AboutToChangeViewWidth",
                         windowId, width);
}

int DetailSpaceHelper::getAnimationDuration()
{
    return DConfigManager::instance()->value(kAnimationDConfName,
                                             kAnimationDetailviewDuration, 366)
            .toInt();
}

QEasingCurve::Type DetailSpaceHelper::getAnimationCurve()
{
    return static_cast<QEasingCurve::Type>(
            DConfigManager::instance()->value(kAnimationDConfName,
                                              kAnimationDetailviewCurve)
                    .toInt());
}
