/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "detailspacehelper.h"
#include "views/detailspacewidget.h"
#include "utils/detailmanager.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_detailspace;

DFMBASE_USE_NAMESPACE

QMap<quint64, DetailSpaceWidget *> DetailSpaceHelper::kDetailSpaceMap {};
QUrl DetailSpaceHelper::kLastSelectedUrl {};

DetailSpaceWidget *DetailSpaceHelper::findDetailSpaceByWindowId(quint64 windowId)
{
    if (!kDetailSpaceMap.contains(windowId))
        return nullptr;

    return kDetailSpaceMap[windowId];
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
            showDetailView(windowId, checked);
            return;
        }

        w->setVisible(true);

        if (!kLastSelectedUrl.isEmpty() && kLastSelectedUrl.isValid()) {
            setDetailViewSelectFileUrl(windowId, kLastSelectedUrl);
        } else {
            auto window = FMWindowsIns.findWindowById(windowId);
            setDetailViewSelectFileUrl(windowId, window->currentUrl());
        }
    } else {
        if (w)
            w->setVisible(false);
    }
}

void DetailSpaceHelper::setDetailViewSelectFileUrl(quint64 windowId, const QUrl &url)
{
    kLastSelectedUrl = url;

    DetailSpaceWidget *w = findDetailSpaceByWindowId(windowId);
    if (w) {
        if (!w->isVisible())
            return;

        w->setCurrentUrl(url);
        QMap<int, QWidget *> widgetMap = DetailManager::instance().createExtensionView(url);
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
