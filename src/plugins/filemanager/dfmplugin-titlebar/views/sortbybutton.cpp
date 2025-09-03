// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
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
#include <QActionGroup>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static constexpr int kSortToolButtonWidth { 46 };
static constexpr int kSortToolLeftButtonSize { 16 };
static constexpr int kSortToolArrowButtonSize { 12 };
static constexpr int kSortToolHMargin { 6 };
static constexpr int kSortToolVMargin { 9 };
static constexpr char kSortToolName[] = "sort-by-name";
static constexpr char kSortToolTimeModified[] = "sort-by-time-modified";
static constexpr char kSortToolTimeCreated[] = "sort-by-time-created";
static constexpr char kSortToolSize[] = "sort-by-size";
static constexpr char kSortToolType[] = "sort-by-type";

// Group by constants
static constexpr char kGroupToolNone[] = "group-by-none";
static constexpr char kGroupToolName[] = "group-by-name";
static constexpr char kGroupToolTimeModified[] = "group-by-time-modified";
static constexpr char kGroupToolTimeCreated[] = "group-by-time-created";
static constexpr char kGroupToolSize[] = "group-by-size";
static constexpr char kGroupToolType[] = "group-by-type";

SortByButtonPrivate::SortByButtonPrivate(SortByButton *parent)
    : QObject(parent), q(parent)
{
    initializeUi();
    initConnect();
}

void SortByButtonPrivate::setItemSortRoles()
{
    DFMGLOBAL_NAMESPACE::ItemRoles sortRole = TitleBarEventCaller::sendCurrentSortRole(q);
    switch (sortRole) {
    case DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole: {
        auto action = menu->findChild<QAction *>(kSortToolName);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileLastModifiedRole: {
        auto action = menu->findChild<QAction *>(kSortToolTimeModified);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileCreatedRole: {
        auto action = menu->findChild<QAction *>(kSortToolTimeCreated);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileSizeRole: {
        auto action = menu->findChild<QAction *>(kSortToolSize);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileMimeTypeRole: {
        auto action = menu->findChild<QAction *>(kSortToolType);
        if (action) {
            action->setChecked(true);
        }
    } break;
    default:
        break;
    }
}

void SortByButtonPrivate::setItemGroupRoles()
{
    DFMGLOBAL_NAMESPACE::ItemRoles groupRole = TitleBarEventCaller::sendCurrentGroupRole(q);
    switch (groupRole) {
    case DFMGLOBAL_NAMESPACE::kItemUnknowRole: {
        auto action = groupMenu->findChild<QAction *>(kGroupToolNone);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole: {
        auto action = groupMenu->findChild<QAction *>(kGroupToolName);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileLastModifiedRole: {
        auto action = groupMenu->findChild<QAction *>(kGroupToolTimeModified);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileCreatedRole: {
        auto action = groupMenu->findChild<QAction *>(kGroupToolTimeCreated);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileSizeRole: {
        auto action = groupMenu->findChild<QAction *>(kGroupToolSize);
        if (action) {
            action->setChecked(true);
        }
    } break;
    case DFMGLOBAL_NAMESPACE::kItemFileMimeTypeRole: {
        auto action = groupMenu->findChild<QAction *>(kGroupToolType);
        if (action) {
            action->setChecked(true);
        }
    } break;
    default:
        break;
    }
}

void SortByButtonPrivate::sort()
{
    TitleBarEventCaller::sendSetSort(q, TitleBarEventCaller::sendCurrentSortRole(q));
}

SortByButtonPrivate::~SortByButtonPrivate() = default;

void SortByButtonPrivate::initializeUi()
{
    q->setFixedSize(kSortToolButtonWidth, kToolButtonSize);   // 设置固定大小

    auto actionGroup = new QActionGroup(q);
    menu = new QMenu(q);
    auto action = menu->addAction(tr("Name"));
    action->setObjectName(kSortToolName);
    action->setCheckable(true);
    actionGroup->addAction(action);

    action = menu->addAction(tr("Time modified"));
    action->setObjectName(kSortToolTimeModified);
    action->setCheckable(true);
    actionGroup->addAction(action);

    action = menu->addAction(tr("Time created"));
    action->setObjectName(kSortToolTimeCreated);
    action->setCheckable(true);
    actionGroup->addAction(action);

    action = menu->addAction(tr("Size"));
    action->setObjectName(kSortToolSize);
    action->setCheckable(true);
    actionGroup->addAction(action);

    action = menu->addAction(tr("Type"));
    action->setObjectName(kSortToolType);
    action->setCheckable(true);
    actionGroup->addAction(action);

    // Add Group by submenu
    auto groupByAction = menu->addAction(tr("Group by"));
    groupMenu = new QMenu(q);
    groupByAction->setMenu(groupMenu);

    auto groupActionGroup = new QActionGroup(q);
    
    action = groupMenu->addAction(tr("None"));
    action->setObjectName(kGroupToolNone);
    action->setCheckable(true);
    groupActionGroup->addAction(action);

    action = groupMenu->addAction(tr("Name"));
    action->setObjectName(kGroupToolName);
    action->setCheckable(true);
    groupActionGroup->addAction(action);

    action = groupMenu->addAction(tr("Time modified"));
    action->setObjectName(kGroupToolTimeModified);
    action->setCheckable(true);
    groupActionGroup->addAction(action);

    action = groupMenu->addAction(tr("Time created"));
    action->setObjectName(kGroupToolTimeCreated);
    action->setCheckable(true);
    groupActionGroup->addAction(action);

    action = groupMenu->addAction(tr("Size"));
    action->setObjectName(kGroupToolSize);
    action->setCheckable(true);
    groupActionGroup->addAction(action);

    action = groupMenu->addAction(tr("Type"));
    action->setObjectName(kGroupToolType);
    action->setCheckable(true);
    groupActionGroup->addAction(action);
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
    if (action->objectName() == kSortToolName) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole);
    } else if (action->objectName() == kSortToolTimeModified) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileLastModifiedRole);
    } else if (action->objectName() == kSortToolTimeCreated) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileCreatedRole);
    } else if (action->objectName() == kSortToolSize) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileSizeRole);
    } else if (action->objectName() == kSortToolType) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileMimeTypeRole);
    }
}

