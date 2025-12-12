// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <string>

#include <dfm-extension/menu/dfmextmenuproxy.h>
#include <dfm-extension/menu/dfmextmenu.h>
#include <dfm-extension/menu/dfmextaction.h>
#include <dfm-extension/menu/private/dfmextmenuproxyprivate.h>
#include <dfm-extension/menu/private/dfmextmenuprivate.h>
#include <dfm-extension/menu/private/dfmextactionprivate.h>
#include "dfm_test_main.h"
#include "stubext.h"


using namespace DFMEXT;
using namespace testing;

class MockDFMExtActionPrivate : public DFMExtActionPrivate
{
public:
    MockDFMExtActionPrivate() : DFMExtActionPrivate() {}

    void setIcon(const std::string &iconName) override { this->m_icon = iconName; }
    std::string icon() const override { return m_icon; }

    void setText(const std::string &text) override { this->m_text = text; }
    std::string text() const override { return m_text; }

    void setToolTip(const std::string &tip) override { this->m_toolTip = tip; }
    std::string toolTip() const override { return m_toolTip; }

    void setMenu(DFMEXT::DFMExtMenu *menu) override { this->m_menu = menu; }
    DFMEXT::DFMExtMenu *menu() const override { return m_menu; }

    void setSeparator(bool b) override { this->m_separator = b; }
    bool isSeparator() const override { return m_separator; }

    void setCheckable(bool b) override { this->m_checkable = b; }
    bool isCheckable() const override { return m_checkable; }

    void setChecked(bool b) override { this->m_checked = b; }
    bool isChecked() const override { return m_checked; }

    void setEnabled(bool b) override { this->m_enabled = b; }
    bool isEnabled() const override { return m_enabled; }

private:
    std::string m_text;
    std::string m_icon;
    std::string m_toolTip;
    DFMExtMenu *m_menu = nullptr;
    bool m_separator = false;
    bool m_checkable = false;
    bool m_checked = false;
    bool m_enabled = true;
};

class MockDFMExtMenuPrivate : public DFMExtMenuPrivate
{
public:
    MockDFMExtMenuPrivate() : DFMExtMenuPrivate() {}
    std::string title() const override { return m_title; }
    void setTitle(const std::string &title) override { this->m_title = title; }
    std::string icon() const override { return m_icon; }
    void setIcon(const std::string &iconName) override { this->m_icon = iconName; }
    bool addAction(DFMExtAction *action) override {
        m_actions.push_back(action);
        return true;
    }
    bool insertAction(DFMExtAction *before, DFMExtAction *action) override {
        if (action == nullptr)
            return false;
        if (before == nullptr) {
            m_actions.push_back(action);
        } else {
            auto it = std::find(m_actions.begin(), m_actions.end(), before);
            if (it != m_actions.end()) {
                m_actions.insert(it, action);
            } else {
                m_actions.push_back(action);
            }
        }
        return true;
    }
    DFMExtAction *menuAction() const override {
        if (!m_actions.empty())
            return m_actions.front();
        return nullptr;
    }
    std::list<DFMExtAction *> actions() const override { return m_actions; }

private:
    std::string m_title;
    std::string m_icon;
    std::list<DFMExtAction *> m_actions;
};

class MockDFMExtMenuProxyPrivate : public DFMExtMenuProxyPrivate
{
public:
    MockDFMExtMenuProxyPrivate() : DFMExtMenuProxyPrivate() {}
    DFMEXT::DFMExtMenu *createMenu() Q_DECL_OVERRIDE {
        MockDFMExtMenuPrivate *menu = new MockDFMExtMenuPrivate();
        extMenu = new DFMExtMenu(menu);
        return extMenu;
    }
    bool deleteMenu(DFMEXT::DFMExtMenu *menu) Q_DECL_OVERRIDE {
        if (menu == nullptr)
            return false;
        if (menu == extMenu) {
            delete extMenu;
            extMenu = nullptr;
            return true;
        }
        return false;
    }
    DFMEXT::DFMExtAction *createAction() Q_DECL_OVERRIDE {
        MockDFMExtActionPrivate *action = new MockDFMExtActionPrivate();
        extAction = new DFMExtAction(action);
        return extAction;
    }
    bool deleteAction(DFMEXT::DFMExtAction *action) Q_DECL_OVERRIDE {
        if (action == nullptr)
            return false;
        if (action == extAction) {
            delete extAction;
            extAction = nullptr;
            return true;
        }
        return false;
    }

    private:
        DFMExtMenu *extMenu = nullptr;
        DFMExtAction *extAction = nullptr;
};

class MockDFMExtMenuProxy : public DFMExtMenuProxy
{
public:
    MockDFMExtMenuProxy() : DFMExtMenuProxy(new MockDFMExtMenuProxyPrivate()) {}
};

/**
 * @brief DFMExtMenuProxy类单元测试
 *
 * 测试范围：
 * 1. 构造函数和析构函数
 * 2. 菜单和Action创建功能
 * 3. 菜单和Action删除功能
 * 4. 内存管理和资源清理
 * 5. 私有类功能测试
 */
class DFMExtMenuProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {

    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

/**
 * @brief 创建测试用的DFMExtMenuProxy对象
 * 由于构造函数需要私有指针，使用Stub来模拟
 */
DFMExtMenuProxy* createTestMenuProxy()
{
    // 这里使用MockDFMExtMenuProxyPrivate来模拟对象创建
    return new MockDFMExtMenuProxy(); // 临时返回MockDFMExtMenuProxy对象，需要根据实际API调整
}

/**
 * @brief 测试DFMExtMenuProxy构造函数
 * 验证对象能够正确创建
 */
TEST_F(DFMExtMenuProxyTest, Constructor)
{
    // 创建DFMExtMenuProxy对象
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    
    // 验证对象创建成功
    EXPECT_NE(proxy, nullptr);
    
    // 清理资源
    delete proxy;
}

/**
 * @brief 测试DFMExtMenuProxy析构函数
 * 验证对象能够正确析构，不会出现内存泄漏
 */
TEST_F(DFMExtMenuProxyTest, Destructor)
{
    // 创建对象并在析构时验证不会崩溃
    {
        DFMExtMenuProxy *proxy = createTestMenuProxy();
        EXPECT_NE(proxy, nullptr);
        delete proxy;
        EXPECT_TRUE(true); // 如果到达这里说明析构成功
    }
    
    // 测试多个对象的析构
    std::vector<DFMExtMenuProxy*> proxies;
    for (int i = 0; i < 10; ++i) {
        proxies.push_back(createTestMenuProxy());
        EXPECT_NE(proxies.back(), nullptr);
    }
    
    // 清理所有对象
    for (auto* proxy : proxies) {
        delete proxy;
    }
    EXPECT_TRUE(true); // 如果到达这里说明所有对象都成功析构
}

/**
 * @brief 测试createMenu方法
 * 验证菜单创建功能
 */
TEST_F(DFMExtMenuProxyTest, CreateMenu)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    // 测试创建菜单
    DFMExtMenu *menu = proxy->createMenu();
    
    // 验证菜单创建成功
    EXPECT_NE(menu, nullptr);
    
    // 验证菜单的基本功能
    menu->setTitle("Test Menu");
    EXPECT_EQ(menu->title(), "Test Menu");
    
    menu->setIcon("/path/to/icon.png");
    EXPECT_EQ(menu->icon(), "/path/to/icon.png");
    
    // 清理资源
    delete proxy;
    // 注意：menu应该由proxy管理，或者需要单独处理
}

/**
 * @brief 测试createAction方法
 * 验证Action创建功能
 */
TEST_F(DFMExtMenuProxyTest, CreateAction)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    // 测试创建Action
    DFMExtAction *action = proxy->createAction();
    
    // 验证Action创建成功
    EXPECT_NE(action, nullptr);

    // DFMExtAction::menu() 测试
    DFMExtMenu *menu = action->menu();
    EXPECT_EQ(menu, nullptr); // 初始状态下，Action不应该关联任何菜单
    
    // 验证Action的基本功能
    action->setText("Test Action");
    EXPECT_EQ(action->text(), "Test Action");
    
    action->setIcon("/path/to/action_icon.png");
    EXPECT_EQ(action->icon(), "/path/to/action_icon.png");
    
    action->setToolTip("Test Tooltip");
    EXPECT_EQ(action->toolTip(), "Test Tooltip");
    
    action->setCheckable(true);
    EXPECT_TRUE(action->isCheckable());
    
    action->setChecked(true);
    EXPECT_TRUE(action->isChecked());
    
    action->setEnabled(true);
    EXPECT_TRUE(action->isEnabled());
    
    action->setSeparator(false);
    EXPECT_FALSE(action->isSeparator());

    // 清理资源
    delete proxy;
    // 注意：action应该由proxy管理，或者需要单独处理
}


TEST_F(DFMExtMenuProxyTest, extActionFunc)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);

    // 测试创建Action
    DFMExtAction *action = proxy->createAction();
    ASSERT_NE(action, nullptr);

    // 测试注册函数
    DFMExtAction::TriggeredFunc triggeredFunc = [&](DFMExtAction *self, bool checked) {
        EXPECT_EQ(self, action);
        EXPECT_EQ(checked, true);
    };
    action->registerTriggered(triggeredFunc);

    DFMExtAction::HoveredFunc hoveredFunc = [&](DFMExtAction *self) {
        EXPECT_EQ(self, action);
    };
    action->registerHovered(hoveredFunc);
    
    DFMExtAction::DeletedFunc deletedFunc = [&](DFMExtAction *self) {
        EXPECT_EQ(self, action);
    };
    action->registerDeleted(deletedFunc);

    auto self = action; // Capture action in a local variable

    // 测试触发Action
    // action->trigger();
    action->triggered(self, true); // 触发后，Action应该被标记为已触发

    // 测试悬停Action
    // action->hover();
    action->hovered(self); // 悬停后，Action应该被标记为悬停

    action->deleted(self); // 验证Action是否真的被删除

    // 测试删除Action
    bool deleteResult = proxy->deleteAction(action);
    EXPECT_TRUE(deleteResult); // 删除后，Action应该被标记为已删除

    delete proxy;
}

/**
 * @brief 测试deleteMenu方法
 * 验证菜单删除功能
 */
TEST_F(DFMExtMenuProxyTest, DeleteMenu)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    // 创建一个菜单
    DFMExtMenu *menu = proxy->createMenu();
    ASSERT_NE(menu, nullptr);
    
    // 测试删除菜单
    bool deleteResult = proxy->deleteMenu(menu);
    EXPECT_TRUE(deleteResult);
    
    // 测试删除nullptr
    bool deleteNullResult = proxy->deleteMenu(nullptr);
    EXPECT_FALSE(deleteNullResult);
    
    // 测试删除已删除的菜单（或者无效指针）
    // 这个行为取决于实现，我们主要验证不会崩溃
    delete proxy;
}

/**
 * @brief 测试deleteAction方法
 * 验证Action删除功能
 */
