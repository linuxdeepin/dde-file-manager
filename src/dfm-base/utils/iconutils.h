// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONUTILS_H
#define ICONUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <DIconButton>
#include <DStyleOption>

#include <QIcon>
#include <QPixmap>

DFMBASE_BEGIN_NAMESPACE

namespace IconUtils {

/*!
 * \class IconLabel
 * \brief 非交互式的图标显示控件，继承自 DIconButton 但屏蔽所有交互效果
 *
 * 该控件用于需要显示主题感知图标但不需要交互的场景。
 * 通过重写事件处理方法为空实现，完全屏蔽鼠标和键盘交互，
 * 同时保留 DIconButton 的主题切换自动更新图标能力。
 */
class IconLabel : public DTK_WIDGET_NAMESPACE::DIconButton
{
    Q_OBJECT

public:
    explicit IconLabel(QWidget *parent = nullptr)
        : DIconButton(parent)
    {
        // 设置为扁平样式，移除按钮装饰
        setFlat(true);
        // 禁止键盘焦点
        setFocusPolicy(Qt::NoFocus);
        // 使用箭头光标
        setCursor(Qt::ArrowCursor);
    }

protected:
    // 重写 initStyleOption，移除悬停和按下状态标志，阻止样式渲染交互效果
    void initStyleOption(DTK_WIDGET_NAMESPACE::DStyleOptionButton *option) const override
    {
        DIconButton::initStyleOption(option);
        // 清除悬停和按下状态标志
        option->state &= ~(QStyle::State_MouseOver | QStyle::State_Sunken);
    }

    // 屏蔽所有鼠标交互事件
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *) override { }
#else
    void enterEvent(QEvent *) override { }
#endif
    void leaveEvent(QEvent *) override { }
    void mousePressEvent(QMouseEvent *) override { }
    void mouseReleaseEvent(QMouseEvent *) override { }
    void mouseMoveEvent(QMouseEvent *) override { }
    void mouseDoubleClickEvent(QMouseEvent *) override { }
    void keyPressEvent(QKeyEvent *) override { }
};

struct IconStyle
{
    int stroke { 2 };
    int radius { 4 };
    int shadowOffset { 1 };
    int shadowRange { 3 };
};
QPixmap renderIconBackground(const QSize &size, const IconStyle &style = IconStyle {});
QPixmap renderIconBackground(const QSizeF &size, const IconStyle &style = IconStyle {});
QPixmap addShadowToPixmap(const QPixmap &originalPixmap, int shadowOffsetY, qreal blurRadius, qreal shadowOpacity);
IconStyle getIconStyle(int size);
bool shouldSkipThumbnailFrame(const QString &mimeType);

/*!
 * \brief Generate a HiDPI-aware pixmap from QIcon.
 * Returns a pixmap scaled by the given widget's devicePixelRatio,
 * so it renders crisply on high-DPI screens.
 * \param icon Source QIcon
 * \param size Logical pixel size (e.g. 128x128)
 * \param widget Target widget to read devicePixelRatio from
 */
QPixmap hiDpiPixmap(const QIcon &icon, const QSize &size, const QWidget *widget);
}   // end namespace IconUtils

DFMBASE_END_NAMESPACE

#endif   // ICONUTILS_H
