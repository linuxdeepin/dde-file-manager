// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/viewswitchbutton_p.h"
#include "views/viewswitchbutton.h"

#include <DGuiApplicationHelper>
#include <DMenu>
#include <dtkwidget_global.h>

#include <QPainter>
#include <QStyleOptionButton>
#include <QMouseEvent>

using namespace dfmplugin_titlebar;
using namespace dfmbase::Global;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static constexpr int kCompactToolPadding { 6 };   // 按钮整体边距
static constexpr int kCompactToolSpacing { 6 };   // 内部元素间距
static constexpr int kCompactToolLeftIconSize { 16 };   // 左侧视图模式图标尺寸
static constexpr int kCompactToolArrowCircleSize { 18 };   // 右侧箭头圆形背景尺寸
static constexpr int kCompactToolArrowSize { 12 };   // 右侧箭头图标尺寸
// 按钮总宽 = padding + iconSize + spacing + circleSize + padding
static constexpr int kCompactToolButtonWidth { kCompactToolPadding + kCompactToolLeftIconSize
                                               + kCompactToolSpacing + kCompactToolArrowCircleSize
                                               + kCompactToolPadding };

ViewSwitchButtonPrivate::ViewSwitchButtonPrivate(ViewSwitchButton *parent)
    : QObject(parent),
      q(parent)
{
    initializeUi();
    setupMenu();
}

ViewSwitchButtonPrivate::~ViewSwitchButtonPrivate() = default;

void ViewSwitchButtonPrivate::initializeUi()
{
    q->setFixedSize(kCompactToolButtonWidth, kToolButtonSize);
}

void ViewSwitchButtonPrivate::setupMenu()
{
    menu = new DTK_WIDGET_NAMESPACE::DMenu(q);

    iconAction = menu->addAction(QObject::tr("Icon view"));
    iconAction->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));
    iconAction->setCheckable(true);

    listAction = menu->addAction(QObject::tr("List view"));
    listAction->setIcon(QIcon::fromTheme("dfm_viewlist_details"));
    listAction->setCheckable(true);

    treeAction = menu->addAction(QObject::tr("Tree view"));
    treeAction->setIcon(QIcon::fromTheme("dfm_viewlist_tree"));
    treeAction->setCheckable(true);

    // 菜单显示前同步勾选状态
    connect(menu, &DTK_WIDGET_NAMESPACE::DMenu::aboutToShow,
            this, &ViewSwitchButtonPrivate::updateCheckedState);

    connect(iconAction, &QAction::triggered, q, [this]() {
        emit q->viewModeChangeRequested(ViewMode::kIconMode);
    });
    connect(listAction, &QAction::triggered, q, [this]() {
        emit q->viewModeChangeRequested(ViewMode::kListMode);
    });
    connect(treeAction, &QAction::triggered, q, [this]() {
        emit q->viewModeChangeRequested(ViewMode::kTreeMode);
    });
}

void ViewSwitchButtonPrivate::updateCheckedState()
{
    if (iconAction)
        iconAction->setChecked(currentMode == ViewMode::kIconMode);
    if (listAction)
        listAction->setChecked(currentMode == ViewMode::kListMode);
    if (treeAction)
        treeAction->setChecked(currentMode == ViewMode::kTreeMode);
}

ViewSwitchButton::ViewSwitchButton(QWidget *parent)
    : DToolButton(parent), d(new ViewSwitchButtonPrivate(this))
{
}

ViewSwitchButton::~ViewSwitchButton() = default;

void ViewSwitchButton::setViewModeIcon(DFMBASE_NAMESPACE::Global::ViewMode mode)
{
    // 同时更新图标和内部记录的当前模式（供菜单勾选状态同步用）
    d->currentMode = mode;
    switch (mode) {
    case DFMBASE_NAMESPACE::Global::ViewMode::kIconMode:
        d->currentIconName = "dfm_viewlist_icons";
        break;
    case DFMBASE_NAMESPACE::Global::ViewMode::kListMode:
        d->currentIconName = "dfm_viewlist_details";
        break;
    case DFMBASE_NAMESPACE::Global::ViewMode::kTreeMode:
        d->currentIconName = "dfm_viewlist_tree";
        break;
    default:
        fmWarning() << "CompactButton: unknown view mode" << static_cast<int>(mode);
        return;
    }
    update();
}

