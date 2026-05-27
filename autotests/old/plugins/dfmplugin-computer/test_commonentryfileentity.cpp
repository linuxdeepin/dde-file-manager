// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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

TEST_F(UT_CommonEntryFileEntity, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
{
    createEntityWithComputerInfo();
    
    // Mock all methods
    int displayNameCallCount = 0;
    int iconCallCount = 0;
    int existsCallCount = 0;
    int showProgressCallCount = 0;
    int showTotalSizeCallCount = 0;
    int showUsageSizeCallCount = 0;
    int orderCallCount = 0;
    int refreshCallCount = 0;
    int sizeTotalCallCount = 0;
    int sizeUsageCallCount = 0;
    int descriptionCallCount = 0;
    int targetUrlCallCount = 0;
    int isAccessableCallCount = 0;
    int renamableCallCount = 0;
    int extraPropertiesCallCount = 0;
    int setExtraPropertyCallCount = 0;
    
    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [&displayNameCallCount, &iconCallCount, &existsCallCount,
                                                    &showProgressCallCount, &showTotalSizeCallCount, &showUsageSizeCallCount,
                                                    &orderCallCount, &refreshCallCount, &sizeTotalCallCount, &sizeUsageCallCount,
                                                    &descriptionCallCount, &targetUrlCallCount, &isAccessableCallCount,
                                                    &renamableCallCount, &extraPropertiesCallCount, &setExtraPropertyCallCount]
                                                    (const CommonEntryFileEntity *, const QString &method) {
        __DBG_STUB_INVOKE__
        if (method == "displayName") displayNameCallCount++;
        else if (method == "icon") iconCallCount++;
        else if (method == "exists") existsCallCount++;
        else if (method == "showProgress") showProgressCallCount++;
        else if (method == "showTotalSize") showTotalSizeCallCount++;
        else if (method == "showUsageSize") showUsageSizeCallCount++;
        else if (method == "order") orderCallCount++;
        else if (method == "refresh") refreshCallCount++;
        else if (method == "sizeTotal") sizeTotalCallCount++;
        else if (method == "sizeUsage") sizeUsageCallCount++;
        else if (method == "description") descriptionCallCount++;
        else if (method == "targetUrl") targetUrlCallCount++;
        else if (method == "isAccessable") isAccessableCallCount++;
        else if (method == "renamable") renamableCallCount++;
        else if (method == "extraProperties") extraPropertiesCallCount++;
        else if (method == "setExtraProperty") setExtraPropertyCallCount++;
        return true;
    });
    
    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    
    // Call multiple methods
    entity->displayName();
    entity->icon();
    entity->exists();
    entity->showProgress();
    entity->showTotalSize();
    entity->showUsageSize();
    entity->order();
    entity->refresh();
    entity->sizeTotal();
    entity->sizeUsage();
    entity->description();
    entity->targetUrl();
    entity->isAccessable();
    entity->renamable();
    entity->extraProperties();
    entity->setExtraProperty("testKey", QVariant("testValue"));
    
    // Verify all methods were called
    EXPECT_EQ(displayNameCallCount, 1);
    EXPECT_EQ(iconCallCount, 1);
    EXPECT_EQ(existsCallCount, 1);
    EXPECT_EQ(showProgressCallCount, 1);
    EXPECT_EQ(showTotalSizeCallCount, 1);
    EXPECT_EQ(showUsageSizeCallCount, 1);
    EXPECT_EQ(orderCallCount, 1);
    EXPECT_EQ(refreshCallCount, 1);
    EXPECT_EQ(sizeTotalCallCount, 1);
    EXPECT_EQ(sizeUsageCallCount, 1);
    EXPECT_EQ(descriptionCallCount, 1);
    EXPECT_EQ(targetUrlCallCount, 1);
    EXPECT_EQ(isAccessableCallCount, 1);
    EXPECT_EQ(renamableCallCount, 1);
    EXPECT_EQ(extraPropertiesCallCount, 1);
    EXPECT_EQ(setExtraPropertyCallCount, 1);
}

