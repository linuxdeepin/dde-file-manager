// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include "preview_plugin_global.h"

#include <QWidget>

#include <QPointer>

namespace plugin_filepreview {
class DocSheet;
class SideBarImageListView;
class ThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThumbnailWidget(DocSheet *sheet, QWidget *parent = nullptr);
    ~ThumbnailWidget() Q_DECL_OVERRIDE;

public:
    /**
     * @brief handleOpenSuccess
     * 文档打开成功,初始化数据
     */
    void handleOpenSuccess();

    /**
     * @brief handlePage
     * 滚动到指定页数
     * @param index
     */
    void handlePage(int index);

    /**
     * @brief adaptWindowSize
     * 控件大小变化响应
     * @param scale
     */
    void adaptWindowSize(const double &scale);

protected:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

private:
    /**
     * @brief scrollToCurrentPage
     * 滚动到当前页数
     */
    void scrollToCurrentPage();

private:
    QPointer<DocSheet> docSheet;
    SideBarImageListView *pImageListView { nullptr };
    bool bIshandOpenSuccess { false };
};
}
#endif   // THUMBNAILWIDGET_H
