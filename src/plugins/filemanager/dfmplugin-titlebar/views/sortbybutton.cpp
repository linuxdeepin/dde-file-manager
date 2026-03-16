// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/sortbybutton_p.h"
#include "views/sortbybutton.h"
#include "events/titlebareventcaller.h"

#include <dfm-framework/event/event.h>

#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>

#include <QDebug>
#include <QMenu>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOptionButton>
#include <QMouseEvent>
#include <QStylePainter>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static constexpr int kSortToolPadding { 6 };   // 按钮整体边距
static constexpr int kSortToolSpacing { 6 };   // 内部元素间距
static constexpr int kSortToolLeftButtonSize { 16 };   // 左侧排序图标尺寸
static constexpr int kSortToolArrowCircleSize { 18 };   // 右侧箭头圆形背景尺寸
static constexpr int kSortToolArrowButtonSize { 12 };   // 右侧箭头图标尺寸
static constexpr char kItemRole[] = "item-role";
// 按钮总宽 = padding + iconSize + spacing + circleSize + padding
static constexpr int kSortToolButtonWidth { kSortToolPadding + kSortToolLeftButtonSize
                                            + kSortToolSpacing + kSortToolArrowCircleSize
                                            + kSortToolPadding };

namespace GroupStrategy {
inline constexpr char kNoGroup[] { "NoGroupStrategy" };
inline constexpr char kName[] { "Name" };
inline constexpr char kSize[] { "Size" };
inline constexpr char kModifiedTime[] { "ModifiedTime" };
inline constexpr char kCreatedTime[] { "CreatedTime" };
inline constexpr char kType[] { "Type" };
inline constexpr char kCustomPath[] { "CustomPath" };
inline constexpr char kCustomTime[] { "CustomTime" };
}   // namespace GroupStrategy

// TODO: This is workaround
static QString roleToGroupingStrategy(const DFMGLOBAL_NAMESPACE::ItemRoles &role)
{
    static const QHash<DFMGLOBAL_NAMESPACE::ItemRoles, QString> kMapping = {
        { kItemFileDisplayNameRole, GroupStrategy::kName },
        { kItemFileLastModifiedRole, GroupStrategy::kModifiedTime },
        { kItemFileCreatedRole, GroupStrategy::kCreatedTime },
        { kItemFileSizeRole, GroupStrategy::kSize },
        { kItemFileMimeTypeRole, GroupStrategy::kType },
        { kItemFileOriginalPath, GroupStrategy::kCustomPath },
        { kItemFilePathRole, GroupStrategy::kCustomPath },
        { kItemFileDeletionDate, GroupStrategy::kCustomTime },
        { kItemFileLastReadRole, GroupStrategy::kCustomTime }
    };

    return kMapping.value(role, "NoGroupStrategy");
}

SortByButtonPrivate::SortByButtonPrivate(SortByButton *parent)
    : QObject(parent),
      q(parent)
{
    initializeUi();
    initConnect();
}

void SortByButtonPrivate::setItemSortRoles()
{
    DFMGLOBAL_NAMESPACE::ItemRoles sortRole = TitleBarEventCaller::sendCurrentSortRole(q);
    auto actList = menu->actions();
    for (auto act : actList) {
        auto role = act->property(kItemRole).value<DFMGLOBAL_NAMESPACE::ItemRoles>();
        if (role == sortRole) {
            act->setChecked(true);
            break;
        }
    }
}

void SortByButtonPrivate::setItemGroupRoles()
{
    const QString groupStrategy = TitleBarEventCaller::sendCurrentGroupRoleStrategy(q);
    auto actList = groupMenu->actions();
    for (auto act : actList) {
        auto role = act->property(kItemRole).value<DFMGLOBAL_NAMESPACE::ItemRoles>();
        const auto &strategy = roleToGroupingStrategy(role);
        if (strategy == groupStrategy) {
            act->setChecked(true);
            break;
        }
    }
}

void SortByButtonPrivate::sort()
{
    TitleBarEventCaller::sendSetSort(q, TitleBarEventCaller::sendCurrentSortRole(q));
}

SortByButtonPrivate::~SortByButtonPrivate() = default;

void SortByButtonPrivate::setupMenu()
{
    menu->clear();
    groupMenu->clear();

    auto noneAct = groupMenu->addAction(QObject::tr("None"));
    noneAct->setProperty(kItemRole, DFMGLOBAL_NAMESPACE::ItemRoles::kItemUnknowRole);

    auto roleList = TitleBarEventCaller::sendColumnRoles(q);
    for (const auto &role : std::as_const(roleList)) {
        const auto &name = TitleBarEventCaller::sendColumnDisplyName(q, role);
        auto act = menu->addAction(name);
        act->setCheckable(true);
        act->setProperty(kItemRole, role);

        act = groupMenu->addAction(name);
        act->setCheckable(true);
        act->setProperty(kItemRole, role);
    }

    // Add Group by submenu
    auto groupByAction = menu->addAction(QObject::tr("Group by"));
    groupByAction->setMenu(groupMenu);
}

void SortByButtonPrivate::initializeUi()
{
    q->setFixedSize(kSortToolButtonWidth, kToolButtonSize);   // 按钮宽度由常量自动计算
    menu = new QMenu(q);
    groupMenu = new QMenu(q);
}

