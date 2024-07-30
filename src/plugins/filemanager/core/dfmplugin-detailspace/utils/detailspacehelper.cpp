// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspacehelper.h"
#include "views/detailspacewidget.h"
#include "utils/detailmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/dpf.h>

#include <QPropertyAnimation>

using namespace dfmplugin_detailspace;

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
    DetailSpaceWidget *w = findDetailSpaceByWindowId(windowId);

    if (checked) {
        if (!w) {
            addDetailSpace(windowId);
            w = findDetailSpaceByWindowId(windowId);
            if (!w) {
                fmCritical() << "Can't find the detail space!";
                return;
            }
        }

        // Set initial width to 0
        w->setFixedWidth(0);
        w->setVisible(true);

        QPropertyAnimation *ani = new QPropertyAnimation(w, "maximumWidth");
        ani->setEasingCurve(QEasingCurve::OutExpo);
        ani->setDuration(kViewAnimationDuration);
        ani->setStartValue(0);
        ani->setEndValue(w->detailWidth());   // Set to desired width
        ani->start(QAbstractAnimation::DeleteWhenStopped);

        auto window = FMWindowsIns.findWindowById(windowId);
        setDetailViewByUrl(w, window->currentUrl());

    } else {
        if (w) {
            QPropertyAnimation *aniHide = new QPropertyAnimation(w, "maximumWidth");
            aniHide->setDuration(kViewAnimationDuration);
            aniHide->setStartValue(w->width());
            aniHide->setEndValue(0);
            aniHide->setEasingCurve(QEasingCurve::OutExpo);
            aniHide->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
}

void DetailSpaceHelper::setDetailViewSelectFileUrl(quint64 windowId, const QUrl &url)
{
    DetailSpaceWidget *w = findDetailSpaceByWindowId(windowId);
    if (w)
        setDetailViewByUrl(w, url);
}

void DetailSpaceHelper::setDetailViewByUrl(DetailSpaceWidget *w, const QUrl &url)
{
    if (w) {
        if (!w->isVisible())
            return;

        w->setCurrentUrl(url);
        QMap<int, QWidget *> widgetMap = DetailManager::instance().createExtensionView(w->currentUrl());
        if (!widgetMap.isEmpty()) {
            QList<int> indexs = widgetMap.keys();
            for (int &index : indexs) {
                w->insterExpandControl(index, widgetMap.value(index));
            }
        }
    }
}

QMutex &DetailSpaceHelper::mutex()
{
    static QMutex m;
    return m;
}
