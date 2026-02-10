// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BROWSERPAGE_H
#define BROWSERPAGE_H

#include "preview_plugin_global.h"

#include "global.h"
#include "pdfmodel.h"
#include "docsheet.h"

#include <QGraphicsItem>

namespace plugin_filepreview {
class Page;
class SheetBrowser;

class BrowserPage : public QGraphicsItem
{
    friend class PageRenderThread;
    friend class PageViewportThread;

public:
    explicit BrowserPage(SheetBrowser *parent, int index, DocSheet *sheet);

    ~BrowserPage() override;

    /**
     * @brief 文档页缩放后的原区域 不受旋转影响
     * @return
     */
    QRectF boundingRect() const override;

    /**
     * @brief 文档页实际区域
     * @return
     */
    QRectF rect();

    /**
     * @brief 文档进行加载像显示内容
     * @param scaleFactor 缩放系数
     * @param rotation 旋转角度
     * @param renderLater 是否延迟加载
     * @param force 是否强制更新
     */
    void render(const double &scaleFactor, const Rotation &rotation, const bool &renderLater = false, const bool &force = false);

    /**
     * @brief 加载局部区域
     * @param scaleFactor 缩放系数
     * @param rect 需要被局部加载的文档区域
     */
    void renderRect(const QRectF &rect);

    /**
     * @brief 加载当前视图区域
     * @param scaleFactor 缩放因子
     */
    void renderViewPort();

    /**
     * @brief currentImage
     * 获取整个图片并转成对应宽高
     * @param width 宽
     * @param height 高
     * @param mode 缩放模式
     * @param bSrc 是否可以使用已存在图片缩放
     * @return
     */
    QImage getCurrentImage(int width, int height);

    /**
     * @brief itemIndex
     * 当前页的编号(从0开始)
     * @return
     */
    int itemIndex();

    /**
     * @brief clearPixmap
     * 删除缓存图片,当距离当前页较远的时候需要被清除以节省内存空间占用
     */
    void clearPixmap();

    /**
     * @brief getTopLeftPos
     * 根据旋转角度和item坐标计算出item的左上角坐标
     * @return
     */
    QPointF getTopLeftPos();

private:
    /**
     * @brief handleRenderFinished
     * 渲染缩略图
     * @param scaleFactor 缩放系数
     * @param image 缩略图
     * @param rect 范围
     */
    void handleRenderFinished(const int &pixmapId, const QPixmap &pixmap, const QRect &slice = QRect());

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    DocSheet *docSheet { nullptr };

    SheetBrowser *sheetBrowserParent { nullptr };

    double currentScaleFactor { -1 };   //当前被设置的缩放
    int currentIndex { 0 };   //当前索引
    Rotation currentRotation { Rotation::kNumberOfRotations };   //当前被设置的旋转

    QPixmap currentPixmap;   //当前图片
    QPixmap currentRenderPixmap;   //当前被绘制图片
    int currentPixmapId { 0 };   //当前图片的标识
    bool pixmapIsLastest { false };   //当前图示是否最新
    bool pixmapHasRendered { false };   //当前图片是否已经加载
    double renderPixmapScaleFactor { -1 };   //当前图片的缩放
    bool viewportRendered { false };   //图片初始化加载视图窗口

    QSizeF originSizeF;
};
}
#endif   // BROWSERPAGE_H
