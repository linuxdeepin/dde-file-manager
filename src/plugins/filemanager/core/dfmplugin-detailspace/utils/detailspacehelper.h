// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILSPACEHELPER_H
#define DETAILSPACEHELPER_H

#include "dfmplugin_detailspace_global.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QMap>
#include <QUrl>
#include <QMutex>
namespace dfmplugin_detailspace {

class DetailSpaceWidget;
class DetailSpaceHelper
{
public:
    static DetailSpaceWidget *findDetailSpaceByWindowId(quint64 windowId);
    static quint64 findWindowIdByDetailSpace(DetailSpaceWidget *widget);
    static void addDetailSpace(quint64 windowId);
    static void removeDetailSpace(quint64 windowId);
    static void showDetailView(quint64 windowId, bool checked);
    static void setDetailViewSelectFileUrl(quint64 windowId, const QUrl &url);
    static void setDetailViewByUrl(DetailSpaceWidget *w, const QUrl &url);

private:
    static QMutex &mutex();
    static QMap<quint64, DetailSpaceWidget *> kDetailSpaceMap;
};

}   // namespace dfmplugin_detailspace

#endif   // DETAILSPACEHELPER_H
