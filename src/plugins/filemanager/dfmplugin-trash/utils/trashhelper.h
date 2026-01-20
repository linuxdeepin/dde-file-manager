// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHHELPER_H
#define TRASHHELPER_H

#include "dfmplugin_trash_global.h"

#include <dfm-base/utils/clipboard.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QUrl>
#include <QIcon>

QT_BEGIN_NAMESPACE
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
    static void emptyTrash(const quint64 windowId = 0);
    static ExpandFieldMap propetyExtensionFunc(const QUrl &url);
    static ExpandFieldMap detailExtensionFunc(const QUrl &url);
    static JobHandlePointer restoreFromTrashHandle(const quint64 windowId,
                                                   const QList<QUrl> urls,
                                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    bool checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool checkCanMove(const QUrl &url);
    bool detailViewIcon(const QUrl &url, QString *iconName);

    bool customColumnRole(const QUrl &rootUrl, QList<DFMGLOBAL_NAMESPACE::ItemRoles> *roleList);
    bool customRoleDisplayName(const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role, QString *displayName);
    void onTrashEmptyState();
    void trashNotEmpty();
    void handleWindowUrlChanged(quint64 winId, const QUrl &url);

private Q_SLOTS:
    void onTrashNotEmptyState();

Q_SIGNALS:
    void trashNotEmptyState();

private:
    void onTrashStateChanged();

private:
    enum class TrashState {
        Unknown,    // Initial state, not yet determined
        Empty,      // Trash is empty
        NotEmpty    // Trash contains files
    };

    explicit TrashHelper(QObject *parent = nullptr);
    void initEvent();

private:
    DFMBASE_NAMESPACE::LocalFileWatcher *trashFileWatcher { nullptr };
    TrashState trashState { TrashState::Unknown };
};

}
#endif   // TRASHHELPER_H