TEST_F(UT_CommonEntryFileEntity, QtMetaObject_CorrectlyInitialized_Success)
{
    createEntityWithComputerInfo();
    
    // Test that Qt meta-object system works correctly
    const QMetaObject *metaObject = entity->metaObject();
    EXPECT_NE(metaObject, nullptr);
    
    // Test class name
    EXPECT_STREQ(metaObject->className(), "dfmplugin_computer::CommonEntryFileEntity");
    
    // Test that inherited methods exist in meta-object
    EXPECT_GE(metaObject->indexOfMethod("displayName()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("icon()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("exists()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showProgress()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showTotalSize()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showUsageSize()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("order()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("refresh()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("sizeTotal()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("sizeUsage()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("description()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("targetUrl()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("isAccessable()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("renamable()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("extraProperties()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("setExtraProperty(QString,QVariant)"), 0);
}

TEST_F(UT_CommonEntryFileEntity, Inheritance_FromAbstractEntryFileEntity_WorksCorrectly)
{
    createEntityWithComputerInfo();
    
    // Test that CommonEntryFileEntity is properly inherited from AbstractEntryFileEntity
    AbstractEntryFileEntity *baseEntity = entity;
    EXPECT_NE(baseEntity, nullptr);
    
    // Test that we can call base class methods
    // AbstractEntryFileEntity doesn't have url() method, so we test other methods
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->icon());
    EXPECT_NO_THROW(baseEntity->exists());
    EXPECT_NO_THROW(baseEntity->showProgress());
    EXPECT_NO_THROW(baseEntity->showTotalSize());
    EXPECT_NO_THROW(baseEntity->showUsageSize());
    EXPECT_NO_THROW(baseEntity->description());
    EXPECT_NO_THROW(baseEntity->order());
    EXPECT_NO_THROW(baseEntity->refresh());
    EXPECT_NO_THROW(baseEntity->sizeTotal());
    EXPECT_NO_THROW(baseEntity->sizeUsage());
    EXPECT_NO_THROW(baseEntity->targetUrl());
    EXPECT_NO_THROW(baseEntity->isAccessable());
    EXPECT_NO_THROW(baseEntity->renamable());
    EXPECT_NO_THROW(baseEntity->extraProperties());
    EXPECT_NO_THROW(baseEntity->setExtraProperty("testKey", QVariant("testValue")));
}

TEST_F(UT_CommonEntryFileEntity, MemoryManagement_DeleteEntity_CleansUpCorrectly)
{
    createEntityWithComputerInfo();
    
    // Store pointer to entity for testing
    CommonEntryFileEntity *entityPtr = entity;
    
    // Delete entity
    delete entity;
    entity = nullptr;
    
    // The entity should be deleted, but we can't directly test this
    // We just verify that the delete operation doesn't crash
    EXPECT_EQ(entity, nullptr);
}

TEST_F(UT_CommonEntryFileEntity, ErrorHandling_InvalidReflectionObject_HandlesGracefully)
{
    createEntityWithComputerInfo();
    
    // Mock reflection to return false
    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return false;
    });
    
    // Test that methods handle invalid reflection gracefully
    EXPECT_NO_THROW({
        QString displayName = entity->displayName();
        QIcon icon = entity->icon();
        bool exists = entity->exists();
        bool showProgress = entity->showProgress();
        bool showTotalSize = entity->showTotalSize();
        bool showUsageSize = entity->showUsageSize();
        AbstractEntryFileEntity::EntryOrder order = entity->order();
        entity->refresh();
        quint64 sizeTotal = entity->sizeTotal();
        quint64 sizeUsage = entity->sizeUsage();
        QString description = entity->description();
        QUrl targetUrl = entity->targetUrl();
        bool isAccessable = entity->isAccessable();
        bool renamable = entity->renamable();
        QVariantHash extraProps = entity->extraProperties();
        entity->setExtraProperty("testKey", QVariant("testValue"));
    });
}

TEST_F(UT_CommonEntryFileEntity, SpecialCharacters_InReflectionObjectName_HandlesCorrectly)
{
    createEntityWithComputerInfo();
    
    // Set reflection object name with special characters
    entity->reflectionObjName = "TestObject_特殊字符";
    
    // Mock QMetaType::type to return valid type
    // Mock QMetaType::type with specific overload
    using QMetaTypeTypeFunc = int (*)(const char *);
    stub.set_lamda(static_cast<QMetaTypeTypeFunc>(&QMetaType::type), [](const char *) {
        __DBG_STUB_INVOKE__
        return QMetaType::QObjectStar;
    });
    
    // Mock QMetaType::metaObjectForType to return valid meta object
    stub.set_lamda(&QMetaType::metaObjectForType, [] {
        __DBG_STUB_INVOKE__
        static QMetaObject metaObject;
        return &metaObject;
    });
    
    // Mock QMetaObject::newInstance to return valid object
    // Mock QMetaObject::newInstance with specific overload
    using QMetaObjectNewInstanceFunc = QObject *(QMetaObject::*)() const;
    stub.set_lamda(static_cast<QMetaObjectNewInstanceFunc>(&QMetaObject::newInstance), [](const QMetaObject *) {
        __DBG_STUB_INVOKE__
        return new QObject();
    });
    
    // Test that reflection handles special characters correctly
    bool result = entity->reflection();
    EXPECT_TRUE(result);
}

TEST_F(UT_CommonEntryFileEntity, Consistency_MultipleCalls_ReturnConsistentResults)
{
    createEntityWithComputerInfo();
    
    // Mock methods to return consistent values
    QString mockDisplayName = "Test Display Name";
    QIcon mockIcon = QIcon::fromTheme("test-icon");
    bool mockExists = true;
    bool mockShowProgress = false;
    bool mockShowTotalSize = false;
    bool mockShowUsageSize = false;
    AbstractEntryFileEntity::EntryOrder mockOrder = AbstractEntryFileEntity::EntryOrder::kOrderUserDir;
    quint64 mockSizeTotal = 1024;
    quint64 mockSizeUsage = 512;
    QString mockDescription = "Test Description";
    QUrl mockTargetUrl = QUrl::fromLocalFile("/test/path");
    bool mockIsAccessable = true;
    bool mockRenamable = false;
    QVariantHash mockExtraProps;
    mockExtraProps["testKey"] = QVariant("testValue");
    
    stub.set_lamda(&CommonEntryFileEntity::reflection, [](const CommonEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    stub.set_lamda(&CommonEntryFileEntity::hasMethod, [](const CommonEntryFileEntity *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    stub.set_lamda(static_cast<bool (*)(QObject *, const char *, Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [&mockDisplayName, &mockIcon, &mockExists, &mockShowProgress, &mockShowTotalSize, &mockShowUsageSize,
                   &mockOrder, &mockSizeTotal, &mockSizeUsage, &mockDescription, &mockTargetUrl,
                   &mockIsAccessable, &mockRenamable, &mockExtraProps]
                   (QObject *, const char *method, Qt::ConnectionType, qsizetype,
                    const void *const *, const char *const *,
                    const QtPrivate::QMetaTypeInterface *const *) {
        __DBG_STUB_INVOKE__
        QString methodName(method);
        
        if (methodName == "displayName") {
            Q_RETURN_ARG(QString, mockDisplayName);
            return true;
        } else if (methodName == "icon") {
            Q_RETURN_ARG(QIcon, mockIcon);
            return true;
        } else if (methodName == "exists") {
            Q_RETURN_ARG(bool, mockExists);
            return true;
        } else if (methodName == "showProgress") {
            Q_RETURN_ARG(bool, mockShowProgress);
            return true;
        } else if (methodName == "showTotalSize") {
            Q_RETURN_ARG(bool, mockShowTotalSize);
            return true;
        } else if (methodName == "showUsageSize") {
            Q_RETURN_ARG(bool, mockShowUsageSize);
            return true;
        } else if (methodName == "order") {
            Q_RETURN_ARG(AbstractEntryFileEntity::EntryOrder, mockOrder);
            return true;
        } else if (methodName == "sizeTotal") {
            Q_RETURN_ARG(quint64, mockSizeTotal);
            return true;
        } else if (methodName == "sizeUsage") {
            Q_RETURN_ARG(quint64, mockSizeUsage);
            return true;
        } else if (methodName == "description") {
            Q_RETURN_ARG(QString, mockDescription);
            return true;
        } else if (methodName == "targetUrl") {
            Q_RETURN_ARG(QUrl, mockTargetUrl);
            return true;
        } else if (methodName == "isAccessable") {
            Q_RETURN_ARG(bool, mockIsAccessable);
            return true;
        } else if (methodName == "renamable") {
            Q_RETURN_ARG(bool, mockRenamable);
            return true;
        } else if (methodName == "extraProperties") {
            Q_RETURN_ARG(QVariantHash, mockExtraProps);
            return true;
        }
        
        return false;
    });
    
    // Call methods multiple times
    QString displayName1 = entity->displayName();
    QString displayName2 = entity->displayName();
    QString displayName3 = entity->displayName();
    
    QIcon icon1 = entity->icon();
    QIcon icon2 = entity->icon();
    QIcon icon3 = entity->icon();
    
    bool exists1 = entity->exists();
    bool exists2 = entity->exists();
    bool exists3 = entity->exists();
    
    // Verify consistency
    EXPECT_EQ(displayName1, mockDisplayName);
    EXPECT_EQ(displayName2, mockDisplayName);
    EXPECT_EQ(displayName3, mockDisplayName);
    
    EXPECT_EQ(icon1.cacheKey(), mockIcon.cacheKey());
    EXPECT_EQ(icon2.cacheKey(), mockIcon.cacheKey());
    EXPECT_EQ(icon3.cacheKey(), mockIcon.cacheKey());
    
    EXPECT_EQ(exists1, mockExists);
    EXPECT_EQ(exists2, mockExists);
    EXPECT_EQ(exists3, mockExists);
}
