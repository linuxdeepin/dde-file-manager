// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDERLISTWIDGET_P_H
#define FOLDERLISTWIDGET_P_H

#include "dfmplugin_titlebar_global.h"

#include <DListView>

class QVBoxLayout;
class QStandardItemModel;

namespace dfmplugin_titlebar {
class FolderViewDelegate;
class FolderListWidget;
class FolderListWidgetPrivate : public QObject
{
    Q_OBJECT
    friend class FolderListWidget;
    FolderListWidget *q { nullptr };

    QVBoxLayout *layout { nullptr };
    QStandardItemModel *folderModel { nullptr };
    Dtk::Widget::DListView *folderView { nullptr };
    FolderViewDelegate *folderDelegate { nullptr };

    QList<CrumbData> crumbDatas;   // 数据列表

public:
    explicit FolderListWidgetPrivate(FolderListWidget *qq);
    virtual ~FolderListWidgetPrivate();

private:
    void initConnect();
    QModelIndex getStartIndexFromHover(bool isUp = false);
    int getStartRowFromHover();
    bool findAndSelectMatch(const QString &text, int startRow) const;
    bool matchText(const QString &source, const QString &input) const;

private Q_SLOTS:
    void clicked(const QModelIndex &index);
    void selectUp();
    void selectDown();
    void returnPressed();
    void handleKeyInput(const QString &pressedText);
};

}   // namespace dfmplugin_titlebar

#endif   // FOLDERLISTWIDGET_P_H
