// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <QObject>
#include <QMenu>
#include <QAction>
#include <QVariantHash>
#include <QSignalSpy>

#include <dfm-base/interfaces/abstractmenuscene.h>

// Include stub headers
#include "stubext.h"

using namespace dfmbase;

// Create a concrete implementation of AbstractMenuScene for testing
class TestMenuSceneImpl : public dfmbase::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit TestMenuSceneImpl(QObject *parent = nullptr) : AbstractMenuScene(parent) {}
    
    QString name() const override {
        return "TestMenuScene";
    }
    
    // Override other virtual methods for testing
    bool initialize(const QVariantHash &params) override {
        initialized = true;
        parameters = params;
        return AbstractMenuScene::initialize(params);
    }
    
    bool create(QMenu *parent) override {
        created = true;
        menu = parent;
        return AbstractMenuScene::create(parent);
    }
    
    void updateState(QMenu *parent) override {
        updated = true;
        menu = parent;
        AbstractMenuScene::updateState(parent);
    }
    
    bool triggered(QAction *action) override {
        triggeredAction = action;
        return AbstractMenuScene::triggered(action);
    }
    
    bool actionFilter(AbstractMenuScene *caller, QAction *action) override {
        filterCalled = true;
        return AbstractMenuScene::actionFilter(caller, action);
    }
    
    AbstractMenuScene *scene(QAction *action) const override {
        return AbstractMenuScene::scene(action);
    }
    
    // Test helpers
    bool isInitialized() const { return initialized; }
    bool isCreated() const { return created; }
    bool isUpdated() const { return updated; }
    QAction* getTriggeredAction() const { return triggeredAction; }
    bool isFilterCalled() const { return filterCalled; }
    QVariantHash getParameters() const { return parameters; }
    QMenu* getMenu() const { return menu; }
    
private:
    bool initialized = false;
    bool created = false;
    bool updated = false;
    bool filterCalled = false;
    QAction *triggeredAction = nullptr;
    QMenu *menu = nullptr;
    QVariantHash parameters;
};

class TestAbstractMenuScene : public testing::Test
{
protected:
    void SetUp() override {
        stub.clear();
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        scene = new TestMenuSceneImpl();
    }
    
    void TearDown() override {
        stub.clear();
        if (scene) {
            delete scene;
            scene = nullptr;
        }
    }
    
public:
    stub_ext::StubExt stub;
    QApplication *app = nullptr;
    TestMenuSceneImpl *scene = nullptr;
};

// Test constructor
TEST_F(TestAbstractMenuScene, TestConstructor)
{
    QObject parent;
    
    TestMenuSceneImpl *testScene = new TestMenuSceneImpl(&parent);
    ASSERT_NE(testScene, nullptr);
    EXPECT_EQ(testScene->parent(), &parent);
    
    delete testScene;
}

// Test constructor without parent
TEST_F(TestAbstractMenuScene, TestConstructorWithoutParent)
{
    TestMenuSceneImpl *testScene = new TestMenuSceneImpl();
    ASSERT_NE(testScene, nullptr);
    EXPECT_EQ(testScene->parent(), nullptr);
    
    delete testScene;
}

// Test destructor
TEST_F(TestAbstractMenuScene, TestDestructor)
{
    TestMenuSceneImpl *testScene = new TestMenuSceneImpl();
    delete testScene;
    
    SUCCEED();
}

// Test name method (pure virtual)
TEST_F(TestAbstractMenuScene, TestName)
{
    QString name = scene->name();
    EXPECT_EQ(name, "TestMenuScene");
}

// Test initialize method
TEST_F(TestAbstractMenuScene, TestInitialize)
{
    QVariantHash params;
    params["key1"] = "value1";
    params["key2"] = 123;
    
    bool result = scene->initialize(params);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(scene->isInitialized());
    EXPECT_EQ(scene->getParameters(), params);
}

