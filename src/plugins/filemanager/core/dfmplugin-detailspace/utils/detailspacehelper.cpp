// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspacehelper.h"
#include "detailspacecontainment.h"
#include "utils/detailmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-gui/panel.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_detailspace;

DFMBASE_USE_NAMESPACE

QMap<quint64, DetailSpaceContainment *> DetailSpaceHelper::kDetailSpaceMap {};

DetailSpaceContainment *DetailSpaceHelper::findDetailSpaceByWindowId(quint64 windowId)
{
    if (!kDetailSpaceMap.contains(windowId))
        return nullptr;

    return kDetailSpaceMap[windowId];
}

quint64 DetailSpaceHelper::findWindowIdByDetailSpace(DetailSpaceContainment *contain)
{
    return kDetailSpaceMap.key(contain, 0);
}

void DetailSpaceHelper::addDetailSpace(DetailSpaceContainment *contain)
{
    QMutexLocker locker(&DetailSpaceHelper::mutex());
    if (auto panel = contain->panel()) {
        if (!kDetailSpaceMap.contains(panel->windId())) {
            kDetailSpaceMap.insert(panel->windId(), contain);
        }
    }
}

void DetailSpaceHelper::removeDetailSpace(quint64 windowId)
{
    QMutexLocker locker(&DetailSpaceHelper::mutex());
    if (kDetailSpaceMap.contains(windowId)) {
        // 对应Applet生命周期由Panel管理，此处暂存的指针不析构
        (void)kDetailSpaceMap.take(windowId);
    }
}

void DetailSpaceHelper::showDetailView(quint64 windowId, bool checked)
{
    DetailSpaceContainment *contain = findDetailSpaceByWindowId(windowId);
    if (contain) {
        contain->setDetailVisible(checked);
    }
}

void DetailSpaceHelper::setDetailViewSelectFileUrl(quint64 windowId, const QUrl &url)
{
    DetailSpaceContainment *contain = findDetailSpaceByWindowId(windowId);
    if (contain)
        setDetailViewByUrl(contain, url);
}

void DetailSpaceHelper::setDetailViewByUrl(DetailSpaceContainment *contain, const QUrl &url)
{
    if (contain) {
        if (!contain->rootObject())
            return;

        contain->setCurrentUrl(url);

        // TODO: 完善拓展接口
#if 0
        QMap<int, QWidget *> widgetMap = DetailManager::instance().createExtensionView(w->currentUrl());
        if (!widgetMap.isEmpty()) {
            QList<int> indexs = widgetMap.keys();
            for (int &index : indexs) {
                w->insterExpandControl(index, widgetMap.value(index));
            }
        }
#endif
    }
}

QMutex &DetailSpaceHelper::mutex()
{
    static QMutex m;
    return m;
}
