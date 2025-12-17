// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_abstractscenecreator.cpp - AbstractSceneCreator class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <memory>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/interfaces/abstractscenecreator.h>
#include <dfm-base/interfaces/abstractmenuscene.h>

DFMBASE_USE_NAMESPACE

/**
 * @brief Concrete implementation of AbstractMenuScene for testing
 */
class TestMenuScene : public AbstractMenuScene
{
    Q_OBJECT
public:
    explicit TestMenuScene(QObject *parent = nullptr)
        : AbstractMenuScene(parent)
    {
        sceneName = "TestScene";
    }
    
    QString name() const override
    {
        return sceneName;
    }
    
    void setSceneName(const QString &name)
    {
        sceneName = name;
    }
    
private:
    QString sceneName;
};

/**
 * @brief Concrete implementation of AbstractSceneCreator for testing
 */
class TestSceneCreator : public AbstractSceneCreator
{
    Q_OBJECT
public:
    explicit TestSceneCreator(QObject *parent = nullptr)
        : AbstractSceneCreator()
    {
        setParent(parent);
        createCount = 0;
    }
    
    AbstractMenuScene *create() override
    {
        createCount++;
        auto *scene = new TestMenuScene();
        scene->setSceneName(QString("Scene_%1").arg(createCount));
        return scene;
    }
    
    // Test helper methods
    int getCreateCount() const { return createCount; }
    
private:
    int createCount;
};

/**
 * @brief Alternative scene creator for testing inheritance
 */
class AlternativeSceneCreator : public AbstractSceneCreator
{
    Q_OBJECT
public:
    explicit AlternativeSceneCreator(QObject *parent = nullptr)
        : AbstractSceneCreator()
    {
        setParent(parent);
    }
    
    AbstractMenuScene *create() override
    {
        auto *scene = new TestMenuScene();
        scene->setSceneName("AlternativeScene");
        return scene;
    }
};

/**
 * @brief AbstractSceneCreator class unit tests
 *
 * Test scope:
 * 1. Construction and initialization
 * 2. Pure virtual function implementation (create)
 * 3. Child scene management (addChild, removeChild, getChildren)
 * 4. Scene creation and factory behavior
 * 5. Inheritance and polymorphism
 * 6. Memory management
 * 7. Edge cases and error handling
 */
class AbstractSceneCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        
        // Set up test application if not exists
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { const_cast<char*>("test_abstractscenecreator") };
            app = std::make_unique<QApplication>(argc, argv);
        }
        
        // Create test scene creator instance
        creator = std::make_unique<TestSceneCreator>();
    }
    
    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        creator.reset();
        app.reset();
    }
    
    // Test stubbing utility
    stub_ext::StubExt stub;
    std::unique_ptr<QApplication> app;
    std::unique_ptr<TestSceneCreator> creator;
};

TEST_F(AbstractSceneCreatorTest, Constructor)
{
    // Test basic construction
    EXPECT_NE(creator.get(), nullptr);
    EXPECT_EQ(creator->parent(), nullptr);
    
    // Test initial children list
    EXPECT_TRUE(creator->getChildren().isEmpty());
    
    // Test construction with parent
    QObject parent;
    auto childCreator = std::make_unique<TestSceneCreator>(&parent);
    EXPECT_EQ(childCreator->parent(), &parent);
    EXPECT_TRUE(childCreator->getChildren().isEmpty());
}

TEST_F(AbstractSceneCreatorTest, CreateScene)
{
    // Test creating scenes
    auto *scene1 = creator->create();
    EXPECT_NE(scene1, nullptr);
    EXPECT_EQ(creator->getCreateCount(), 1);
    
    auto *testScene1 = qobject_cast<TestMenuScene*>(scene1);
    EXPECT_NE(testScene1, nullptr);
    EXPECT_EQ(testScene1->name(), "Scene_1");
    
    // Create another scene
    auto *scene2 = creator->create();
    EXPECT_NE(scene2, nullptr);
    EXPECT_EQ(creator->getCreateCount(), 2);
    
    auto *testScene2 = qobject_cast<TestMenuScene*>(scene2);
    EXPECT_EQ(testScene2->name(), "Scene_2");
    
    delete scene1;
    delete scene2;
}

TEST_F(AbstractSceneCreatorTest, AddChild)
{
    // Test adding children
    EXPECT_TRUE(creator->addChild("child1"));
    EXPECT_TRUE(creator->addChild("child2"));
    EXPECT_TRUE(creator->addChild("child3"));
    
    auto children = creator->getChildren();
    EXPECT_EQ(children.size(), 3);
    EXPECT_TRUE(children.contains("child1"));
    EXPECT_TRUE(children.contains("child2"));
    EXPECT_TRUE(children.contains("child3"));
    
    // Test adding duplicate child
    EXPECT_TRUE(creator->addChild("child1"));  // Should allow duplicates
    children = creator->getChildren();
    EXPECT_EQ(children.count("child1"), 2);
}

TEST_F(AbstractSceneCreatorTest, RemoveChild)
{
    // First add some children
    creator->addChild("child1");
    creator->addChild("child2");
    creator->addChild("child3");
    creator->addChild("child1");  // Add duplicate
    
    // Test removing existing child
    creator->removeChild("child2");
    auto children = creator->getChildren();
    EXPECT_EQ(children.size(), 3);
    EXPECT_FALSE(children.contains("child2"));
    
    // Test removing child with duplicates (should remove one instance)
    creator->removeChild("child1");
    children = creator->getChildren();
    EXPECT_EQ(children.size(), 2);
    EXPECT_TRUE(children.contains("child1"));  // Still has one child1
    
    // Test removing non-existing child (should not crash)
    creator->removeChild("nonexistent");
    children = creator->getChildren();
    EXPECT_EQ(children.size(), 2);  // Should remain unchanged
}

