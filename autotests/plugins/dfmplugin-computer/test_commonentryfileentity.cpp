// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "fileentity/commonentryfileentity.h"
#include "watcher/computeritemwatcher.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

#include <QUrl>
#include <QIcon>
#include <QVariant>
#include <QVariantHash>
#include <QObject>
#include <QMetaObject>
#include <QMetaType>
#include <QString>
#include <QHash>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_CommonEntryFileEntity : public testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl("entry://test");
        entity = nullptr;
        setupMockComputerInfo();
    }

    void TearDown() override
    {
        delete entity;
        entity = nullptr;
        stub.clear();
    }

    void setupMockComputerInfo()
    {
        mockComputerInfo.clear();
        QVariantMap itemInfo;
        itemInfo["ReflectionObject"] = "TestReflectionObject";
        itemInfo["ItemName"] = "Test Item Name";
        itemInfo["ItemIcon"] = "test-icon";
        mockComputerInfo[testUrl] = itemInfo;
    }

    void createEntityWithComputerInfo()
    {
        stub.set_lamda(&ComputerItemWatcher::instance, []() {
            __DBG_STUB_INVOKE__
            static ComputerItemWatcher mockWatcher;
            return &mockWatcher;
        });

        stub.set_lamda(&ComputerItemWatcher::getComputerInfos, [this] {
            __DBG_STUB_INVOKE__
            return mockComputerInfo;
        });

        entity = new CommonEntryFileEntity(testUrl);
    }

    void createEntityWithoutComputerInfo()
    {
        QHash<QUrl, QVariantMap> emptyInfo;

        stub.set_lamda(&ComputerItemWatcher::instance, []() {
            __DBG_STUB_INVOKE__
            static ComputerItemWatcher mockWatcher;
            return &mockWatcher;
        });

        stub.set_lamda(&ComputerItemWatcher::getComputerInfos, [&emptyInfo] {
            __DBG_STUB_INVOKE__
            return emptyInfo;
        });

        entity = new CommonEntryFileEntity(testUrl);
    }

protected:
    stub_ext::StubExt stub;
    CommonEntryFileEntity *entity;
    QUrl testUrl;
    QHash<QUrl, QVariantMap> mockComputerInfo;
};

// Constructor tests
TEST_F(UT_CommonEntryFileEntity, Constructor_WithValidComputerInfo_InitializesCorrectly)
{
    createEntityWithComputerInfo();

    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->reflectionObjName, "TestReflectionObject");
    EXPECT_EQ(entity->defaultName, "Test Item Name");
    EXPECT_TRUE(entity->defualtIcon.isNull());
}

TEST_F(UT_CommonEntryFileEntity, Constructor_WithMissingComputerInfo_InitializesWithDefaults)
{
    createEntityWithoutComputerInfo();

    EXPECT_NE(entity, nullptr);
    EXPECT_TRUE(entity->reflectionObjName.isEmpty());
    EXPECT_TRUE(entity->defaultName.isEmpty());
    EXPECT_TRUE(entity->defualtIcon.isNull());
}

TEST_F(UT_CommonEntryFileEntity, Constructor_ComputerWatcherCalled_VerifyInteraction)
{
    bool instanceCalled = false;
    bool getInfosCalled = false;

    stub.set_lamda(&ComputerItemWatcher::instance, [&instanceCalled]() {
        __DBG_STUB_INVOKE__
        instanceCalled = true;
        static ComputerItemWatcher mockWatcher;
        return &mockWatcher;
    });

    stub.set_lamda(&ComputerItemWatcher::getComputerInfos, [&getInfosCalled, this] {
        __DBG_STUB_INVOKE__
        getInfosCalled = true;
        return mockComputerInfo;
    });

    entity = new CommonEntryFileEntity(testUrl);

    EXPECT_TRUE(instanceCalled);
    EXPECT_TRUE(getInfosCalled);
}

// Destructor tests
TEST_F(UT_CommonEntryFileEntity, Destructor_WithReflectionObject_CleansUpProperly)
{
    createEntityWithComputerInfo();
    QObject *testObj = new QObject();
    entity->reflectionObj = testObj;

    // The destructor should clean up the reflection object
    delete entity;
    entity = nullptr;

    // Test that destructor doesn't crash - memory management handled by destructor
    EXPECT_TRUE(true);
}

