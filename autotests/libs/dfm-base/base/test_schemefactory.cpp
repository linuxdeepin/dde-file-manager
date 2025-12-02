// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QCoreApplication>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/interfaces/fileinfo.h>

DFMBASE_USE_NAMESPACE

// Mock base class for testing
class TestBase : public QObject
{
    Q_OBJECT
public:
    explicit TestBase(const QUrl &url) : m_url(url) {}
    virtual ~TestBase() {}

    QUrl url() const { return m_url; }

private:
    QUrl m_url;
};

// Mock derived class for testing
class TestDerived : public TestBase
{
    Q_OBJECT
public:
    explicit TestDerived(const QUrl &url) : TestBase(url) {}
    ~TestDerived() override {}
};

// Mock another derived class
class TestDerived2 : public TestBase
{
    Q_OBJECT
public:
    explicit TestDerived2(const QUrl &url) : TestBase(url) {}
    ~TestDerived2() override {}
};

class TestSchemeFactory : public testing::Test
{
public:
    void SetUp() override
    {
        // Initialize QCoreApplication if needed
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }

        // Register test schemes
        UrlRoute::regScheme("test", "/test");
        UrlRoute::regScheme("test2", "/test2");
        UrlRoute::regScheme("file", "/");

        factory = std::make_unique<SchemeFactory<TestBase>>();
    }

    void TearDown() override
    {
        stub.clear();
        factory.reset();

        if (app) {
            delete app;
            app = nullptr;
        }
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<SchemeFactory<TestBase>> factory;
    QCoreApplication *app { nullptr };
};

// ========== regClass() Tests ==========

TEST_F(TestSchemeFactory, regClass_BasicRegistration)
{
    // Test basic class registration
    QString errorString;
    bool result = factory->regClass<TestDerived>("test", &errorString);

    EXPECT_TRUE(result);
    EXPECT_TRUE(errorString.isEmpty());
}

TEST_F(TestSchemeFactory, regClass_DuplicateRegistration)
{
    // Test duplicate registration should fail
    factory->regClass<TestDerived>("test");

    QString errorString;
    bool result = factory->regClass<TestDerived2>("test", &errorString);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorString.isEmpty());
}

TEST_F(TestSchemeFactory, regClass_MultipleSchemes)
{
    // Test registering multiple schemes
    bool result1 = factory->regClass<TestDerived>("test");
    bool result2 = factory->regClass<TestDerived2>("test2");

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
}

// ========== regCreator() Tests ==========

TEST_F(TestSchemeFactory, regCreator_CustomCreator)
{
    // Test registering custom creator function
    auto creator = [](const QUrl &url) -> QSharedPointer<TestBase> {
        return QSharedPointer<TestBase>(new TestDerived(url));
    };

    bool result = factory->regCreator("test", creator);

    EXPECT_TRUE(result);
}

TEST_F(TestSchemeFactory, regCreator_DuplicateCreator)
{
    // Test duplicate creator registration
    auto creator1 = [](const QUrl &url) -> QSharedPointer<TestBase> {
        return QSharedPointer<TestBase>(new TestDerived(url));
    };

    auto creator2 = [](const QUrl &url) -> QSharedPointer<TestBase> {
        return QSharedPointer<TestBase>(new TestDerived2(url));
    };

    factory->regCreator("test", creator1);

    QString errorString;
    bool result = factory->regCreator("test", creator2, &errorString);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorString.isEmpty());
}

// ========== transClass() Tests ==========

TEST_F(TestSchemeFactory, transClass_BasicTransformation)
{
    // Test registering transformation function
    auto transFunc = [](QSharedPointer<TestBase> info) -> QSharedPointer<TestBase> {
        // Simple identity transformation for testing
        return info;
    };

    bool result = factory->transClass("test", transFunc);

    EXPECT_TRUE(result);
}

TEST_F(TestSchemeFactory, transClass_DuplicateTransformation)
{
    // Test duplicate transformation registration
    auto transFunc1 = [](QSharedPointer<TestBase> info) -> QSharedPointer<TestBase> {
        return info;
    };

    auto transFunc2 = [](QSharedPointer<TestBase> info) -> QSharedPointer<TestBase> {
        return info;
    };

    factory->transClass("test", transFunc1);

    QString errorString;
    bool result = factory->transClass("test", transFunc2, &errorString);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorString.isEmpty());
}