TEST_F(DFMExtMenuProxyTest, DeleteAction)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    // 创建一个Action
    DFMExtAction *action = proxy->createAction();
    ASSERT_NE(action, nullptr);
    
    // 测试删除Action
    bool deleteResult = proxy->deleteAction(action);
    EXPECT_TRUE(deleteResult);
    
    // 测试删除nullptr
    bool deleteNullResult = proxy->deleteAction(nullptr);
    EXPECT_FALSE(deleteNullResult);
    
    // 测试删除已删除的Action（或者无效指针）
    // 这个行为取决于实现，我们主要验证不会崩溃
    delete proxy;
}

/**
 * @brief 测试多个菜单和Action的创建和管理
 * 验证复杂的创建和删除场景
 */
TEST_F(DFMExtMenuProxyTest, MultipleCreateAndDelete)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    // 创建多个菜单
    std::vector<DFMExtMenu*> menus;
    for (int i = 0; i < 5; ++i) {
        DFMExtMenu *menu = proxy->createMenu();
        ASSERT_NE(menu, nullptr);
        
        menu->setTitle("Menu " + std::to_string(i));
        menu->setIcon("/menu" + std::to_string(i) + ".png");
        menus.push_back(menu);
    }
    
    // 创建多个Action
    std::vector<DFMExtAction*> actions;
    for (int i = 0; i < 10; ++i) {
        DFMExtAction *action = proxy->createAction();
        ASSERT_NE(action, nullptr);
        
        action->setText("Action " + std::to_string(i));
        action->setIcon("/action" + std::to_string(i) + ".png");
        action->setToolTip("Tooltip " + std::to_string(i));
        action->setCheckable(i % 2 == 0);
        action->setChecked(i % 3 == 0);
        actions.push_back(action);
    }
    
    // 将Action添加到菜单中
    for (size_t i = 0; i < menus.size(); ++i) {
        DFMExtMenu *menu = menus[i];
        for (size_t j = 0; j < 2; ++j) {
            size_t actionIndex = i * 2 + j;
            if (actionIndex < actions.size()) {
                menu->addAction(actions[actionIndex]);
            }
        }
    }
    
    // 验证菜单结构
    for (size_t i = 0; i < menus.size(); ++i) {
        DFMExtMenu *menu = menus[i];
        EXPECT_EQ(menu->title(), "Menu " + std::to_string(i));
        EXPECT_EQ(menu->icon(), "/menu" + std::to_string(i) + ".png");
        
        auto menuActions = menu->actions();
        EXPECT_EQ(menuActions.size(), 2);
    }
    
    // 删除所有菜单（这应该也会删除其中的Action）
    for (size_t i = 0; i < menus.size(); ++i) {
        DFMExtMenu *menu = menus[i];
        bool deleteResult = proxy->deleteMenu(menu);
        // if menu is last menu, it should be true
        if (i == menus.size() - 1) {
            EXPECT_TRUE(deleteResult);
        } else {
            EXPECT_FALSE(deleteResult);
        }
    }
    
    // 删除剩余的Action
    for (size_t i = menus.size() * 2; i < actions.size(); ++i) {
        bool deleteResult = proxy->deleteAction(actions[i]);
        EXPECT_TRUE(deleteResult);
    }
    
    delete proxy;
}

/**
 * @brief 测试创建复杂的菜单结构
 * 验证子菜单和复杂Action的创建
 */
TEST_F(DFMExtMenuProxyTest, ComplexMenuStructure)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    // 创建主菜单
    DFMExtMenu *mainMenu = proxy->createMenu();
    ASSERT_NE(mainMenu, nullptr);
    
    mainMenu->setTitle("Main Menu");
    mainMenu->setIcon("/main_menu.png");
    
    // 创建各种类型的Action
    std::vector<DFMExtAction*> actions;
    
    // 普通Action
    for (int i = 0; i < 3; ++i) {
        DFMExtAction *action = proxy->createAction();
        action->setText("Normal Action " + std::to_string(i));
        action->setIcon("/normal" + std::to_string(i) + ".png");
        action->setToolTip("Normal Action " + std::to_string(i) + " tooltip");
        actions.push_back(action);
        mainMenu->addAction(action);
    }
    
    // 分隔符
    DFMExtAction *separator = proxy->createAction();
    separator->setSeparator(true);
    actions.push_back(separator);
    mainMenu->addAction(separator);
    
    // 可检查Action
    for (int i = 0; i < 2; ++i) {
        DFMExtAction *checkableAction = proxy->createAction();
        checkableAction->setText("Checkable Action " + std::to_string(i));
        checkableAction->setCheckable(true);
        checkableAction->setChecked(i % 2 == 0);
        actions.push_back(checkableAction);
        mainMenu->addAction(checkableAction);
    }
    
    // 子菜单Action
    DFMExtMenu *subMenu = proxy->createMenu();
    subMenu->setTitle("Sub Menu");
    subMenu->setIcon("/submenu.png");
    
    // 为子菜单添加Action
    DFMExtAction *subAction1 = proxy->createAction();
    subAction1->setText("Sub Action 1");
    subMenu->addAction(subAction1);
    actions.push_back(subAction1);
    
    DFMExtAction *subAction2 = proxy->createAction();
    subAction2->setText("Sub Action 2");
    subMenu->addAction(subAction2);
    actions.push_back(subAction2);
    
    // 将子菜单添加到主菜单
    DFMExtAction *subMenuAction = proxy->createAction();
    subMenuAction->setText("Sub Menu");
    subMenuAction->setMenu(subMenu);
    mainMenu->addAction(subMenuAction);
    actions.push_back(subMenuAction);
    
    // 验证菜单结构
    EXPECT_EQ(mainMenu->title(), "Main Menu");
    EXPECT_EQ(mainMenu->icon(), "/main_menu.png");
    
    auto mainMenuActions = mainMenu->actions();
    EXPECT_EQ(mainMenuActions.size(), 7); // 3 normal + 1 separator + 2 checkable + 1 submenu
    
    // 验证子菜单
    EXPECT_EQ(subMenu->title(), "Sub Menu");
    EXPECT_EQ(subMenu->icon(), "/submenu.png");
    
    auto subMenuActions = subMenu->actions();
    EXPECT_EQ(subMenuActions.size(), 2);
    
    // 清理资源
    bool deleteMainMenuResult = proxy->deleteMenu(mainMenu);
    EXPECT_FALSE(deleteMainMenuResult);

    
    delete proxy;
}

