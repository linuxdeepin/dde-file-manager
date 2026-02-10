// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARIMAGELISTVIEW_H
#define SIDEBARIMAGELISTVIEW_H

#include "preview_plugin_global.h"

#include <DListView>

namespace plugin_filepreview {
class DocSheet;
class SideBarImageViewModel;

const int LEFTMINWIDTH = 266;
const int LEFTMAXWIDTH = 380;

class SideBarImageListView : public DTK_WIDGET_NAMESPACE::DListView
{
    Q_OBJECT
public:
    SideBarImageListView(DocSheet *sheet, QWidget *parent = nullptr);

signals:

    /**
     * @brief sigListItemClicked
     * 节点左键点击
     * @param row
     */
    void sigListItemClicked(int row);

public:
    /**
     * @brief handleOpenSuccess
     * 文档打开成功,初始化相关数据
     */
    void handleOpenSuccess();

    /**
     * @brief scrollToIndex
     * 滚动到指定页数
     * @param pageIndex
     * @param scrollTo
     * @return
     */
    bool scrollToIndex(int pageIndex, bool scrollTo = true);

    /**
     * @brief pageUpIndex
     * 上翻页
     * @return
     */
    QModelIndex pageUpIndex();

    /**
     * @brief pageDownIndex
     * 下翻页
     * @return
     */
    QModelIndex pageDownIndex();

private:
    /**
     * @brief initControl
     * 初始化控件
     */
    void initControl();

private slots:
    /**
     * @brief onItemClicked
     * 节点点击
     * @param index
     */
    void onItemClicked(const QModelIndex &index);

protected:
    /**
     * @brief mousePressEvent
     * 鼠标点击事件
     * @param event
     */
    void mousePressEvent(QMouseEvent *event);

private:
    int listType;
    DocSheet *docSheet { nullptr };
    SideBarImageViewModel *imageModel { nullptr };
};
}
#endif   // SIDEBARIMAGELISTVIEW_H
