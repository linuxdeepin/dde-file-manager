// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspacehelper.h"
#include "views/detailspacewidget.h"
#include "utils/detailmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/dpf.h>

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

    // create new detail widget in window and
    if (checked) {
        if (!w) {
            addDetailSpace(windowId);
            w = findDetailSpaceByWindowId(windowId);
            if (!w) {
                qCritical() << "Can't find the detail space!";
                return;
            }
        }
        w->setVisible(true);
        auto window = FMWindowsIns.findWindowById(windowId);
        setDetailViewByUrl(w, window->currentUrl());
    } else {
        if (w)
            w->setVisible(false);
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
