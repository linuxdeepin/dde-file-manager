/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