// Test initialize with empty params
TEST_F(TestAbstractMenuScene, TestInitializeEmptyParams)
{
    QVariantHash emptyParams;
    
    bool result = scene->initialize(emptyParams);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(scene->isInitialized());
    EXPECT_TRUE(scene->getParameters().isEmpty());
}

// Test create method
TEST_F(TestAbstractMenuScene, TestCreate)
{
    QMenu menu;
    
    bool result = scene->create(&menu);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(scene->isCreated());
    EXPECT_EQ(scene->getMenu(), &menu);
}

// Test create with null menu
TEST_F(TestAbstractMenuScene, TestCreateWithNullMenu)
{
    bool result = scene->create(nullptr);
    
    EXPECT_TRUE(result);  // Base implementation should handle null
    EXPECT_TRUE(scene->isCreated());
}

// Test updateState method
TEST_F(TestAbstractMenuScene, TestUpdateState)
{
    QMenu menu;
    
    scene->updateState(&menu);
    
    EXPECT_TRUE(scene->isUpdated());
    EXPECT_EQ(scene->getMenu(), &menu);
}

// Test updateState with null menu
TEST_F(TestAbstractMenuScene, TestUpdateStateWithNullMenu)
{
    scene->updateState(nullptr);
    
    EXPECT_TRUE(scene->isUpdated());
}

// Test triggered method
TEST_F(TestAbstractMenuScene, TestTriggered)
{
    QAction action("Test Action");
    
    bool result = scene->triggered(&action);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(scene->getTriggeredAction(), &action);
}

// Test triggered with null action
TEST_F(TestAbstractMenuScene, TestTriggeredWithNullAction)
{
    bool result = scene->triggered(nullptr);
    
    EXPECT_TRUE(result);  // Base implementation should handle null
    EXPECT_EQ(scene->getTriggeredAction(), nullptr);
}

// Test actionFilter method
TEST_F(TestAbstractMenuScene, TestActionFilter)
{
    TestMenuSceneImpl caller;
    QAction action("Test Action");
    
    bool result = scene->actionFilter(&caller, &action);
    
    EXPECT_TRUE(scene->isFilterCalled());
    // The result depends on the base implementation
}

// Test actionFilter with null parameters
TEST_F(TestAbstractMenuScene, TestActionFilterWithNullParameters)
{
    bool result = scene->actionFilter(nullptr, nullptr);
    
    EXPECT_TRUE(scene->isFilterCalled());
}

// Test scene method
TEST_F(TestAbstractMenuScene, TestScene)
{
    QAction action("Test Action");
    
    AbstractMenuScene *result = scene->scene(&action);
    
    // The result depends on the base implementation
    // This test just verifies the method can be called
    SUCCEED();
}

// Test scene with null action
TEST_F(TestAbstractMenuScene, TestSceneWithNullAction)
{
    AbstractMenuScene *result = scene->scene(nullptr);
    
    // The result depends on the base implementation
    SUCCEED();
}

// Test addSubscene method
TEST_F(TestAbstractMenuScene, TestAddSubscene)
{
    TestMenuSceneImpl *subscene = new TestMenuSceneImpl();
    
    bool result = scene->addSubscene(subscene);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(scene->subscene().contains(subscene));
    
    // Clean up
    scene->removeSubscene(subscene);
    delete subscene;
}

// Test addSubscene with null
TEST_F(TestAbstractMenuScene, TestAddSubsceneWithNull)
{
    bool result = scene->addSubscene(nullptr);
    
    // Base implementation should handle null gracefully
    SUCCEED();
}

// Test removeSubscene method
TEST_F(TestAbstractMenuScene, TestRemoveSubscene)
{
    TestMenuSceneImpl *subscene = new TestMenuSceneImpl();
    
    // Add first
    scene->addSubscene(subscene);
    EXPECT_TRUE(scene->subscene().contains(subscene));
    
    // Then remove
    scene->removeSubscene(subscene);
    EXPECT_FALSE(scene->subscene().contains(subscene));
    
    delete subscene;
}