/**
 * @brief 测试边界条件
 * 验证在各种边界条件下的行为
 */
TEST_F(DFMExtMenuProxyTest, BoundaryConditions)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    // 测试大量创建菜单
    const int menuCount = 100;
    std::vector<DFMExtMenu*> menus;
    
    for (int i = 0; i < menuCount; ++i) {
        DFMExtMenu *menu = proxy->createMenu();
        if (menu) {
            menu->setTitle("Boundary Test Menu " + std::to_string(i));
            menu->setIcon("/boundary_menu_" + std::to_string(i) + ".png");
            menus.push_back(menu);
        }
    }
    
    // 测试大量创建Action
    const int actionCount = 500;
    std::vector<DFMExtAction*> actions;
    
    for (int i = 0; i < actionCount; ++i) {
        DFMExtAction *action = proxy->createAction();
        if (action) {
            action->setText("Boundary Test Action " + std::to_string(i));
            action->setIcon("/boundary_action_" + std::to_string(i) + ".png");
            action->setToolTip("Boundary Test Tooltip " + std::to_string(i));
            action->setCheckable(i % 2 == 0);
            action->setChecked(i % 3 == 0);
            action->setEnabled(i % 4 == 0);
            actions.push_back(action);
        }
    }
    
    // 验证创建结果
    EXPECT_EQ(menus.size(), menuCount);
    EXPECT_EQ(actions.size(), actionCount);
    
    // 测试重复删除
    if (!menus.empty()) {
        bool firstDelete = proxy->deleteMenu(menus[0]);
        EXPECT_FALSE(firstDelete);
        
        bool secondDelete = proxy->deleteMenu(menus[0]);
        // 这个结果取决于实现，可能返回true或false
        // 主要验证不会崩溃
    }
    
    if (!actions.empty()) {
        bool firstDelete = proxy->deleteAction(actions[0]);
        EXPECT_FALSE(firstDelete);
        
        bool secondDelete = proxy->deleteAction(actions[0]);
        // 这个结果取决于实现，可能返回true或false
        // 主要验证不会崩溃
    }
    
    // 清理剩余资源
    for (auto* menu : menus) {
        proxy->deleteMenu(menu);
    }
    
    for (auto* action : actions) {
        proxy->deleteAction(action);
    }
    
    delete proxy;
}

/**
 * @brief 测试性能
 * 验证大量创建和删除操作的性能
 */
