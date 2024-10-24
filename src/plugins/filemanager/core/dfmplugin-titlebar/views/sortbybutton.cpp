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
static constexpr char kSortToolSize[] = "sort-by-size";
static constexpr char kSortToolType[] = "sort-by-type";

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

    action = menu->addAction(tr("Size"));
    action->setObjectName(kSortToolSize);
    action->setCheckable(true);
    actionGroup->addAction(action);

    action = menu->addAction(tr("Type"));
    action->setObjectName(kSortToolType);
    action->setCheckable(true);
    actionGroup->addAction(action);
}

void SortByButtonPrivate::initConnect()
{
    connect(menu, &QMenu::triggered, this, &SortByButtonPrivate::menuTriggered);
}

void SortByButtonPrivate::menuTriggered(QAction *action)
{
    if (!action)
        return;
    if (action->objectName() == kSortToolName) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole);
    } else if (action->objectName() == kSortToolTimeModified) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileLastModifiedRole);
    } else if (action->objectName() == kSortToolSize) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileSizeRole);
    } else if (action->objectName() == kSortToolType) {
        TitleBarEventCaller::sendSetSort(q, DFMGLOBAL_NAMESPACE::kItemFileMimeTypeRole);
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
        style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);
    }

    // 绘制左侧图标
    int leftIconY = kSortToolVMargin + (height() - 2 * kSortToolVMargin - kSortToolLeftButtonSize) / 2;
    QRect leftRect(kSortToolHMargin, leftIconY, kSortToolLeftButtonSize, kSortToolLeftButtonSize);
    auto leftIcon = QIcon::fromTheme("dfm_sortby_arrange");
    leftIcon.paint(&painter, leftRect);

    // 绘制右侧箭头
    int arrowY = kSortToolVMargin + (height() - 2 * kSortToolVMargin - kSortToolArrowButtonSize) / 2;
    QRect rightRect(width() - kSortToolHMargin - kSortToolArrowButtonSize, arrowY, kSortToolArrowButtonSize, kSortToolArrowButtonSize);
    opt.rect = rightRect;
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
        if (event->x() > leftWidth) {   // 右侧区域
            if (d->menu) {
                d->setItemSortRoles();
                d->menu->exec(mapToGlobal(rect().bottomLeft()));
            }
        } else {
            d->sort();
        }
        update();   // 触发重绘
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
        update();
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
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}