// Test removeSubscene with non-existent subscene
TEST_F(TestAbstractMenuScene, TestRemoveSubsceneNonExistent)
{
    TestMenuSceneImpl *subscene = new TestMenuSceneImpl();
    
    // Try to remove without adding
    scene->removeSubscene(subscene);
    
    delete subscene;
    SUCCEED();
}

// Test removeSubscene with null
TEST_F(TestAbstractMenuScene, TestRemoveSubsceneWithNull)
{
    scene->removeSubscene(nullptr);
    
    // Should not crash
    SUCCEED();
}

// Test subscene method
TEST_F(TestAbstractMenuScene, TestSubscene)
{
    TestMenuSceneImpl *subscene1 = new TestMenuSceneImpl();
    TestMenuSceneImpl *subscene2 = new TestMenuSceneImpl();
    
    // Add subscenes
    scene->addSubscene(subscene1);
    scene->addSubscene(subscene2);
    
    // Get list of subscenes
    QList<AbstractMenuScene *> subscenes = scene->subscene();
    
    EXPECT_EQ(subscenes.size(), 2);
    EXPECT_TRUE(subscenes.contains(subscene1));
    EXPECT_TRUE(subscenes.contains(subscene2));
    
    // Clean up
    scene->removeSubscene(subscene1);
    scene->removeSubscene(subscene2);
    delete subscene1;
    delete subscene2;
}

// Test multiple subscenes management
TEST_F(TestAbstractMenuScene, TestMultipleSubscenesManagement)
{
    const int numSubscenes = 5;
    QList<TestMenuSceneImpl *> subscenes;
    
    // Create and add multiple subscenes
    for (int i = 0; i < numSubscenes; ++i) {
        TestMenuSceneImpl *subscene = new TestMenuSceneImpl();
        subscene->setObjectName(QString("Subscene_%1").arg(i));
        subscenes.append(subscene);
        scene->addSubscene(subscene);
    }
    
    EXPECT_EQ(scene->subscene().size(), numSubscenes);
    
    // Remove all subscenes
    for (TestMenuSceneImpl *subscene : subscenes) {
        scene->removeSubscene(subscene);
        delete subscene;
    }
    
    EXPECT_TRUE(scene->subscene().isEmpty());
}

// Test setSubscene method (protected)
TEST_F(TestAbstractMenuScene, TestSetSubscene)
{
    QList<AbstractMenuScene *> newSubscenes;
    TestMenuSceneImpl *subscene1 = new TestMenuSceneImpl();
    TestMenuSceneImpl *subscene2 = new TestMenuSceneImpl();
    
    newSubscenes << subscene1 << subscene2;
    
    // Since setSubscene is protected, we can't call it directly
    // But we can test the functionality through addSubscene
    scene->addSubscene(subscene1);
    scene->addSubscene(subscene2);
    
    EXPECT_EQ(scene->subscene().size(), 2);
    
    // Clean up
    scene->removeSubscene(subscene1);
    scene->removeSubscene(subscene2);
    delete subscene1;
    delete subscene2;
}

// Test QObject inheritance
TEST_F(TestAbstractMenuScene, TestQObjectInheritance)
{
    // Should inherit from QObject
    EXPECT_TRUE(dynamic_cast<QObject*>(scene) != nullptr);
    
    // Test QObject features
    scene->setObjectName("TestMenuScene");
    EXPECT_EQ(scene->objectName(), "TestMenuScene");
}

// Test signal-slot mechanism
TEST_F(TestAbstractMenuScene, TestSignalSlotMechanism)
{
    // Test that signals can be connected
    QSignalSpy spy(scene, &QObject::objectNameChanged);
    
    scene->setObjectName("NewName");
    
    // Should emit objectNameChanged signal
    EXPECT_EQ(spy.count(), 1);
}