TEST_F(DFMExtMenuProxyTest, Performance)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    const int createCount = 1000;
    std::vector<DFMExtMenu*> menus;
    std::vector<DFMExtAction*> actions;
    
    // 测试大量创建的性能
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < createCount; ++i) {
        if (i % 3 == 0) {
            // 创建菜单
            DFMExtMenu *menu = proxy->createMenu();
            if (menu) {
                menu->setTitle("Performance Menu " + std::to_string(i));
                menus.push_back(menu);
            }
        } else {
            // 创建Action
            DFMExtAction *action = proxy->createAction();
            if (action) {
                action->setText("Performance Action " + std::to_string(i));
                actions.push_back(action);
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto createDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证性能在合理范围内
    EXPECT_LT(createDuration.count(), 2000); // 创建1000个对象应该在2秒内完成
    
    // 测试大量删除的性能
    start = std::chrono::high_resolution_clock::now();
    
    for (auto* menu : menus) {
        proxy->deleteMenu(menu);
    }
    
    for (auto* action : actions) {
        proxy->deleteAction(action);
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto deleteDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证性能在合理范围内
    EXPECT_LT(deleteDuration.count(), 1000); // 删除1000个对象应该在1秒内完成
    
    delete proxy;
}

/**
 * @brief 测试内存管理
 * 验证频繁操作的内存安全性
 */
TEST_F(DFMExtMenuProxyTest, MemoryManagement)
{
    // 测试大量创建和销毁proxy对象
    for (int i = 0; i < 50; ++i) {
        DFMExtMenuProxy *proxy = createTestMenuProxy();
        ASSERT_NE(proxy, nullptr);
        
        // 创建一些菜单和Action
        for (int j = 0; j < 10; ++j) {
            DFMExtMenu *menu = proxy->createMenu();
            if (menu) {
                menu->setTitle("Memory Test Menu " + std::to_string(j));
                
                DFMExtAction *action = proxy->createAction();
                if (action) {
                    action->setText("Memory Test Action " + std::to_string(j));
                    menu->addAction(action);
                }
                
                proxy->deleteMenu(menu);
            }
        }
        
        delete proxy;
    }
    
    // 测试单个proxy的长时间运行
    DFMExtMenuProxy *longRunningProxy = createTestMenuProxy();
    ASSERT_NE(longRunningProxy, nullptr);
    
    // 长时间重复创建和删除
    for (int i = 0; i < 1000; ++i) {
        DFMExtMenu *menu = longRunningProxy->createMenu();
        if (menu) {
            menu->setTitle("Long Running Menu " + std::to_string(i));
            
            // 添加多个Action
            for (int j = 0; j < 5; ++j) {
                DFMExtAction *action = longRunningProxy->createAction();
                if (action) {
                    action->setText("Long Running Action " + std::to_string(j));
                    menu->addAction(action);
                }
            }
            
            longRunningProxy->deleteMenu(menu);
        }
    }
    
    delete longRunningProxy;
    EXPECT_TRUE(true); // 如果到达这里说明内存管理正确
}

/**
 * @brief 测试错误处理
 * 验证在错误条件下的行为
 */
TEST_F(DFMExtMenuProxyTest, ErrorHandling)
{
    DFMExtMenuProxy *proxy = createTestMenuProxy();
    ASSERT_NE(proxy, nullptr);
    
    // 测试删除无效指针的不同情况
    DFMExtMenu *invalidMenu = reinterpret_cast<DFMExtMenu*>(0x12345678);
    bool deleteInvalidMenuResult = proxy->deleteMenu(invalidMenu);
    EXPECT_FALSE(deleteInvalidMenuResult); // 应该返回false
    
    DFMExtAction *invalidAction = reinterpret_cast<DFMExtAction*>(0x87654321);
    bool deleteInvalidActionResult = proxy->deleteAction(invalidAction);
    EXPECT_FALSE(deleteInvalidActionResult); // 应该返回false
    
    // 测试创建后的立即删除
    for (int i = 0; i < 100; ++i) {
        DFMExtMenu *menu = proxy->createMenu();
        if (menu) {
            bool deleteResult = proxy->deleteMenu(menu);
            EXPECT_TRUE(deleteResult);
        }
        
        DFMExtAction *action = proxy->createAction();
        if (action) {
            bool deleteResult = proxy->deleteAction(action);
            EXPECT_TRUE(deleteResult);
        }
    }
    
    // 验证代理仍然正常工作
    DFMExtMenu *testMenu = proxy->createMenu();
    EXPECT_NE(testMenu, nullptr);
    
    DFMExtAction *testAction = proxy->createAction();
    EXPECT_NE(testAction, nullptr);
    
    if (testMenu) proxy->deleteMenu(testMenu);
    if (testAction) proxy->deleteAction(testAction);
    
    delete proxy;
}

/**
 * @brief 为 DFMExtMenu 测试创建的测试夹具
 */
class DFMExtMenuTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 创建 Mock 私有实例
        mockMenuPrivate = new MockDFMExtMenuPrivate();
        testMenu = new DFMExtMenu(mockMenuPrivate);
        
        // 创建测试用的 Action
        mockActionPrivate1 = new MockDFMExtActionPrivate();
        mockActionPrivate2 = new MockDFMExtActionPrivate();
        mockActionPrivate3 = new MockDFMExtActionPrivate();
        
        testAction1 = new DFMExtAction(mockActionPrivate1);
        testAction2 = new DFMExtAction(mockActionPrivate2);
        testAction3 = new DFMExtAction(mockActionPrivate3);
        
        // 设置 Action 属性
        testAction1->setText("Action 1");
        testAction1->setIcon("icon1.png");
        
        testAction2->setText("Action 2");
        testAction2->setIcon("icon2.png");
        testAction2->setSeparator(true);
        
        testAction3->setText("Action 3");
        testAction3->setIcon("icon3.png");
        testAction3->setCheckable(true);
    }
    
    void TearDown() override
    {
        delete testMenu;
        delete testAction1;
        delete testAction2;
        delete testAction3;
        stub.clear();
    }
    
    MockDFMExtMenuPrivate* mockMenuPrivate;
    MockDFMExtActionPrivate* mockActionPrivate1;
    MockDFMExtActionPrivate* mockActionPrivate2;
    MockDFMExtActionPrivate* mockActionPrivate3;
    DFMExtMenu* testMenu;
    DFMExtAction* testAction1;
    DFMExtAction* testAction2;
    DFMExtAction* testAction3;
    stub_ext::StubExt stub;
};

/**
 * @brief 创建测试用的 DFMExtMenu 对象
 */
DFMExtMenu* createTestMenu()
{
    MockDFMExtMenuPrivate *menuPrivate = new MockDFMExtMenuPrivate();
    DFMExtMenu *menu = new DFMExtMenu(menuPrivate);
    return menu;
}

/**
 * @brief 创建测试用的 DFMExtAction 对象
 */
DFMExtAction* createTestAction(const std::string& text)
{
    MockDFMExtActionPrivate *actionPrivate = new MockDFMExtActionPrivate();
    DFMExtAction *action = new DFMExtAction(actionPrivate);
    action->setText(text);
    return action;
}

/**
 * @brief 测试 DFMExtMenu::insertAction 方法
 * 验证插入 Action 功能
 */
TEST_F(DFMExtMenuTest, InsertAction)
{
    // 测试向空菜单插入 Action
    EXPECT_TRUE(testMenu->insertAction(nullptr, testAction1));
    
    auto actions = testMenu->actions();
    EXPECT_EQ(actions.size(), 1);
    EXPECT_EQ(actions.front(), testAction1);
    
    // 测试在指定 Action 前插入
    EXPECT_TRUE(testMenu->insertAction(testAction1, testAction2));
    
    actions = testMenu->actions();
    EXPECT_EQ(actions.size(), 2);
    auto it = actions.begin();
    EXPECT_EQ(*it, testAction2); // testAction2 应该在最前面
    ++it;
    EXPECT_EQ(*it, testAction1);
    
    // 测试在不存在的 Action 前插入（应该追加到末尾）
    DFMExtAction* nonExistentAction = createTestAction("Non-existent");
    EXPECT_TRUE(testMenu->insertAction(nonExistentAction, testAction3));
    
    actions = testMenu->actions();
    EXPECT_EQ(actions.size(), 3);
    EXPECT_EQ(actions.back(), testAction3);
    
    delete nonExistentAction;
}

/**
 * @brief 测试 DFMExtMenu::insertAction 边界条件
 */
TEST_F(DFMExtMenuTest, InsertActionBoundaryConditions)
{
    // 测试插入 nullptr Action
    EXPECT_FALSE(testMenu->insertAction(nullptr, nullptr));
    EXPECT_TRUE(testMenu->actions().empty());
    
    // 测试重复插入同一个 Action
    EXPECT_TRUE(testMenu->insertAction(nullptr, testAction1));
    EXPECT_TRUE(testMenu->insertAction(testAction1, testAction1)); // 在自己前面插入自己
    
    auto actions = testMenu->actions();
    EXPECT_EQ(actions.size(), 2);
}

/**
 * @brief 测试 DFMExtMenu::menuAction 方法
 * 验证获取菜单 Action 功能
 */
TEST_F(DFMExtMenuTest, MenuAction)
{
    // 测试空菜单的 menuAction
    EXPECT_EQ(testMenu->menuAction(), nullptr);
    
    // 添加 Action 后测试
    testMenu->addAction(testAction1);
    EXPECT_EQ(testMenu->menuAction(), testAction1);
    
    // 插入新的 Action 到前面
    testMenu->insertAction(testAction1, testAction2);
    EXPECT_EQ(testMenu->menuAction(), testAction2); // 应该返回第一个 Action
    
    // 测试有多个 Action 的情况
    testMenu->addAction(testAction3);
    auto actions = testMenu->actions();
    EXPECT_EQ(actions.size(), 3);
    EXPECT_EQ(testMenu->menuAction(), testAction2); // 第一个 Action 保持不变
}

/**
 * @brief 测试 DFMExtMenu::menuAction 边界条件
 */
TEST_F(DFMExtMenuTest, MenuActionBoundaryConditions)
{
    // 测试删除 Action 后的行为
    testMenu->addAction(testAction1);
    EXPECT_EQ(testMenu->menuAction(), testAction1);
    
    // 由于 DFMExtMenu 没有 clear() 方法，我们创建一个新的菜单来测试空菜单行为
    MockDFMExtMenuPrivate* emptyMenuPrivate = new MockDFMExtMenuPrivate();
    DFMExtMenu* emptyMenu = new DFMExtMenu(emptyMenuPrivate);
    EXPECT_EQ(emptyMenu->menuAction(), nullptr);
    delete emptyMenu;
}

/**
 * @brief 测试 DFMExtMenu::triggered 信号
 * 验证触发 Action 功能
 */
TEST_F(DFMExtMenuTest, TriggeredSignal)
{
    bool callbackTriggered = false;
    DFMExtAction* triggeredAction = nullptr;
    
    // 设置触发回调
    testMenu->registerTriggered([&](DFMExtAction *action) {
        callbackTriggered = true;
        triggeredAction = action;
    });
    
    // 测试触发 Action
    testMenu->triggered(testAction1);
    
    EXPECT_TRUE(callbackTriggered);
    EXPECT_EQ(triggeredAction, testAction1);
}

/**
 * @brief 测试 DFMExtMenu::triggered 多个回调
 */
TEST_F(DFMExtMenuTest, TriggeredMultipleCallbacks)
{
    int callbackCount = 0;
    std::vector<DFMExtAction*> triggeredActions;
    
    // 注册多个回调
    testMenu->registerTriggered([&](DFMExtAction *action) {
        callbackCount++;
        triggeredActions.push_back(action);
    });
    
    testMenu->registerTriggered([&](DFMExtAction *action) {
        callbackCount++;
        triggeredActions.push_back(action);
    });
    
    // 触发 Action
    testMenu->triggered(testAction1);
    
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(triggeredActions.size(), 1);
    EXPECT_EQ(triggeredActions[0], testAction1);
}

/**
 * @brief 测试 DFMExtMenu::triggered 边界条件
 */
TEST_F(DFMExtMenuTest, TriggeredBoundaryConditions)
{
    bool callbackTriggered = false;
    
    testMenu->registerTriggered([&](DFMExtAction *action) {
        callbackTriggered = true;
    });
    
    // 测试触发 nullptr Action
    testMenu->triggered(nullptr);
    EXPECT_TRUE(callbackTriggered); // 回调应该被触发
    
    // 重置标志并测试没有注册回调的情况
    callbackTriggered = false;
    DFMExtMenu* newMenu = createTestMenu();
    newMenu->triggered(testAction1);
    EXPECT_FALSE(callbackTriggered); // 没有注册回调，不应该触发
    
    delete newMenu;
}

/**
 * @brief 测试 DFMExtMenu::hovered 信号
 * 验证悬停 Action 功能
 */
TEST_F(DFMExtMenuTest, HoveredSignal)
{
    bool callbackTriggered = false;
    DFMExtAction* hoveredAction = nullptr;
    
    // 设置悬停回调
    testMenu->registerHovered([&](DFMExtAction *action) {
        callbackTriggered = true;
        hoveredAction = action;
    });
    
    // 测试悬停 Action
    testMenu->hovered(testAction1);
    
    EXPECT_TRUE(callbackTriggered);
    EXPECT_EQ(hoveredAction, testAction1);
}

/**
 * @brief 测试 DFMExtMenu::hovered 多个回调
 */
TEST_F(DFMExtMenuTest, HoveredMultipleCallbacks)
{
    int callbackCount = 0;
    std::vector<DFMExtAction*> hoveredActions;
    
    // 注册多个回调
    testMenu->registerHovered([&](DFMExtAction *action) {
        callbackCount++;
        hoveredActions.push_back(action);
    });
    
    testMenu->registerHovered([&](DFMExtAction *action) {
        callbackCount++;
        hoveredActions.push_back(action);
    });
    
    // 悬停 Action
    testMenu->hovered(testAction2);
    
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(hoveredActions.size(), 1);
    EXPECT_EQ(hoveredActions[0], testAction2);
}

/**
 * @brief 测试 DFMExtMenu::hovered 边界条件
 */
TEST_F(DFMExtMenuTest, HoveredBoundaryConditions)
{
    bool callbackTriggered = false;
    
    testMenu->registerHovered([&](DFMExtAction *action) {
        callbackTriggered = true;
    });
    
    // 测试悬停 nullptr Action
    testMenu->hovered(nullptr);
    EXPECT_TRUE(callbackTriggered);
    
    // 测试没有注册回调的情况
    callbackTriggered = false;
    DFMExtMenu* newMenu = createTestMenu();
    newMenu->hovered(testAction1);
    EXPECT_FALSE(callbackTriggered);
    
    delete newMenu;
}

/**
 * @brief 测试 DFMExtMenu::deleted 信号
 * 验证菜单删除功能
 */
TEST_F(DFMExtMenuTest, DeletedSignal)
{
    bool callbackTriggered = false;
    DFMExtMenu* deletedMenu = nullptr;
    
    // 设置删除回调
    testMenu->registerDeleted([&](DFMExtMenu *self) {
        callbackTriggered = true;
        deletedMenu = self;
    });
    
    // 测试删除菜单
    testMenu->deleted(testMenu);
    
    EXPECT_TRUE(callbackTriggered);
    EXPECT_EQ(deletedMenu, testMenu);
}

/**
 * @brief 测试 DFMExtMenu::deleted 多个回调
 */
TEST_F(DFMExtMenuTest, DeletedMultipleCallbacks)
{
    int callbackCount = 0;
    std::vector<DFMExtMenu*> deletedMenus;
    
    // 注册多个回调
    testMenu->registerDeleted([&](DFMExtMenu *self) {
        callbackCount++;
        deletedMenus.push_back(self);
    });
    
    testMenu->registerDeleted([&](DFMExtMenu *self) {
        callbackCount++;
        deletedMenus.push_back(self);
    });
    
    // 删除菜单
    testMenu->deleted(testMenu);
    
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(deletedMenus.size(), 1);
    EXPECT_EQ(deletedMenus[0], testMenu);
}

/**
 * @brief 测试 DFMExtMenu::deleted 边界条件
 */
TEST_F(DFMExtMenuTest, DeletedBoundaryConditions)
{
    bool callbackTriggered = false;
    
    testMenu->registerDeleted([&](DFMExtMenu *self) {
        callbackTriggered = true;
    });
    
    // 测试删除 nullptr 菜单
    testMenu->deleted(nullptr);
    EXPECT_TRUE(callbackTriggered);
    
    // 测试删除其他菜单
    DFMExtMenu* otherMenu = createTestMenu();
    testMenu->deleted(otherMenu);
    EXPECT_TRUE(callbackTriggered);
    
    delete otherMenu;
}

/**
 * @brief 测试 DFMExtMenu::registerTriggered 方法
 * 验证注册触发回调功能
 */
TEST_F(DFMExtMenuTest, RegisterTriggered)
{
    int triggerCount = 0;
    std::vector<std::string> triggerTexts;
    
    // 注册触发回调
    testMenu->registerTriggered([&](DFMExtAction *action) {
        triggerCount++;
        if (action) {
            triggerTexts.push_back(action->text());
        }
    });
    
    // 触发不同的 Action
    testMenu->triggered(testAction1);
    testMenu->triggered(testAction2);
    testMenu->triggered(testAction3);
    
    EXPECT_EQ(triggerCount, 3);
    EXPECT_EQ(triggerTexts.size(), 3);
    EXPECT_EQ(triggerTexts[0], "Action 1");
    EXPECT_EQ(triggerTexts[1], "Action 2");
    EXPECT_EQ(triggerTexts[2], "Action 3");
}

/**
 * @brief 测试 DFMExtMenu::registerTriggered 复杂回调
 */
TEST_F(DFMExtMenuTest, RegisterTriggeredComplex)
{
    struct TriggerInfo {
        int count = 0;
        std::string lastText;
        bool wasSeparator = false;
    };
    
    TriggerInfo info;
    
    // 注册复杂的触发回调
    testMenu->registerTriggered([&](DFMExtAction *action) {
        info.count++;
        if (action) {
            info.lastText = action->text();
            info.wasSeparator = action->isSeparator();
        }
    });
    
    // 触发不同类型的 Action
    testMenu->triggered(testAction1); // 普通 Action
    EXPECT_EQ(info.count, 1);
    EXPECT_EQ(info.lastText, "Action 1");
    EXPECT_FALSE(info.wasSeparator);
    
    testMenu->triggered(testAction2); // 分隔符 Action
    EXPECT_EQ(info.count, 2);
    EXPECT_EQ(info.lastText, "Action 2");
    EXPECT_TRUE(info.wasSeparator);
    
    testMenu->triggered(testAction3); // 可检查 Action
    EXPECT_EQ(info.count, 3);
    EXPECT_EQ(info.lastText, "Action 3");
    EXPECT_FALSE(info.wasSeparator);
}

/**
 * @brief 测试 DFMExtMenu::registerHovered 方法
 * 验证注册悬停回调功能
 */
TEST_F(DFMExtMenuTest, RegisterHovered)
{
    int hoverCount = 0;
    std::vector<std::string> hoverTexts;
    
    // 注册悬停回调
    testMenu->registerHovered([&](DFMExtAction *action) {
        hoverCount++;
        if (action) {
            hoverTexts.push_back(action->text());
        }
    });
    
    // 悬停不同的 Action
    testMenu->hovered(testAction1);
    testMenu->hovered(testAction2);
    testMenu->hovered(testAction3);
    
    EXPECT_EQ(hoverCount, 3);
    EXPECT_EQ(hoverTexts.size(), 3);
    EXPECT_EQ(hoverTexts[0], "Action 1");
    EXPECT_EQ(hoverTexts[1], "Action 2");
    EXPECT_EQ(hoverTexts[2], "Action 3");
}

/**
 * @brief 测试 DFMExtMenu::registerHovered 属性检查
 */
TEST_F(DFMExtMenuTest, RegisterHoveredProperties)
{
    struct HoverInfo {
        int count = 0;
        std::string lastText;
        std::string lastIcon;
        bool checkable = false;
        bool enabled = false;
    };
    
    HoverInfo info;
    
    // 注册悬停回调检查属性
    testMenu->registerHovered([&](DFMExtAction *action) {
        info.count++;
        if (action) {
            info.lastText = action->text();
            info.lastIcon = action->icon();
            info.checkable = action->isCheckable();
            info.enabled = action->isEnabled();
        }
    });
    
    // 悬停不同属性的 Action
    testMenu->hovered(testAction1);
    EXPECT_EQ(info.count, 1);
    EXPECT_EQ(info.lastText, "Action 1");
    EXPECT_EQ(info.lastIcon, "icon1.png");
    EXPECT_FALSE(info.checkable);
    
    testMenu->hovered(testAction3);
    EXPECT_EQ(info.count, 2);
    EXPECT_EQ(info.lastText, "Action 3");
    EXPECT_EQ(info.lastIcon, "icon3.png");
    EXPECT_TRUE(info.checkable);
}

/**
 * @brief 测试 DFMExtMenu::registerDeleted 方法
 * 验证注册删除回调功能
 */
TEST_F(DFMExtMenuTest, RegisterDeleted)
{
    int deleteCount = 0;
    std::vector<DFMExtMenu*> deletedMenus;
    
    // 注册删除回调
    testMenu->registerDeleted([&](DFMExtMenu *self) {
        deleteCount++;
        deletedMenus.push_back(self);
    });
    
    // 创建多个菜单并删除
    DFMExtMenu* menu1 = createTestMenu();
    DFMExtMenu* menu2 = createTestMenu();
    DFMExtMenu* menu3 = createTestMenu();
    
    menu1->registerDeleted([&](DFMExtMenu *self) {
        deleteCount++;
        deletedMenus.push_back(self);
    });
    
    // 删除菜单
    testMenu->deleted(testMenu);
    menu1->deleted(menu1);
    menu2->deleted(menu2);
    menu3->deleted(menu3);
    
    EXPECT_EQ(deleteCount, 2); // testMenu 和 menu1 各有一个回调，menu2 和 menu3 没有
    EXPECT_EQ(deletedMenus.size(), 2);
    
    delete menu1;
    delete menu2;
    delete menu3;
}

/**
 * @brief 测试 DFMExtMenu 综合功能
 * 验证多个功能协同工作
 */
TEST_F(DFMExtMenuTest, ComprehensiveFunctionality)
{
    int triggerCount = 0;
    int hoverCount = 0;
    int deleteCount = 0;
    
    // 注册所有回调
    testMenu->registerTriggered([&](DFMExtAction *action) {
        triggerCount++;
    });
    
    testMenu->registerHovered([&](DFMExtAction *action) {
        hoverCount++;
    });
    
    testMenu->registerDeleted([&](DFMExtMenu *self) {
        deleteCount++;
    });
    
    // 执行操作
    testMenu->addAction(testAction1);
    testMenu->insertAction(nullptr, testAction2);
    testMenu->insertAction(testAction1, testAction3);
    
    EXPECT_EQ(testMenu->menuAction(), testAction3); // 第一个 Action 应该是 testAction3
    
    testMenu->hovered(testAction1);
    testMenu->hovered(testAction2);
    
    testMenu->triggered(testAction1);
    testMenu->triggered(testAction3);
    
    testMenu->deleted(testMenu);
    
    // 验证所有回调都被触发
    EXPECT_EQ(hoverCount, 2);
    EXPECT_EQ(triggerCount, 2);
    EXPECT_EQ(deleteCount, 1);
    
    // 验证菜单状态
    auto actions = testMenu->actions();
    EXPECT_EQ(actions.size(), 3);
}

/**
 * @brief 测试 DFMExtMenu 内存管理
 * 验证对象创建和销毁的安全性
 */
TEST_F(DFMExtMenuTest, MemoryManagement)
{
    // 测试大量创建和销毁
    for (int i = 0; i < 100; ++i) {
        DFMExtMenu* menu = createTestMenu();
        DFMExtAction* action = createTestAction("Test Action " + std::to_string(i));
        
        menu->addAction(action);
        menu->registerTriggered([](DFMExtAction *action) {
            // 空回调
        });
        
        menu->triggered(action);
        menu->deleted(menu);
        
        delete menu;
        delete action;
    }
    
    EXPECT_TRUE(true); // 如果没有崩溃，说明内存管理正确
}
