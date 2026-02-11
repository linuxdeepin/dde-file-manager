// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHEETBROWSER_H
#define SHEETBROWSER_H

#include "preview_plugin_global.h"

#include <DGraphicsView>

#include <QPointer>
#include <QBasicTimer>

class QScroller;

namespace plugin_filepreview {
class SheetOperation;
class BrowserPage;
class DocSheet;
class RenderViewportThread;
class BrowserMagniFier;
class FindWidget;

class SheetBrowser : public DTK_WIDGET_NAMESPACE::DGraphicsView
{
    Q_OBJECT
public:
    explicit SheetBrowser(DocSheet *parent = nullptr);

    ~SheetBrowser() override;

    /**
     * @brief deform
     * 渲染当前视图中的页
     * @param operation 渲染参数包括,大小,单双页,旋转,缩放比例
     */
    void deform(SheetOperation &operation);

    /**
     * @brief hasLoaded
     * 文档是否已加载过
     * @return
     */
    bool hasLoaded();

    /**
     * @brief allPages
     * 当前文档页码总数
     * @return
     */
    int allPages();

    /**
     * @brief currentPage
     * 当前页页码(从1开始)
     * @return
     */
    int currentPage();

    /**
     * @brief setCurrentPage
     * 页码跳转
     * @param page 跳转的页码(base 1)
     */
    void setCurrentPage(int page);

    /**
     * @brief getExistImage
     * 获取文档页图片
     * @param index 页码编号
     * @param image 页码图片
     * @param width 图片宽度
     * @param height 图片高度
     * @param mode 图片状态
     * @param bSrc
     * @return
     */
    bool getExistImage(int index, QImage &image, int width, int height);

    /**
     * @brief magnifierOpened
     * 是否开启了放大镜
     * @return true:开启,false:未开启
     */
    bool magnifierOpened();

    /**
     * @brief maxWidth
     * 最大一页的宽度
     * @return  宽度值
     */
    qreal maxWidth();

    /**
     * @brief maxHeight
     *最大一页的高度
     * @return 高度值
     */
    qreal maxHeight();

    /**
     * @brief beginViewportChange
     * 延时进行视图更新 过滤高频率调用
     */
    void beginViewportChange();

    /**
     * @brief pages
     * @return
     */
    QList<BrowserPage *> pages();

    /**
     * @brief init
     * 按历史记录初始化，只可调用一次
     * @param operation 操作记录
     */
    void init(SheetOperation &operation);

signals:
    void sigPageChanged(int page);

    void sigNeedPageFirst();

    void sigNeedPagePrev();

    void sigNeedPageNext();

    void sigNeedPageLast();

protected:
    /**
     * @brief showEvent
     * 第一次显示之后跳转到初始页码
     * @param event
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief resizeEvent
     * 当前文档视图大小变化事件
     * @param event
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief focusOutEvent
     * 失去焦点处理
     * @param event
     */
    void focusOutEvent(QFocusEvent *event) override;

    /**
     * @brief 计时器事件
     * @param event
     */
    void timerEvent(QTimerEvent *event) override;

    /**
     * @brief wheelEvent 鼠标滚轮事件
     * @param e
     */
    void wheelEvent(QWheelEvent *e) override;

    /**
     * @brief 根据鼠标点击位置判断在哪一页
     * @param viewPoint 鼠标点击位置
     * @return 鼠标点击的页对象
     */
    BrowserPage *getBrowserPageForPoint(QPointF &viewPoint);

protected:
    // 实现拖拽移动
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    // 实现拖拽移动
    bool isPressed { false };
    QPoint mouseStartPos;
    QPoint windowStartPos;

private slots:
    /**
     * @brief onInit
     * 初始化响应,跳转到初始化页
     */
    void onInit();

    /**
     * @brief onVerticalScrollBarValueChanged
     * 纵向滚动条数值改变槽函数,并通知左侧栏页变化
     */
    void onVerticalScrollBarValueChanged(int value);

    /**
     * @brief onViewportChanged
     * 当前视图区域改变,渲染文档
     */
    void onViewportChanged();

    /**
     * @brief onRemoveDocSlideGesture
     * 移除文档触摸屏滑动事件
     */
    void onRemoveDocSlideGesture();

private:
    /**
     * @brief currentIndexRange
     * 当前显示的页数范围
     * @param fromIndex 当前显示的起始页码索引
     * @param toIndex 当前显示的结束页码索引
     * @return
     */
    void currentIndexRange(int &fromIndex, int &toIndex);

    /**
     * @brief currentScrollValueForPage
     * 纵向滚动条值变化,得到滚动条当前位置所在文档页的编号
     * @return 文档页的编号(页码从1开始)
     */
    int currentScrollValueForPage();

private:
    DocSheet *docSheet { nullptr };

    BrowserPage *lastFindBrowserPage { nullptr };
    QTimer *viewportChangeTimer { nullptr };   //用于延时进行视图区域更新 防止高频率调用

    QString filePassword;
    QList<BrowserPage *> browserPageList {};

    QPointF selectPressedPos;   // 鼠标按压的起始位置
    QPointF selectStartPos;   // 选取文字的开始位置
    QPointF selectEndPos;   // 选取文字的结束位置(鼠标释放的最后位置)

    double lastScaleFactor { 0 };
    qreal maxPageWidth { 0 };   //最大一页的宽度
    qreal maxPageHeight { 0 };   //最大一页的高度
    bool changSearchFlag { false };
    bool hasLoadedPage { false };   //是否已经加载过每页的信息
    int jumpPageNumber { 1 };   //用于刚显示跳转的页数

    int currentPageIndex { 0 };
    int lastrotation { 0 };
    bool needNotifyCurPageChanged { true };

    QBasicTimer repeatTimer;
    bool startPinch { false };   //开始收缩
    bool canTouchScreen { false };
    QScroller *scrollerPage { nullptr };
};
}
#endif   // SHEETBROWSER_H