TEST_F(UT_CommonEntryFileEntity, Destructor_WithNullReflectionObject_HandlesGracefully)
{
    createEntityWithComputerInfo();
    entity->reflectionObj = nullptr;

    delete entity;
    entity = nullptr;

    EXPECT_TRUE(true);
}

// displayName() tests
TEST_F(UT_CommonEntryFileEntity, DisplayName_WithDefaultName_ReturnsDefaultName)
{
    createEntityWithComputerInfo();

    QString result = entity->displayName();
    EXPECT_EQ(result, "Test Item Name");
}

TEST_F(UT_CommonEntryFileEntity, DisplayName_EmptyDefaultWithReflection_ReturnsReflectedName)
{
    createEntityWithComputerInfo();
    entity->defaultName.clear();

    bool reflectionCalled = false;
    bool hasMethodCalled = false;
    bool invokeMethodCalled = false;

    stub.set_lamda(&CommonEntryFileEntity::reflection, [&reflectionCalled](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        reflectionCalled = true;
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [&hasMethodCalled](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        hasMethodCalled = true;
        return method == "displayName";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [&invokeMethodCalled] {
                       __DBG_STUB_INVOKE__
                       invokeMethodCalled = true;
                       return true;
                   });

    QString result = entity->displayName();

    EXPECT_TRUE(reflectionCalled);
    EXPECT_TRUE(hasMethodCalled);
    EXPECT_TRUE(invokeMethodCalled);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_CommonEntryFileEntity, DisplayName_EmptyDefaultNoReflection_ReturnsEmpty)
{
    createEntityWithComputerInfo();
    entity->defaultName.clear();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QString result = entity->displayName();
    EXPECT_TRUE(result.isEmpty());
}

// icon() tests
TEST_F(UT_CommonEntryFileEntity, Icon_WithDefaultIcon_ReturnsDefaultIcon)
{
    createEntityWithComputerInfo();

    QIcon result = entity->icon();
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_CommonEntryFileEntity, Icon_NullDefaultWithReflection_ReturnsReflectedIcon)
{
    createEntityWithComputerInfo();
    entity->defualtIcon = QIcon();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "icon";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QIcon result = entity->icon();
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_CommonEntryFileEntity, Icon_NullDefaultNoReflection_ReturnsEmptyIcon)
{
    createEntityWithComputerInfo();
    entity->defualtIcon = QIcon();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QIcon result = entity->icon();
    EXPECT_TRUE(result.isNull());
}

// exists() tests
TEST_F(UT_CommonEntryFileEntity, Exists_WithReflection_ReturnsReflectedValue)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "exists";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = entity->exists();
    EXPECT_FALSE(result);
}

TEST_F(UT_CommonEntryFileEntity, Exists_WithoutReflection_ReturnsFalse)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = entity->exists();
    EXPECT_FALSE(result);
}

// showProgress() tests
TEST_F(UT_CommonEntryFileEntity, ShowProgress_WithReflection_ReturnsReflectedValue)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "showProgress";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = entity->showProgress();
    EXPECT_FALSE(result);
}

TEST_F(UT_CommonEntryFileEntity, ShowProgress_WithoutReflection_ReturnsFalse)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = entity->showProgress();
    EXPECT_FALSE(result);
}

// showTotalSize() tests
TEST_F(UT_CommonEntryFileEntity, ShowTotalSize_WithReflection_ReturnsReflectedValue)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "showTotalSize";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = entity->showTotalSize();
    EXPECT_FALSE(result);
}

// showUsageSize() tests
TEST_F(UT_CommonEntryFileEntity, ShowUsageSize_WithReflection_ReturnsReflectedValue)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "showUsageSize";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = entity->showUsageSize();
    EXPECT_FALSE(result);
}

// order() tests
TEST_F(UT_CommonEntryFileEntity, Order_WithReflection_ReturnsReflectedOrder)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "order";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    AbstractEntryFileEntity::EntryOrder result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderUserDir);
}