// Test with complex QVariantHash
TEST_F(TestAbstractMenuScene, TestWithComplexQVariantHash)
{
    QVariantHash complexParams;
    complexParams["string"] = "test string";
    complexParams["integer"] = 42;
    complexParams["double"] = 3.14;
    complexParams["bool"] = true;
    complexParams["url"] = QUrl("file:///test/path");
    
    bool result = scene->initialize(complexParams);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(scene->isInitialized());
    
    QVariantHash retrievedParams = scene->getParameters();
    EXPECT_EQ(retrievedParams["string"].toString(), "test string");
    EXPECT_EQ(retrievedParams["integer"].toInt(), 42);
    EXPECT_DOUBLE_EQ(retrievedParams["double"].toDouble(), 3.14);
    EXPECT_EQ(retrievedParams["bool"].toBool(), true);
    EXPECT_EQ(retrievedParams["url"].toUrl(), QUrl("file:///test/path"));
}

// Test menu and action interactions
TEST_F(TestAbstractMenuScene, TestMenuActionInteractions)
{
    QMenu menu;
    menu.setTitle("Test Menu");
    
    // Add some actions to the menu
    QAction *action1 = menu.addAction("Action 1");
    QAction *action2 = menu.addAction("Action 2");
    QAction *action3 = menu.addAction("Action 3");
    
    // Test creating with menu
    scene->create(&menu);
    EXPECT_TRUE(scene->isCreated());
    
    // Test updating state
    scene->updateState(&menu);
    EXPECT_TRUE(scene->isUpdated());
    
    // Test triggering actions
    bool result1 = scene->triggered(action1);
    bool result2 = scene->triggered(action2);
    
    EXPECT_EQ(scene->getTriggeredAction(), action2);  // Last triggered
    
    // Test scene method with actions
    AbstractMenuScene *foundScene1 = scene->scene(action1);
    AbstractMenuScene *foundScene3 = scene->scene(action3);
    
    // Results depend on base implementation
    SUCCEED();
}

// Test multiple instances
TEST_F(TestAbstractMenuScene, TestMultipleInstances)
{
    TestMenuSceneImpl *scene1 = new TestMenuSceneImpl();
    TestMenuSceneImpl *scene2 = new TestMenuSceneImpl();
    
    scene1->setObjectName("Scene1");
    scene2->setObjectName("Scene2");
    
    EXPECT_NE(scene1, scene2);
    EXPECT_NE(scene1->name(), scene2->name());
    
    // Test that they can have different subscenes
    TestMenuSceneImpl *subscene1 = new TestMenuSceneImpl();
    TestMenuSceneImpl *subscene2 = new TestMenuSceneImpl();
    
    scene1->addSubscene(subscene1);
    scene2->addSubscene(subscene2);
    
    EXPECT_TRUE(scene1->subscene().contains(subscene1));
    EXPECT_TRUE(scene2->subscene().contains(subscene2));
    EXPECT_FALSE(scene1->subscene().contains(subscene2));
    
    // Clean up
    scene1->removeSubscene(subscene1);
    scene2->removeSubscene(subscene2);
    delete subscene1;
    delete subscene2;
    delete scene1;
    delete scene2;
}

// Test memory management
TEST_F(TestAbstractMenuScene, TestMemoryManagement)
{
    // Create and destroy multiple instances
    for (int i = 0; i < 10; ++i) {
        TestMenuSceneImpl *tempScene = new TestMenuSceneImpl();
        tempScene->setObjectName(QString("TempScene_%1").arg(i));
        
        // Test basic operations
        tempScene->initialize(QVariantHash());
        tempScene->name();
        
        delete tempScene;
    }
    
    SUCCEED();
}

// Test edge cases
TEST_F(TestAbstractMenuScene, TestEdgeCases)
{
    // Test with empty menu
    QMenu emptyMenu;
    scene->create(&emptyMenu);
    scene->updateState(&emptyMenu);
    
    // Test with action without parent
    QAction orphanAction("Orphan Action");
    scene->triggered(&orphanAction);
    scene->scene(&orphanAction);
    
    // Test with very long name
    TestMenuSceneImpl longNameScene;
    longNameScene.setObjectName(QString("A").repeated(1000));
    
    SUCCEED();
}

#include "test_abstractmenuscene.moc"