TEST_F(AbstractSceneCreatorTest, GetChildren)
{
    // Test getting children from empty creator
    EXPECT_TRUE(creator->getChildren().isEmpty());
    
    // Add children and verify
    creator->addChild("sceneA");
    creator->addChild("sceneB");
    creator->addChild("sceneC");
    
    auto children = creator->getChildren();
    EXPECT_EQ(children.size(), 3);
    
    // Verify order is preserved
    EXPECT_EQ(children[0], "sceneA");
    EXPECT_EQ(children[1], "sceneB");
    EXPECT_EQ(children[2], "sceneC");
    
    // Test that returned list is a copy (modifying it shouldn't affect original)
    auto childrenCopy = creator->getChildren();
    childrenCopy.clear();
    childrenCopy.append("newchild");
    
    auto originalChildren = creator->getChildren();
    EXPECT_EQ(originalChildren.size(), 3);
    EXPECT_FALSE(originalChildren.contains("newchild"));
}

TEST_F(AbstractSceneCreatorTest, MultipleCreators)
{
    // Test multiple creators work independently
    auto creator1 = std::make_unique<TestSceneCreator>();
    auto creator2 = std::make_unique<AlternativeSceneCreator>();
    
    // Add children to each creator
    creator1->addChild("creator1_child");
    creator2->addChild("creator2_child");
    
    // Verify they are independent
    auto children1 = creator1->getChildren();
    auto children2 = creator2->getChildren();
    
    EXPECT_EQ(children1.size(), 1);
    EXPECT_EQ(children2.size(), 1);
    EXPECT_EQ(children1[0], "creator1_child");
    EXPECT_EQ(children2[0], "creator2_child");
    
    // Test scene creation
    auto *scene1 = creator1->create();
    auto *scene2 = creator2->create();
    
    auto *testScene1 = qobject_cast<TestMenuScene*>(scene1);
    auto *testScene2 = qobject_cast<TestMenuScene*>(scene2);
    
    EXPECT_EQ(testScene1->name(), "Scene_1");
    EXPECT_EQ(testScene2->name(), "AlternativeScene");
    
    delete scene1;
    delete scene2;
}

TEST_F(AbstractSceneCreatorTest, Inheritance)
{
    // Test that derived classes properly inherit behavior
    auto derivedCreator = std::make_unique<AlternativeSceneCreator>();
    
    // Test inherited methods work
    EXPECT_TRUE(derivedCreator->addChild("inherited_child"));
    auto children = derivedCreator->getChildren();
    EXPECT_EQ(children.size(), 1);
    EXPECT_EQ(children[0], "inherited_child");
    
    derivedCreator->removeChild("inherited_child");
    children = derivedCreator->getChildren();
    EXPECT_TRUE(children.isEmpty());
    
    // Test create method is overridden
    auto *scene = derivedCreator->create();
    auto *testScene = qobject_cast<TestMenuScene*>(scene);
    EXPECT_EQ(testScene->name(), "AlternativeScene");
    
    delete scene;
}

TEST_F(AbstractSceneCreatorTest, EdgeCases)
{
    // Test with empty string child name
    EXPECT_TRUE(creator->addChild(""));
    auto children = creator->getChildren();
    EXPECT_TRUE(children.contains(""));
    
    creator->removeChild("");
    children = creator->getChildren();
    EXPECT_FALSE(children.contains(""));
    
    // Test with special characters in child names
    QStringList specialNames = {
        "child-with-dash",
        "child_with_underscore",
        "child.with.dots",
        "child with spaces",
        "child@#$%^&*()",
        "中文场景",
        "🎭emoji scene"
    };
    
    for (const QString &name : specialNames) {
        EXPECT_TRUE(creator->addChild(name));
    }
    
    children = creator->getChildren();
    for (const QString &name : specialNames) {
        EXPECT_TRUE(children.contains(name));
    }
    
    // Remove special names
    for (const QString &name : specialNames) {
        creator->removeChild(name);
    }
    
    children = creator->getChildren();
    for (const QString &name : specialNames) {
        EXPECT_FALSE(children.contains(name));
    }
}

TEST_F(AbstractSceneCreatorTest, LargeScaleOperations)
{
    // Test adding many children
    const int childCount = 1000;
    for (int i = 0; i < childCount; ++i) {
        EXPECT_TRUE(creator->addChild(QString("child_%1").arg(i)));
    }
    
    auto children = creator->getChildren();
    EXPECT_EQ(children.size(), childCount);
    
    // Test removing many children
    for (int i = 0; i < childCount; i += 2) {
        creator->removeChild(QString("child_%1").arg(i));
    }
    
    children = creator->getChildren();
    EXPECT_EQ(children.size(), childCount / 2);
}

TEST_F(AbstractSceneCreatorTest, MemoryManagement)
{
    // Test that created scenes are properly managed
    QList<AbstractMenuScene*> scenes;
    
    // Create multiple scenes
    for (int i = 0; i < 10; ++i) {
        auto *scene = creator->create();
        EXPECT_NE(scene, nullptr);
        scenes.append(scene);
    }
    
    // Verify all scenes are valid
    EXPECT_EQ(scenes.size(), 10);
    EXPECT_EQ(creator->getCreateCount(), 10);
    
    for (auto *scene : scenes) {
        auto *testScene = qobject_cast<TestMenuScene*>(scene);
        EXPECT_NE(testScene, nullptr);
    }
    
    // Clean up
    for (auto *scene : scenes) {
        delete scene;
    }
    scenes.clear();
}

#include "test_abstractscenecreator.moc"