void SortByButtonPrivate::initConnect()
{
    connect(menu, &QMenu::triggered, this, &SortByButtonPrivate::menuTriggered);
    connect(groupMenu, &QMenu::triggered, this, &SortByButtonPrivate::groupMenuTriggered);
}

void SortByButtonPrivate::menuTriggered(QAction *action)
{
    if (!action)
        return;

    auto property = action->property(kItemRole);
    if (!property.isValid()) {
        fmWarning() << "Invalid action property in menuTriggered";
        return;
    }

    auto role = property.value<DFMGLOBAL_NAMESPACE::ItemRoles>();
    TitleBarEventCaller::sendSetSort(q, role);
}

void SortByButtonPrivate::groupMenuTriggered(QAction *action)
{
    if (!action)
        return;

    auto property = action->property(kItemRole);
    if (!property.isValid()) {
        fmWarning() << "Invalid action property in groupMenuTriggered";
        return;
    }

    auto role = property.value<DFMGLOBAL_NAMESPACE::ItemRoles>();
    const auto &strategy = roleToGroupingStrategy(role);
    if (strategy.isEmpty()) {
        fmWarning() << "No strategy found for role:" << static_cast<int>(role);
        return;
    }

    TitleBarEventCaller::sendSetGroupStrategy(q, strategy);
}

SortByButton::SortByButton(QWidget *parent)
    : DToolButton(parent), d(new SortByButtonPrivate(this))
{
}

SortByButton::~SortByButton() = default;

void SortByButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    bool menuVisible = d->menu->isVisible();

    // 绘制整体按钮背景（hover/press 状态）
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

    // 计算内容区垂直居中位置
    int iconY = (height() - kSortToolLeftButtonSize) / 2;
    int circleY = (height() - kSortToolArrowCircleSize) / 2;

    // 左侧排序图标矩形
    QRect leftRect(kSortToolPadding, iconY, kSortToolLeftButtonSize, kSortToolLeftButtonSize);

    // 右侧圆形背景矩形
    int circleX = kSortToolPadding + kSortToolLeftButtonSize + kSortToolSpacing;
    QRect circleRect(circleX, circleY, kSortToolArrowCircleSize, kSortToolArrowCircleSize);

    // 绘制右侧圆形背景
    if (d->hoverFlag || menuVisible) {
        QColor circleColor;
        if (menuVisible) {
            // active 状态：高亮色半透明圆形背景
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

    // 绘制左侧排序图标
    auto leftIcon = QIcon::fromTheme("dfm_sortby_arrange");
    painter.save();
    if (d->iconClicked) {
        painter.setPen(palette().highlight().color());   // Use highlight color for icon when clicked
    }
    leftIcon.paint(&painter, leftRect);
    painter.restore();

    // 绘制右侧下拉箭头
    int arrowSize = kSortToolArrowButtonSize;
    int arrowX = circleX + (kSortToolArrowCircleSize - arrowSize) / 2;
    int arrowY = (height() - arrowSize) / 2;
    QRect arrowRect(arrowX, arrowY, arrowSize, arrowSize);

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

void SortByButton::mousePressEvent(QMouseEvent *event)
{
    DToolButton::mousePressEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
    if (event->button() == Qt::LeftButton) {
        int leftWidth = 2 * kSortToolPadding + kSortToolLeftButtonSize;
        d->iconClicked = event->position().x() <= leftWidth;   // Check if icon area is clicked

        if (event->position().x() > leftWidth && d->menu) {
            // Query current tab's busy state before showing menu
            bool isBusy = TitleBarEventCaller::sendGetCurrentModelBusy(this);

            d->setupMenu();
            d->setItemSortRoles();
            d->setItemGroupRoles();

            // Disable all actions if model is busy
            if (isBusy) {
                for (QAction *action : d->menu->actions()) {
                    action->setEnabled(false);
                }
                fmDebug() << "SortByButton: Menu actions disabled - current tab model is busy";
            }

            d->menu->exec(mapToGlobal(rect().bottomLeft()));
            // 菜单关闭后（exec 返回），重置按下状态
            if (isDown()) {
                setDown(false);
                update();
            }
        } else if (d->iconClicked) {
            // Check busy state before sorting
            bool isBusy = TitleBarEventCaller::sendGetCurrentModelBusy(this);
            if (isBusy) {
                fmDebug() << "SortByButton: Sort operation blocked - current tab model is busy";
                return;
            }
            d->sort();
        }
        update();   // Trigger repaint
    }
}

void SortByButton::enterEvent(QEnterEvent *event)
{
    DToolButton::enterEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void SortByButton::leaveEvent(QEvent *event)
{
    DToolButton::leaveEvent(event);
    if (d->hoverFlag) {
        d->hoverFlag = false;
        d->iconClicked = false;   // Reset icon click state
        update();   // Trigger repaint to restore colors
    }
}

void SortByButton::mouseMoveEvent(QMouseEvent *event)
{
    DToolButton::mouseMoveEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void SortByButton::mouseReleaseEvent(QMouseEvent *event)
{
    DToolButton::mouseReleaseEvent(event);
    d->iconClicked = false;   // Reset icon click state
    update();   // Trigger repaint to restore colors
}