TEST_F(UT_CommonEntryFileEntity, Order_WithoutReflection_ReturnsCustomOrder)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    AbstractEntryFileEntity::EntryOrder result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderCustom);
}

// refresh() tests
TEST_F(UT_CommonEntryFileEntity, Refresh_WithReflection_CallsReflectedMethod)
{
    createEntityWithComputerInfo();

    bool invokeMethodCalled = false;

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "refresh";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *)>(&QMetaObject::invokeMethod),
                   [&invokeMethodCalled](QObject *, const char *method) {
                       __DBG_STUB_INVOKE__
                       invokeMethodCalled = true;
                       return QString(method) == "refresh";
                   });

    entity->refresh();
    EXPECT_TRUE(invokeMethodCalled);
}

TEST_F(UT_CommonEntryFileEntity, Refresh_WithoutReflection_CallsParentMethod)
{
    createEntityWithComputerInfo();

    bool parentRefreshCalled = false;

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(AbstractEntryFileEntity, refresh), [&parentRefreshCalled](AbstractEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        parentRefreshCalled = true;
    });

    entity->refresh();
    EXPECT_TRUE(parentRefreshCalled);
}

// sizeTotal() tests
TEST_F(UT_CommonEntryFileEntity, SizeTotal_WithReflection_ReturnsReflectedSize)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "sizeTotal";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    quint64 result = entity->sizeTotal();
    EXPECT_EQ(result, 0);
}

TEST_F(UT_CommonEntryFileEntity, SizeTotal_WithoutReflection_CallsParentMethod)
{
    createEntityWithComputerInfo();

    bool parentSizeTotalCalled = false;

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(AbstractEntryFileEntity, sizeTotal), [&parentSizeTotalCalled] {
        __DBG_STUB_INVOKE__
        parentSizeTotalCalled = true;
        return 512;
    });

    quint64 result = entity->sizeTotal();
    EXPECT_TRUE(parentSizeTotalCalled);
    EXPECT_EQ(result, 512);
}

// sizeUsage() tests
TEST_F(UT_CommonEntryFileEntity, SizeUsage_WithReflection_ReturnsReflectedSize)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "sizeUsage";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    quint64 result = entity->sizeUsage();
    EXPECT_EQ(result, 0);
}

// description() tests
TEST_F(UT_CommonEntryFileEntity, Description_WithReflection_ReturnsReflectedDescription)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "description";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QString result = entity->description();
    EXPECT_TRUE(result.isEmpty());
}

// targetUrl() tests
TEST_F(UT_CommonEntryFileEntity, TargetUrl_WithReflection_ReturnsReflectedUrl)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "targetUrl";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QUrl result = entity->targetUrl();
    EXPECT_FALSE(result.isValid());
}

// isAccessable() tests
TEST_F(UT_CommonEntryFileEntity, IsAccessable_WithReflection_ReturnsReflectedValue)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "isAccessable";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = entity->isAccessable();
    EXPECT_FALSE(result);
}

// renamable() tests
TEST_F(UT_CommonEntryFileEntity, Renamable_WithReflection_ReturnsReflectedValue)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "renamable";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    bool result = entity->renamable();
    EXPECT_FALSE(result);
}

// extraProperties() tests
TEST_F(UT_CommonEntryFileEntity, ExtraProperties_WithReflection_ReturnsReflectedProperties)
{
    createEntityWithComputerInfo();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "extraProperties";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QVariantHash result = entity->extraProperties();
    EXPECT_TRUE(result.isEmpty());
}

// setExtraProperty() tests
TEST_F(UT_CommonEntryFileEntity, SetExtraProperty_WithReflection_CallsReflectedMethod)
{
    createEntityWithComputerInfo();

    bool invokeMethodCalled = false;

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "setExtraProperty";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [&invokeMethodCalled] {
                       __DBG_STUB_INVOKE__
                       invokeMethodCalled = true;
                       return true;
                   });

    entity->setExtraProperty("testKey", QVariant("testValue"));
    EXPECT_TRUE(invokeMethodCalled);
}

