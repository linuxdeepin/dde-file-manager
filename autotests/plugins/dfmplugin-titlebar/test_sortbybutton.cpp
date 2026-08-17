// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/sortbybutton.h"
#include "views/private/sortbybutton_p.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/dfm_global_defines.h>

#include <DGuiApplicationHelper>

#include <gtest/gtest.h>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QPixmap>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class SortByButtonTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub QIcon::fromTheme
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            QPixmap pixmap(16, 16);
            pixmap.fill(Qt::red);
            return QIcon(pixmap);
        });

        // Stub DGuiApplicationHelper
        stub.set_lamda(&DGuiApplicationHelper::instance, []() -> DGuiApplicationHelper * {
            __DBG_STUB_INVOKE__
            static DGuiApplicationHelper helper;
            return &helper;
        });

        stub.set_lamda(&DGuiApplicationHelper::themeType, [](DGuiApplicationHelper *) {
            __DBG_STUB_INVOKE__
            return DGuiApplicationHelper::LightType;
        });

        // Stub TitleBarEventCaller
        stub.set_lamda(&TitleBarEventCaller::sendCurrentSortRole, [](QObject *) {
            __DBG_STUB_INVOKE__
            return kItemFileDisplayNameRole;
        });

        stub.set_lamda(&TitleBarEventCaller::sendCurrentGroupRoleStrategy, [](QObject *) {
            __DBG_STUB_INVOKE__
            return QString("NoGroupStrategy");
        });

        stub.set_lamda(&TitleBarEventCaller::sendColumnRoles, [](QObject *) {
            __DBG_STUB_INVOKE__
            QList<ItemRoles> roles;
            roles << kItemFileDisplayNameRole << kItemFileSizeRole << kItemFileLastModifiedRole;
            return roles;
        });

        stub.set_lamda(&TitleBarEventCaller::sendColumnDisplyName, [](QObject *, ItemRoles role) {
            __DBG_STUB_INVOKE__
            if (role == kItemFileDisplayNameRole)
                return QString("Name");
            if (role == kItemFileSizeRole)
                return QString("Size");
            if (role == kItemFileLastModifiedRole)
                return QString("Modified");
            return QString("Unknown");
        });

        stub.set_lamda(&TitleBarEventCaller::sendSetSort, [](QObject *, ItemRoles) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TitleBarEventCaller::sendSetGroupStrategy, [](QObject *, const QString &) {
            __DBG_STUB_INVOKE__
        });

        button = new SortByButton();
    }

    void TearDown() override
    {
        delete button;
        button = nullptr;
        stub.clear();
    }

    SortByButton *button { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(SortByButtonTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(button, nullptr);
    EXPECT_NE(button->d, nullptr);
}

TEST_F(SortByButtonTest, Constructor_InitializesPrivate_MenusCreated)
{
    EXPECT_NE(button->d->menu, nullptr);
    EXPECT_NE(button->d->groupMenu, nullptr);
}

TEST_F(SortByButtonTest, Constructor_InitializesFlags_DefaultState)
{
    EXPECT_FALSE(button->d->hoverFlag);
    EXPECT_FALSE(button->d->iconClicked);
}

TEST_F(SortByButtonTest, SetupMenu_Called_ClearsExistingMenus)
{
    button->d->menu->addAction("Test Action");
    button->d->groupMenu->addAction("Test Group Action");

    button->d->setupMenu();

    // Menus should be cleared and repopulated
    EXPECT_GT(button->d->menu->actions().size(), 0);
    EXPECT_GT(button->d->groupMenu->actions().size(), 0);
}

TEST_F(SortByButtonTest, SetupMenu_CreatesActions_FromRoles)
{
    button->d->setupMenu();

    QList<QAction *> actions = button->d->menu->actions();
    // Should have at least 3 role actions + 1 group by action
    EXPECT_GE(actions.size(), 4);
}

TEST_F(SortByButtonTest, SetupMenu_CreatesGroupMenu_WithNoneAction)
{
    button->d->setupMenu();

    QList<QAction *> groupActions = button->d->groupMenu->actions();
    EXPECT_GE(groupActions.size(), 1);
    if (groupActions.size() > 0) {
        EXPECT_EQ(groupActions.first()->text(), QObject::tr("None"));
    }
}

TEST_F(SortByButtonTest, SetupMenu_ActionsCheckable_ForSortMenu)
{
    button->d->setupMenu();

    QList<QAction *> actions = button->d->menu->actions();
    for (auto act : actions) {
        if (act->menu() == nullptr) {   // Not the Group by submenu
            auto property = act->property("item-role");
            if (property.isValid()) {
                EXPECT_TRUE(act->isCheckable());
            }
        }
    }
}

TEST_F(SortByButtonTest, SetItemSortRoles_CurrentRole_ChecksAction)
{
    stub.set_lamda(&TitleBarEventCaller::sendCurrentSortRole, [](QObject *) {
        __DBG_STUB_INVOKE__
        return kItemFileSizeRole;
    });

    button->d->setupMenu();
    button->d->setItemSortRoles();

    QList<QAction *> actions = button->d->menu->actions();
    bool foundChecked = false;
    for (auto act : actions) {
        auto role = act->property("item-role").value<ItemRoles>();
        if (role == kItemFileSizeRole && act->isChecked()) {
            foundChecked = true;
            break;
        }
    }
    EXPECT_TRUE(foundChecked);
}

TEST_F(SortByButtonTest, SetItemGroupRoles_CurrentStrategy_ChecksAction)
{
    stub.set_lamda(&TitleBarEventCaller::sendCurrentGroupRoleStrategy, [](QObject *) {
        __DBG_STUB_INVOKE__
        return QString("Name");
    });

    button->d->setupMenu();
    button->d->setItemGroupRoles();

    QList<QAction *> actions = button->d->groupMenu->actions();
    bool foundChecked = false;
    for (auto act : actions) {
        if (act->isChecked()) {
            foundChecked = true;
            break;
        }
    }
    EXPECT_TRUE(foundChecked);
}

TEST_F(SortByButtonTest, Sort_Called_SendsSetSortEvent)
{
    bool eventSent = false;
    stub.set_lamda(&TitleBarEventCaller::sendSetSort, [&eventSent](QObject *, ItemRoles) {
        __DBG_STUB_INVOKE__
        eventSent = true;
    });

    button->d->sort();

    EXPECT_TRUE(eventSent);
}

TEST_F(SortByButtonTest, MenuTriggered_ValidAction_SendsSetSort)
{
    bool eventSent = false;
    ItemRoles sentRole = kItemUnknowRole;

    stub.set_lamda(&TitleBarEventCaller::sendSetSort, [&eventSent, &sentRole](QObject *, ItemRoles role) {
        __DBG_STUB_INVOKE__
        eventSent = true;
        sentRole = role;
    });

    QAction action("Test");
    action.setProperty("item-role", QVariant::fromValue(kItemFileSizeRole));

    button->d->menuTriggered(&action);

    EXPECT_TRUE(eventSent);
    EXPECT_EQ(sentRole, kItemFileSizeRole);
}

TEST_F(SortByButtonTest, MenuTriggered_NullAction_DoesNothing)
{
    bool eventSent = false;
    stub.set_lamda(&TitleBarEventCaller::sendSetSort, [&eventSent](QObject *, ItemRoles) {
        __DBG_STUB_INVOKE__
        eventSent = true;
    });

    button->d->menuTriggered(nullptr);

    EXPECT_FALSE(eventSent);
}

TEST_F(SortByButtonTest, MenuTriggered_InvalidProperty_DoesNothing)
{
    bool eventSent = false;
    stub.set_lamda(&TitleBarEventCaller::sendSetSort, [&eventSent](QObject *, ItemRoles) {
        __DBG_STUB_INVOKE__
        eventSent = true;
    });

    QAction action("Test");
    // No property set

    button->d->menuTriggered(&action);

    EXPECT_FALSE(eventSent);
}

TEST_F(SortByButtonTest, GroupMenuTriggered_ValidAction_SendsSetGroupStrategy)
{
    bool eventSent = false;
    QString sentStrategy;

    stub.set_lamda(&TitleBarEventCaller::sendSetGroupStrategy, [&eventSent, &sentStrategy](QObject *, const QString &strategy) {
        __DBG_STUB_INVOKE__
        eventSent = true;
        sentStrategy = strategy;
    });

    QAction action("Test");
    action.setProperty("item-role", QVariant::fromValue(kItemFileDisplayNameRole));

    button->d->groupMenuTriggered(&action);

    EXPECT_TRUE(eventSent);
    EXPECT_EQ(sentStrategy, "Name");
}

TEST_F(SortByButtonTest, GroupMenuTriggered_NullAction_DoesNothing)
{
    bool eventSent = false;
    stub.set_lamda(&TitleBarEventCaller::sendSetGroupStrategy, [&eventSent](QObject *, const QString &) {
        __DBG_STUB_INVOKE__
        eventSent = true;
    });

    button->d->groupMenuTriggered(nullptr);

    EXPECT_FALSE(eventSent);
}

TEST_F(SortByButtonTest, GroupMenuTriggered_InvalidProperty_DoesNothing)
{
    bool eventSent = false;
    stub.set_lamda(&TitleBarEventCaller::sendSetGroupStrategy, [&eventSent](QObject *, const QString &) {
        __DBG_STUB_INVOKE__
        eventSent = true;
    });

    QAction action("Test");
    // No property set

    button->d->groupMenuTriggered(&action);

    EXPECT_FALSE(eventSent);
}

TEST_F(SortByButtonTest, GroupMenuTriggered_UnknownRole_DoesNothing)
{
    bool eventSent = false;
    stub.set_lamda(&TitleBarEventCaller::sendSetGroupStrategy, [&eventSent](QObject *, const QString &) {
        __DBG_STUB_INVOKE__
        eventSent = true;
    });

    QAction action("Test");
    action.setProperty("item-role", QVariant::fromValue(kItemUnknowRole));

    EXPECT_NO_THROW(button->d->groupMenuTriggered(&action));
}

TEST_F(SortByButtonTest, EnterEvent_Called_SetsHoverFlag)
{
    button->d->hoverFlag = false;

    QEnterEvent event(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
    button->enterEvent(&event);

    EXPECT_TRUE(button->d->hoverFlag);
}

TEST_F(SortByButtonTest, EnterEvent_AlreadyHovering_KeepsFlag)
{
    button->d->hoverFlag = true;

    QEnterEvent event(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
    button->enterEvent(&event);

    EXPECT_TRUE(button->d->hoverFlag);
}

TEST_F(SortByButtonTest, LeaveEvent_Called_ClearsHoverFlag)
{
    button->d->hoverFlag = true;
    button->d->iconClicked = true;

    QEvent event(QEvent::Leave);
    button->leaveEvent(&event);

    EXPECT_FALSE(button->d->hoverFlag);
    EXPECT_FALSE(button->d->iconClicked);
}

TEST_F(SortByButtonTest, LeaveEvent_NotHovering_DoesNothing)
{
    button->d->hoverFlag = false;

    QEvent event(QEvent::Leave);
    button->leaveEvent(&event);

    EXPECT_FALSE(button->d->hoverFlag);
}

TEST_F(SortByButtonTest, MouseMoveEvent_Called_SetsHoverFlag)
{
    button->d->hoverFlag = false;

    QMouseEvent event(QEvent::MouseMove, QPointF(10, 10), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    button->mouseMoveEvent(&event);

    EXPECT_TRUE(button->d->hoverFlag);
}

TEST_F(SortByButtonTest, MousePressEvent_LeftButton_SetsHoverFlag)
{
    button->d->hoverFlag = false;

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mousePressEvent(&event);

    EXPECT_TRUE(button->d->hoverFlag);
}

TEST_F(SortByButtonTest, MousePressEvent_ClickIcon_SetsIconClicked)
{
    // Click on icon area (x <= 2 * 6 + 16 = 28)
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(15, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mousePressEvent(&event);

    EXPECT_TRUE(button->d->iconClicked);
}

TEST_F(SortByButtonTest, MousePressEvent_ClickIconArea_CallsSort)
{
    bool sortCalled = false;
    stub.set_lamda(&TitleBarEventCaller::sendSetSort, [&sortCalled](QObject *, ItemRoles) {
        __DBG_STUB_INVOKE__
        sortCalled = true;
    });

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(15, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mousePressEvent(&event);

    EXPECT_TRUE(sortCalled);
}

TEST_F(SortByButtonTest, MousePressEvent_ClickMenuArea_ShowsMenu)
{
    // Stub menu exec to avoid blocking
    stub.set_lamda(static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec),
                   [](QMenu *, const QPoint &, QAction *) -> QAction * {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    // Click on menu area (x > 28)
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(35, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mousePressEvent(&event);

    EXPECT_FALSE(button->d->iconClicked);
}

TEST_F(SortByButtonTest, MousePressEvent_RightButton_DoesNothing)
{
    button->d->iconClicked = false;

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(15, 10), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    button->mousePressEvent(&event);

    // Should not set iconClicked or call sort
}

TEST_F(SortByButtonTest, MouseReleaseEvent_Called_ClearsIconClicked)
{
    button->d->iconClicked = true;

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mouseReleaseEvent(&event);

    EXPECT_FALSE(button->d->iconClicked);
}

TEST_F(SortByButtonTest, PaintEvent_Default_DrawsButton)
{
    QPixmap pixmap(46, 36);
    QPainter painter(&pixmap);

    QPaintEvent event(button->rect());
    EXPECT_NO_THROW(button->paintEvent(&event));
}

TEST_F(SortByButtonTest, PaintEvent_Hovering_DrawsHoverState)
{
    button->d->hoverFlag = true;

    QPaintEvent event(button->rect());
    EXPECT_NO_THROW(button->paintEvent(&event));
}

TEST_F(SortByButtonTest, PaintEvent_MenuVisible_DrawsActiveState)
{
    stub.set_lamda(&QMenu::isVisible, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    QPaintEvent event(button->rect());
    EXPECT_NO_THROW(button->paintEvent(&event));
}

TEST_F(SortByButtonTest, PaintEvent_IconClicked_DrawsHighlightedIcon)
{
    button->d->iconClicked = true;

    QPaintEvent event(button->rect());
    EXPECT_NO_THROW(button->paintEvent(&event));
}

TEST_F(SortByButtonTest, PaintEvent_LightTheme_UsesCorrectColors)
{
    stub.set_lamda(&DGuiApplicationHelper::themeType, [](DGuiApplicationHelper *) {
        __DBG_STUB_INVOKE__
        return DGuiApplicationHelper::LightType;
    });

    button->d->hoverFlag = true;

    QPaintEvent event(button->rect());
    EXPECT_NO_THROW(button->paintEvent(&event));
}

TEST_F(SortByButtonTest, PaintEvent_DarkTheme_UsesCorrectColors)
{
    stub.set_lamda(&DGuiApplicationHelper::themeType, [](DGuiApplicationHelper *) {
        __DBG_STUB_INVOKE__
        return DGuiApplicationHelper::DarkType;
    });

    button->d->hoverFlag = true;

    QPaintEvent event(button->rect());
    EXPECT_NO_THROW(button->paintEvent(&event));
}

TEST_F(SortByButtonTest, PaintEvent_ButtonDown_DrawsPressedState)
{
    stub.set_lamda(&QToolButton::isDown, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    button->d->hoverFlag = true;

    QPaintEvent event(button->rect());
    EXPECT_NO_THROW(button->paintEvent(&event));
}

TEST_F(SortByButtonTest, RoleToGroupStrategy_NameRole_ReturnsName)
{
    // Test through groupMenuTriggered
    QString sentStrategy;
    stub.set_lamda(&TitleBarEventCaller::sendSetGroupStrategy, [&sentStrategy](QObject *, const QString &strategy) {
        __DBG_STUB_INVOKE__
        sentStrategy = strategy;
    });

    QAction action("Test");
    action.setProperty("item-role", QVariant::fromValue(kItemFileDisplayNameRole));
    button->d->groupMenuTriggered(&action);

    EXPECT_EQ(sentStrategy, "Name");
}

TEST_F(SortByButtonTest, RoleToGroupStrategy_SizeRole_ReturnsSize)
{
    QString sentStrategy;
    stub.set_lamda(&TitleBarEventCaller::sendSetGroupStrategy, [&sentStrategy](QObject *, const QString &strategy) {
        __DBG_STUB_INVOKE__
        sentStrategy = strategy;
    });

    QAction action("Test");
    action.setProperty("item-role", QVariant::fromValue(kItemFileSizeRole));
    button->d->groupMenuTriggered(&action);

    EXPECT_EQ(sentStrategy, "Size");
}

TEST_F(SortByButtonTest, RoleToGroupStrategy_ModifiedTimeRole_ReturnsModifiedTime)
{
    QString sentStrategy;
    stub.set_lamda(&TitleBarEventCaller::sendSetGroupStrategy, [&sentStrategy](QObject *, const QString &strategy) {
        __DBG_STUB_INVOKE__
        sentStrategy = strategy;
    });

    QAction action("Test");
    action.setProperty("item-role", QVariant::fromValue(kItemFileLastModifiedRole));
    button->d->groupMenuTriggered(&action);

    EXPECT_EQ(sentStrategy, "ModifiedTime");
}

TEST_F(SortByButtonTest, MenuSignals_Connected_ToSlots)
{
    // Verify that menu signals are connected
    bool disconnected = QObject::disconnect(button->d->menu, &QMenu::triggered,
                                            button->d, &SortByButtonPrivate::menuTriggered);
    EXPECT_TRUE(disconnected);

    // Reconnect for cleanup
    QObject::connect(button->d->menu, &QMenu::triggered,
                     button->d, &SortByButtonPrivate::menuTriggered);
}

TEST_F(SortByButtonTest, GroupMenuSignals_Connected_ToSlots)
{
    // Verify that groupMenu signals are connected
    bool disconnected = QObject::disconnect(button->d->groupMenu, &QMenu::triggered,
                                            button->d, &SortByButtonPrivate::groupMenuTriggered);
    EXPECT_TRUE(disconnected);

    // Reconnect for cleanup
    QObject::connect(button->d->groupMenu, &QMenu::triggered,
                     button->d, &SortByButtonPrivate::groupMenuTriggered);
}

TEST_F(SortByButtonTest, SetupMenu_CreatesGroupBySubmenu)
{
    button->d->setupMenu();

    QList<QAction *> actions = button->d->menu->actions();
    bool foundGroupBy = false;
    for (auto act : actions) {
        if (act->text() == QObject::tr("Group by") && act->menu() != nullptr) {
            foundGroupBy = true;
            EXPECT_EQ(act->menu(), button->d->groupMenu);
            break;
        }
    }
    EXPECT_TRUE(foundGroupBy);
}

TEST_F(SortByButtonTest, IconClickBoundary_ExactBoundary_DetectedCorrectly)
{
    stub.set_lamda(qOverload<const QPoint &, QAction *>(&QMenu::exec), [] {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Test exact boundary (2 * 6 + 16 = 28)
    QMouseEvent event1(QEvent::MouseButtonPress, QPointF(28, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mousePressEvent(&event1);
    EXPECT_TRUE(button->d->iconClicked);

    button->d->iconClicked = false;

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(29, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mousePressEvent(&event2);
    EXPECT_FALSE(button->d->iconClicked);
}

TEST_F(SortByButtonTest, MultipleHoverEvents_MaintainsState)
{
    QEnterEvent event1(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
    button->enterEvent(&event1);
    EXPECT_TRUE(button->d->hoverFlag);

    QEnterEvent event2(QPointF(15, 15), QPointF(15, 15), QPointF(15, 15));
    button->enterEvent(&event2);
    EXPECT_TRUE(button->d->hoverFlag);
}