void SortByButtonPrivate::groupMenuTriggered(QAction *action)
{
    if (!action)
        return;
    if (action->objectName() == kGroupToolNone) {
        TitleBarEventCaller::sendSetGroup(q, DFMGLOBAL_NAMESPACE::kItemUnknowRole);
    } else if (action->objectName() == kGroupToolName) {
        TitleBarEventCaller::sendSetGroup(q, DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole);
    } else if (action->objectName() == kGroupToolTimeModified) {
        TitleBarEventCaller::sendSetGroup(q, DFMGLOBAL_NAMESPACE::kItemFileLastModifiedRole);
    } else if (action->objectName() == kGroupToolTimeCreated) {
        TitleBarEventCaller::sendSetGroup(q, DFMGLOBAL_NAMESPACE::kItemFileCreatedRole);
    } else if (action->objectName() == kGroupToolSize) {
        TitleBarEventCaller::sendSetGroup(q, DFMGLOBAL_NAMESPACE::kItemFileSizeRole);
    } else if (action->objectName() == kGroupToolType) {
        TitleBarEventCaller::sendSetGroup(q, DFMGLOBAL_NAMESPACE::kItemFileMimeTypeRole);
    }
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

    QStyleOptionButton opt;
    opt.initFrom(this);

    if (d->hoverFlag || d->menu->isVisible()) {
        QStylePainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QStyleOptionToolButton option;
        QToolButton::initStyleOption(&option);
        option.state |= QStyle::State_MouseOver;

        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;

        QColor hoverColor;
        if (isDown()) {
            // 按下状态 - 20%不透明度
            hoverColor = isDarkTheme ? QColor(255, 255, 255, 51)
                                     : QColor(0, 0, 0, 51);
        } else {
            // 悬浮状态 - 10%不透明度
            hoverColor = isDarkTheme ? QColor(255, 255, 255, 26)
                                     : QColor(0, 0, 0, 26);
        }

        option.palette.setBrush(QPalette::Button, hoverColor);

        option.rect.adjust(1, -1, -1, 1);
        painter.drawComplexControl(QStyle::CC_ToolButton, option);
    }

    // Draw left icon
    int leftIconY = kSortToolVMargin + (height() - 2 * kSortToolVMargin - kSortToolLeftButtonSize) / 2;
    QRect leftRect(kSortToolHMargin, leftIconY, kSortToolLeftButtonSize, kSortToolLeftButtonSize);
    auto leftIcon = QIcon::fromTheme("dfm_sortby_arrange");

    painter.save();
    if (d->iconClicked) {
        painter.setPen(palette().highlight().color()); // Use highlight color for icon when clicked
    }
    leftIcon.paint(&painter, leftRect);
    painter.restore();

    // Draw right arrow
    int arrowY = kSortToolVMargin + (height() - 2 * kSortToolVMargin - kSortToolArrowButtonSize) / 2;
    QRect rightRect(width() - kSortToolHMargin - kSortToolArrowButtonSize, arrowY, kSortToolArrowButtonSize, kSortToolArrowButtonSize);
    opt.rect = rightRect;

    if (d->menu->isVisible()) {
        painter.setPen(palette().highlight().color()); // Use highlight color for arrow when menu visible
    }
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, &painter, this);
}

void SortByButton::mousePressEvent(QMouseEvent *event)
{
    DToolButton::mousePressEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
    if (event->button() == Qt::LeftButton) {
        int leftWidth = 2 * kSortToolHMargin + kSortToolLeftButtonSize;
        d->iconClicked = event->x() <= leftWidth; // Check if icon area is clicked

        if (event->x() > leftWidth && d->menu) {
            d->setItemSortRoles();
            d->setItemGroupRoles();
            d->menu->exec(mapToGlobal(rect().bottomLeft()));
        } else if (d->iconClicked) {
            d->sort();
        }
        update();   // Trigger repaint
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SortByButton::enterEvent(QEnterEvent *event)
#else
void SortByButton::enterEvent(QEvent *event)
#endif
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
        d->iconClicked = false; // Reset icon click state
        update(); // Trigger repaint to restore colors
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
    d->iconClicked = false; // Reset icon click state
    update(); // Trigger repaint to restore colors
}
