// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URLPUSHBUTTON_P_H
#define URLPUSHBUTTON_P_H

#include "dfmplugin_titlebar_global.h"

#include <DPushButton>
#include <QScopedPointer>

class QUrl;
class QEvent;
class QListView;
class QMenu;

namespace dfmplugin_titlebar {
class CrumbInterface;
class FolderListWidget;
class UrlPushButton;
class UrlPushButtonPrivate : public QObject
{
    Q_OBJECT
    friend class UrlPushButton;
    UrlPushButton *q { nullptr };

    bool active { false };   // 按钮是否处于活动状态
    bool hoverFlag { false };   // 鼠标是否悬停在按钮上
    bool stacked { false };   // 是否为堆叠模式
    bool subDirVisible { true };   // 是否先生子目录
    QList<CrumbData> crumbDatas;   // 面包屑数据列表
    QString subDir;   // 活动子目录
    QScopedPointer<QMenu> menu;   // 上下文菜单
    QFont font;

    CrumbInterface *crumbController { nullptr };
    QStringList completionStringList;
    FolderListWidget *folderListWidget { nullptr };

public:
    explicit UrlPushButtonPrivate(UrlPushButton *qq);
    virtual ~UrlPushButtonPrivate();

private:
    void initConnect();
    int arrowWidth() const;
    bool isAboveArrow(int x) const;
    bool isTextClipped() const;
    bool isSubDir(int x) const;
    void updateWidth();
    QColor foregroundColor() const;
    bool popupVisible() const;

    void requestCompleteByUrl(const QUrl &url);

private Q_SLOTS:
    void onCustomContextMenu(const QPoint &point);
    void activate();
    void onSelectSubDirs();
    void onCompletionFound(const QStringList &stringList);
    void onCompletionCompleted();
};

}   // namespace dfmplugin_titlebar

#endif   // URLPUSHBUTTON_P_H
