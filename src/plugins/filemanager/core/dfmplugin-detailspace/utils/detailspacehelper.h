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
#ifndef DETAILSPACEHELPER_H
#define DETAILSPACEHELPER_H

#include "dfmplugin_detailspace_global.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <QMap>
#include <QUrl>
#include <QMutex>
namespace dfmplugin_detailspace {

class DetailSpaceWidget;
class DetailSpaceHelper
{
public:
    static DetailSpaceWidget *findDetailSpaceByWindowId(quint64 windowId);
    static void addDetailSpace(quint64 windowId);
    static void removeDetailSpace(quint64 windowId);
    static void showDetailView(quint64 windowId, bool checked);
    static void setDetailViewSelectFileUrl(quint64 windowId, const QUrl &url);
    static void setDetailViewByUrl(DetailSpaceWidget *w, const QUrl &url);
    static void resetSelectedUrl();

private:
    static QMutex &mutex();
    static QMap<quint64, DetailSpaceWidget *> kDetailSpaceMap;
    static QUrl kLastSelectedUrl;
};

}   // namespace dfmplugin_detailspace

#endif   // DETAILSPACEHELPER_H
