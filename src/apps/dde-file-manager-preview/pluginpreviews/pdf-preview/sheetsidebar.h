// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHEETSIDEBAR_H
#define SHEETSIDEBAR_H

#include "preview_plugin_global.h"

#include <QWidget>
#include <QStackedLayout>

namespace plugin_filepreview {
enum PreviewWidgesFlag {
    PREVIEW_NULL = 0x000,
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