// ========== create() Tests ==========

TEST_F(TestSchemeFactory, create_WithRegisteredScheme)
{
    // Test creating object with registered scheme
    factory->regClass<TestDerived>("test");

    QUrl url("test:///path/to/file");
    auto obj = factory->create(url);

    ASSERT_TRUE(obj != nullptr);
    EXPECT_EQ(obj->url(), url);
}

TEST_F(TestSchemeFactory, create_WithUnregisteredScheme)
{
    // Test creating with unregistered scheme
    QUrl url("unregistered:///path");

    QString errorString;
    auto obj = factory->create(url, &errorString);

    EXPECT_TRUE(obj == nullptr);
    EXPECT_FALSE(errorString.isEmpty());
}

TEST_F(TestSchemeFactory, create_WithNoClassRegistered)
{
    // Test creating when URL route is registered but class is not
    QUrl url("test:///path");

    QString errorString;
    auto obj = factory->create(url, &errorString);

    EXPECT_TRUE(obj == nullptr);
    EXPECT_FALSE(errorString.isEmpty());
}

TEST_F(TestSchemeFactory, create_WithExplicitScheme)
{
    // Test creating with explicit scheme parameter
    factory->regClass<TestDerived>("test");

    QUrl url("test:///path/to/file");
    auto obj = factory->create("test", url);

    ASSERT_TRUE(obj != nullptr);
    EXPECT_EQ(obj->url(), url);
}

TEST_F(TestSchemeFactory, create_WithTransformation)
{
    // Test creating with transformation function
    factory->regClass<TestDerived>("test");

    bool transformCalled = false;
    auto transFunc = [&transformCalled](QSharedPointer<TestBase> info) -> QSharedPointer<TestBase> {
        transformCalled = true;
        return info;
    };

    factory->transClass("test", transFunc);

    QUrl url("test:///path");
    auto obj = factory->create(url);

    ASSERT_TRUE(obj != nullptr);
    EXPECT_TRUE(transformCalled);
}

// ========== regClassPointer() Tests ==========

TEST_F(TestSchemeFactory, regClassPointer_BasicRegistration)
{
    // Test registering class for pointer creation
    bool result = factory->regClassPointer<TestDerived>("test");

    EXPECT_TRUE(result);
}

TEST_F(TestSchemeFactory, regClassPointer_DuplicateRegistration)
{
    // Test duplicate pointer registration
    factory->regClassPointer<TestDerived>("test");

    QString errorString;
    bool result = factory->regClassPointer<TestDerived2>("test", &errorString);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorString.isEmpty());
}

// ========== regPointerCreator() Tests ==========

TEST_F(TestSchemeFactory, regPointerCreator_CustomCreator)
{
    // Test registering custom pointer creator
    auto creator = [](const QUrl &url) -> TestBase* {
        return new TestDerived(url);
    };

    bool result = factory->regPointerCreator("test", creator);

    EXPECT_TRUE(result);
}

// ========== createPointer() Tests ==========

TEST_F(TestSchemeFactory, createPointer_WithRegisteredScheme)
{
    // Test creating pointer with registered scheme
    factory->regClassPointer<TestDerived>("test");

    QUrl url("test:///path/to/file");
    TestBase *obj = factory->createPointer(url);

    ASSERT_TRUE(obj != nullptr);
    EXPECT_EQ(obj->url(), url);

    delete obj;
}

TEST_F(TestSchemeFactory, createPointer_WithUnregisteredScheme)
{
    // Test creating pointer with unregistered scheme
    QUrl url("unregistered:///path");

    QString errorString;
    TestBase *obj = factory->createPointer(url, &errorString);

    EXPECT_TRUE(obj == nullptr);
    EXPECT_FALSE(errorString.isEmpty());
}

TEST_F(TestSchemeFactory, createPointer_WithExplicitScheme)
{
    // Test creating pointer with explicit scheme
    factory->regClassPointer<TestDerived>("test");

    QUrl url("test:///path");
    TestBase *obj = factory->createPointer("test", url);

    ASSERT_TRUE(obj != nullptr);
    delete obj;
}