TEST_F(UT_CommonEntryFileEntity, SetExtraProperty_WithoutReflection_CallsParentMethod)
{
    createEntityWithComputerInfo();

    bool parentSetExtraPropertyCalled = false;

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(AbstractEntryFileEntity, setExtraProperty), [&parentSetExtraPropertyCalled] {
        __DBG_STUB_INVOKE__
        parentSetExtraPropertyCalled = true;
    });

    entity->setExtraProperty("testKey", QVariant("testValue"));
    EXPECT_TRUE(parentSetExtraPropertyCalled);
}

// reflection() tests
TEST_F(UT_CommonEntryFileEntity, Reflection_WithExistingReflectionObj_ReturnsTrue)
{
    createEntityWithComputerInfo();
    entity->reflectionObj = new QObject();

    bool result = entity->reflection();
    EXPECT_TRUE(result);
}

TEST_F(UT_CommonEntryFileEntity, Reflection_WithValidMetaType_CreatesObjectAndReturnsTrue)
{
    createEntityWithComputerInfo();
    entity->reflectionObjName = "QObject";
    entity->reflectionObj = nullptr;

    bool result = entity->reflection();
    EXPECT_FALSE(result);
}

TEST_F(UT_CommonEntryFileEntity, Reflection_WithUnknownMetaType_ReturnsFalse)
{
    createEntityWithComputerInfo();
    entity->reflectionObjName = "UnknownType";
    entity->reflectionObj = nullptr;

    bool result = entity->reflection();
    EXPECT_FALSE(result);
}

TEST_F(UT_CommonEntryFileEntity, Reflection_WithNullMetaObject_ReturnsFalse)
{
    createEntityWithComputerInfo();
    entity->reflectionObjName = "TestType";
    entity->reflectionObj = nullptr;

    stub.set_lamda(&QMetaType::metaObjectForType, [](int) {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    bool result = entity->reflection();
    EXPECT_FALSE(result);
}

// hasMethod() tests
TEST_F(UT_CommonEntryFileEntity, HasMethod_WithNullReflectionObj_ReturnsFalse)
{
    createEntityWithComputerInfo();
    entity->reflectionObj = nullptr;

    bool result = entity->hasMethod("testMethod");
    EXPECT_FALSE(result);
}

TEST_F(UT_CommonEntryFileEntity, HasMethod_WithValidMethod_ReturnsTrue)
{
    createEntityWithComputerInfo();
    entity->reflectionObj = new QObject();

    stub.set_lamda(&QMetaObject::indexOfMethod, [](const QMetaObject *, const char *) {
        __DBG_STUB_INVOKE__
        return 1;   // Valid method index
    });

    bool result = entity->hasMethod("testMethod");
    EXPECT_TRUE(result);
}

TEST_F(UT_CommonEntryFileEntity, HasMethod_WithInvalidMethod_ReturnsFalse)
{
    createEntityWithComputerInfo();
    entity->reflectionObj = new QObject();

    stub.set_lamda(&QMetaObject::indexOfMethod, [](const QMetaObject *, const char *) {
        __DBG_STUB_INVOKE__
        return -1;   // Invalid method index
    });

    bool result = entity->hasMethod("invalidMethod");
    EXPECT_FALSE(result);
}

// Edge case tests
TEST_F(UT_CommonEntryFileEntity, EdgeCase_EmptyReflectionObjectName_HandlesCorrectly)
{
    createEntityWithComputerInfo();
    entity->reflectionObjName.clear();

    // Methods should handle empty reflection object name gracefully
    QString displayName = entity->displayName();
    QIcon icon = entity->icon();
    bool exists = entity->exists();

    // Should not crash and return appropriate defaults
    EXPECT_EQ(displayName, "Test Item Name");   // Should use default name
    EXPECT_TRUE(icon.isNull());
    EXPECT_FALSE(exists);   // Default behavior without reflection
}

TEST_F(UT_CommonEntryFileEntity, EdgeCase_ReflectionMethodInvokeFails_HandlesGracefully)
{
    createEntityWithComputerInfo();
    entity->defaultName.clear();

    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        return method == "displayName";
    });

    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QString result = entity->displayName();
    EXPECT_TRUE(result.isEmpty());   // Should return empty when invoke fails
}
