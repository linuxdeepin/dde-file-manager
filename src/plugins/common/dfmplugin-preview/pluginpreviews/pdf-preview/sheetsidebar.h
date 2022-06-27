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

#ifndef SHEETSIDEBAR_H
#define SHEETSIDEBAR_H

#include "preview_plugin_global.h"

#include <QWidget>
#include <QStackedLayout>

namespace plugin_filepreview {
enum PreviewWidgesFlag {
    PREVIEW_THUMBNAIL = 0x0001,
};
}
Q_DECLARE_FLAGS(PreviewWidgesFlags, PREVIEW_NAMESPACE::PreviewWidgesFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(PreviewWidgesFlags)

namespace plugin_filepreview {
class DocSheet;
class ThumbnailWidget;

class SheetSidebar : public QWidget
{
    Q_OBJECT

public:
    explicit SheetSidebar(DocSheet *parent = nullptr, PreviewWidgesFlags widgesFlag = PREVIEW_THUMBNAIL);

    ~SheetSidebar() override;

    /**S
     * @brief 设置当前页
     * @param page
     */
    void setCurrentPage(int page);

    /**
     * @brief 文档打开成功
     */
    void handleOpenSuccess();

    /**
     * @brief 重置模型数据
     */
    void changeResetModelData();

protected:
    /**
     * @brief 初始化控件
     */
    void initWidget();

protected:
    /**
     * @brief 窗口大小变化事件
     * @param event
     */
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    /**
     * @brief 显示事件
     * @param event
     */
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private slots:

    /**
     * @brief 文档打开成功处理
     */
    void onHandWidgetDocOpenSuccess();

    /**
     * @brief 文档打开成功延迟处理
     */
    void onHandleOpenSuccessDelay();

private:
    qreal scaleRatio { -1 };
    bool oldVisible { false };
    bool openDocOpenSuccess { false };

    DocSheet *docSheet { nullptr };
    QStackedLayout *stackLayout { nullptr };
    PreviewWidgesFlags widgetsFlag;

    ThumbnailWidget *thumbnailWidget { nullptr };
};
}
#endif   // SHEETSIDEBAR_H