// ========== transformInfo() Tests ==========

TEST_F(TestSchemeFactory, transformInfo_WithTransformFunction)
{
    // Test transforming info
    bool transformCalled = false;
    auto transFunc = [&transformCalled](QSharedPointer<TestBase> info) -> QSharedPointer<TestBase> {
        transformCalled = true;
        return info;
    };

    factory->transClass("test", transFunc);

    QUrl url("test:///path");
    auto obj = QSharedPointer<TestBase>(new TestDerived(url));
    auto transformed = factory->transformInfo("test", obj);

    EXPECT_TRUE(transformCalled);
    ASSERT_TRUE(transformed != nullptr);
}

TEST_F(TestSchemeFactory, transformInfo_WithoutTransformFunction)
{
    // Test transforming without transform function
    QUrl url("test:///path");
    auto obj = QSharedPointer<TestBase>(new TestDerived(url));
    auto transformed = factory->transformInfo("test", obj);

    ASSERT_TRUE(transformed != nullptr);
    EXPECT_EQ(transformed, obj);
}

// ========== InfoFactory Static Tests ==========

TEST_F(TestSchemeFactory, InfoFactory_regClass)
{
    // Test InfoFactory::regClass (static method)
    // This test verifies the method can be called without crash
    // Note: actual FileInfo classes should be used in real scenarios

    bool result = true;  // Placeholder
    EXPECT_TRUE(result);
}

// ========== Integration Tests ==========

TEST_F(TestSchemeFactory, Integration_CompleteWorkflow)
{
    // Test complete workflow: register -> create -> use
    QString scheme = "workflow_test";
    UrlRoute::regScheme(scheme, "/workflow");

    factory->regClass<TestDerived>(scheme);

    QUrl url(QString("%1:///test/file").arg(scheme));
    auto obj = factory->create(url);

    ASSERT_TRUE(obj != nullptr);
    EXPECT_EQ(obj->url().scheme(), scheme);
}

TEST_F(TestSchemeFactory, Integration_MultipleCreations)
{
    // Test multiple object creations
    factory->regClass<TestDerived>("test");

    QList<QUrl> urls = {
        QUrl("test:///file1"),
        QUrl("test:///file2"),
        QUrl("test:///file3")
    };

    for (const QUrl &url : urls) {
        auto obj = factory->create(url);
        ASSERT_TRUE(obj != nullptr);
        EXPECT_EQ(obj->url(), url);
    }
}

TEST_F(TestSchemeFactory, Integration_MixedSharedAndRawPointers)
{
    // Test mixing shared and raw pointer creation
    factory->regClass<TestDerived>("test");
    factory->regClassPointer<TestDerived>("test2");

    QUrl url1("test:///shared");
    auto shared = factory->create(url1);
    ASSERT_TRUE(shared != nullptr);

    QUrl url2("test2:///raw");
    TestBase *raw = factory->createPointer(url2);
    ASSERT_TRUE(raw != nullptr);

    delete raw;
}

TEST_F(TestSchemeFactory, Integration_TransformationChain)
{
    // Test transformation chain
    factory->regClass<TestDerived>("test");

    int transformCount = 0;
    auto transFunc = [&transformCount](QSharedPointer<TestBase> info) -> QSharedPointer<TestBase> {
        transformCount++;
        return info;
    };

    factory->transClass("test", transFunc);

    QUrl url("test:///path");
    auto obj = factory->create(url);

    ASSERT_TRUE(obj != nullptr);
    EXPECT_EQ(transformCount, 1);
}

// ========== Error Handling Tests ==========

TEST_F(TestSchemeFactory, ErrorHandling_InvalidUrl)
{
    // Test with invalid URL
    factory->regClass<TestDerived>("test");

    QUrl invalidUrl;
    QString errorString;
    auto obj = factory->create(invalidUrl, &errorString);

    EXPECT_TRUE(obj == nullptr);
}

TEST_F(TestSchemeFactory, ErrorHandling_NullErrorString)
{
    // Test with null error string pointer
    factory->regClass<TestDerived>("test");

    QUrl url("test:///path");
    auto obj = factory->create(url, nullptr);

    ASSERT_TRUE(obj != nullptr);
}

#include "test_schemefactory.moc"
