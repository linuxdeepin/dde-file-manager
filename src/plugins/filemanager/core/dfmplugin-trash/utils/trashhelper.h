/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef TRASHHELPER_H
#define TRASHHELPER_H

#include "dfmplugin_trash_global.h"

#include "dfm-base/utils/clipboard.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/dfm_global_defines.h"

#include <QUrl>
#include <QIcon>

QT_BEGIN_HEADER
class QFrame;
QT_END_NAMESPACE

namespace dfmbase {
class LocalFileWatcher;
}

namespace dfmplugin_trash {

class EmptyTrashWidget;
class TrashHelper final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashHelper)
    using BasicExpand = QMultiMap<QString, QPair<QString, QString>>;
    using ExpandFieldMap = QMap<QString, BasicExpand>;

public:
    static TrashHelper *instance();

    inline static QString scheme()
    {
        return DFMBASE_NAMESPACE::Global::Scheme::kTrash;
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("user-trash-symbolic");
    }

    static QUrl rootUrl();
    static quint64 windowId(QWidget *sender);
    static void contenxtMenuHandle(const quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static QFrame *createEmptyTrashTopWidget();
    static bool showTopWidget(QWidget *w, const QUrl &url);
    static QUrl transToTrashFile(const QString &filePath);
    static QUrl trashFileToTargetUrl(const QUrl &url);
    static bool isTrashFile(const QUrl &url);
    static bool isTrashRootFile(const QUrl &url);
    static bool isEmpty();
    static void emptyTrash(const quint64 windowId = 0);
    static ExpandFieldMap propetyExtensionFunc(const QUrl &url);
    static JobHandlePointer restoreFromTrashHandle(const quint64 windowId,
                                                   const QList<QUrl> urls,
                                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    bool checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool detailViewIcon(const QUrl &url, QString *iconName);

    bool customColumnRole(const QUrl &rootUrl, QList<DFMGLOBAL_NAMESPACE::ItemRoles> *roleList);
    bool customRoleDisplayName(const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role, QString *displayName);

private:
    void onTrashStateChanged();

private:
    explicit TrashHelper(QObject *parent = nullptr);
    void initEvent();

private:
    DFMBASE_NAMESPACE::LocalFileWatcher *trashFileWatcher { nullptr };
    bool isTrashEmpty;
};

}
#endif   // TRASHHELPER_H