void ViewSwitchButton::setViewModeActionEnabled(DFMBASE_NAMESPACE::Global::ViewMode mode, bool enabled)
{
    QAction *action = nullptr;
    switch (mode) {
    case DFMBASE_NAMESPACE::Global::ViewMode::kIconMode:
        action = d->iconAction;
        break;
    case DFMBASE_NAMESPACE::Global::ViewMode::kListMode:
        action = d->listAction;
        break;
    case DFMBASE_NAMESPACE::Global::ViewMode::kTreeMode:
        action = d->treeAction;
        break;
    default:
        fmWarning() << "CompactButton::setViewModeActionEnabled: unknown view mode" << static_cast<int>(mode);
        return;
    }
    if (action)
        action->setEnabled(enabled);
}

void ViewSwitchButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    const bool menuVisible = d->menu && d->menu->isVisible();

    // ---- 绘制整体按钮背景（hover/press 状态）----
    if (d->hoverFlag || menuVisible) {
        QColor bgColor;
        if (isDown()) {
            // 按下状态 - 20% 不透明度
            bgColor = isDarkTheme ? QColor(255, 255, 255, 51)
                                  : QColor(0, 0, 0, 51);
        } else {
            // 悬浮状态 - 10% 不透明度
            bgColor = isDarkTheme ? QColor(255, 255, 255, 26)
                                  : QColor(0, 0, 0, 26);
        }
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(bgColor);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 6, 6);
        painter.restore();
    }

    // ---- 计算各元素垂直居中位置 ----
    const int iconY = (height() - kCompactToolLeftIconSize) / 2;
    const int circleY = (height() - kCompactToolArrowCircleSize) / 2;
    const int circleX = kCompactToolPadding + kCompactToolLeftIconSize + kCompactToolSpacing;

    // ---- 左侧视图模式图标矩形 ----
    const QRect leftRect(kCompactToolPadding, iconY, kCompactToolLeftIconSize, kCompactToolLeftIconSize);

    // ---- 右侧圆形背景矩形 ----
    const QRect circleRect(circleX, circleY, kCompactToolArrowCircleSize, kCompactToolArrowCircleSize);

    // ---- 绘制右侧圆形背景 ----
    if (d->hoverFlag || menuVisible) {
        QColor circleColor;
        if (menuVisible) {
            // active 状态：较深的半透明圆形背景
            circleColor = isDarkTheme ? QColor(255, 255, 255, 40)
                                      : QColor(0, 0, 0, 30);
        } else {
            circleColor = isDarkTheme ? QColor(255, 255, 255, 30)
                                      : QColor(0, 0, 0, 20);
        }
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(circleColor);
        painter.drawEllipse(circleRect);
        painter.restore();
    }

    // ---- 绘制左侧视图模式图标 ----
    if (!d->currentIconName.isEmpty()) {
        const auto leftIcon = QIcon::fromTheme(d->currentIconName);
        painter.save();
        if (menuVisible) {
            painter.setPen(palette().highlight().color());
        }
        leftIcon.paint(&painter, leftRect);
        painter.restore();
    }

    // ---- 绘制右侧下拉箭头 ----
    const int arrowX = circleX + (kCompactToolArrowCircleSize - kCompactToolArrowSize) / 2;
    const int arrowY = (height() - kCompactToolArrowSize) / 2;
    const QRect arrowRect(arrowX, arrowY, kCompactToolArrowSize, kCompactToolArrowSize);

    QStyleOptionButton arrowOpt;
    arrowOpt.initFrom(this);
    arrowOpt.rect = arrowRect;

    painter.save();
    if (menuVisible) {
        // active 状态：箭头显示高亮色
        painter.setPen(palette().highlight().color());
    }
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOpt, &painter, this);
    painter.restore();
}

void ViewSwitchButton::mousePressEvent(QMouseEvent *event)
{
    DToolButton::mousePressEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
    }

    if (event->button() == Qt::LeftButton && d->menu) {
        update();
        d->menu->exec(mapToGlobal(rect().bottomLeft()));
        // 菜单关闭后（exec 返回），重置按下状态
        // 因为菜单弹出期间 mouseReleaseEvent 被拦截，isDown() 仍为 true
        if (isDown()) {
            setDown(false);
        }
    }
    update();
}

void ViewSwitchButton::enterEvent(QEnterEvent *event)
{
    DToolButton::enterEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void ViewSwitchButton::leaveEvent(QEvent *event)
{
    DToolButton::leaveEvent(event);
    if (d->hoverFlag) {
        d->hoverFlag = false;
        update();
    }
}

void ViewSwitchButton::mouseMoveEvent(QMouseEvent *event)
{
    DToolButton::mouseMoveEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void ViewSwitchButton::mouseReleaseEvent(QMouseEvent *event)
{
    DToolButton::mouseReleaseEvent(event);
    update();
}
