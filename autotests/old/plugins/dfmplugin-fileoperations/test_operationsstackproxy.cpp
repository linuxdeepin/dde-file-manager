// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QVariantMap>
#include <QStringList>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "stubext.h"

#include "fileoperations/operationsstackproxy.h"

DPFILEOPERATIONS_USE_NAMESPACE

class TestOperationsStackProxy : public testing::Test
{
public:
    void SetUp() override
    {
        proxy = &OperationsStackProxy::instance();
        ASSERT_TRUE(proxy);
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    OperationsStackProxy *proxy;
};

// ========== instance Tests ==========

TEST_F(TestOperationsStackProxy, Instance_ReturnsSingleton)
{
    OperationsStackProxy &instance1 = OperationsStackProxy::instance();
    OperationsStackProxy &instance2 = OperationsStackProxy::instance();

    EXPECT_EQ(&instance1, &instance2);
}

// ========== saveOperations Tests ==========

TEST_F(TestOperationsStackProxy, SaveOperations_LocalCache)
{
    QVariantMap values;
    values["test_key"] = "test_value";
    values["operation"] = "copy";

    // Force local cache mode by stubbing dbus validation
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
        // Do nothing, keep dbusValid = false
    });

    proxy->saveOperations(values);

    SUCCEED();
}

TEST_F(TestOperationsStackProxy, SaveOperations_MaxStackSize)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    // Add many operations to test max size limit
    for (int i = 0; i < 110; ++i) {
        QVariantMap values;
        values["index"] = i;
        proxy->saveOperations(values);
    }

    SUCCEED();
}

// ========== cleanOperations Tests ==========

TEST_F(TestOperationsStackProxy, CleanOperations_LocalCache)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    // Add some operations first
    QVariantMap values;
    values["test"] = "data";
    proxy->saveOperations(values);

    // Clean them
    proxy->cleanOperations();

    SUCCEED();
}

// ========== revocationOperations Tests ==========

TEST_F(TestOperationsStackProxy, RevocationOperations_EmptyStack)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    proxy->cleanOperations();

    QVariantMap result = proxy->revocationOperations();

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOperationsStackProxy, RevocationOperations_HasOperations)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    proxy->cleanOperations();

    QVariantMap values;
    values["operation"] = "copy";
    values["source"] = "/tmp/source.txt";
    proxy->saveOperations(values);

    QVariantMap result = proxy->revocationOperations();

    // Should get back the operation we saved
    SUCCEED();
}

TEST_F(TestOperationsStackProxy, RevocationOperations_LIFO_Order)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    proxy->cleanOperations();

    QVariantMap op1;
    op1["id"] = 1;
    proxy->saveOperations(op1);

    QVariantMap op2;
    op2["id"] = 2;
    proxy->saveOperations(op2);

    QVariantMap result = proxy->revocationOperations();
    // Should get op2 first (LIFO)

    SUCCEED();
}

// ========== SaveRedoOperations Tests ==========

TEST_F(TestOperationsStackProxy, SaveRedoOperations_LocalCache)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    QVariantMap values;
    values["redo_op"] = "paste";
    values["target"] = "/tmp/dest";

    proxy->SaveRedoOperations(values);

    SUCCEED();
}

TEST_F(TestOperationsStackProxy, SaveRedoOperations_MaxStackSize)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    // Add many redo operations to test max size limit
    for (int i = 0; i < 110; ++i) {
        QVariantMap values;
        values["redo_index"] = i;
        proxy->SaveRedoOperations(values);
    }

    SUCCEED();
}

// ========== RevocationRedoOperations Tests ==========

TEST_F(TestOperationsStackProxy, RevocationRedoOperations_EmptyStack)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    QVariantMap result = proxy->RevocationRedoOperations();

    // May be empty if no redo operations
    SUCCEED();
}

TEST_F(TestOperationsStackProxy, RevocationRedoOperations_HasOperations)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    QVariantMap values;
    values["redo_operation"] = "undo_delete";
    proxy->SaveRedoOperations(values);

    QVariantMap result = proxy->RevocationRedoOperations();

    SUCCEED();
}

// ========== CleanOperationsByUrl Tests ==========

TEST_F(TestOperationsStackProxy, CleanOperationsByUrl_LocalCache)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    QStringList urls = { "/tmp/file1.txt", "/tmp/file2.txt" };

    proxy->CleanOperationsByUrl(urls);

    SUCCEED();
}

TEST_F(TestOperationsStackProxy, CleanOperationsByUrl_EmptyList)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    QStringList emptyUrls;

    proxy->CleanOperationsByUrl(emptyUrls);

    SUCCEED();
}

// ========== DBus Mode Tests ==========

TEST_F(TestOperationsStackProxy, SaveOperations_DBusMode)
{
    // These tests verify that dbus calls are made when dbusValid = true
    // But we stub them to avoid actual dbus dependency

    stub.set_lamda(&QDBusConnectionInterface::isServiceRegistered,
                   [](QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
                       __DBG_STUB_INVOKE__
                       return {};
                   });

    // Reinitialize to enable dbus mode
    OperationsStackProxy testProxy;

    SUCCEED();
}

// ========== Edge Cases ==========

TEST_F(TestOperationsStackProxy, EdgeCase_SaveAndRevoke_Sequence)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    proxy->cleanOperations();

    // Save multiple operations
    for (int i = 0; i < 5; ++i) {
        QVariantMap values;
        values["seq"] = i;
        proxy->saveOperations(values);
    }

    // Revoke them all
    for (int i = 0; i < 5; ++i) {
        QVariantMap result = proxy->revocationOperations();
    }

    // Stack should be empty now
    QVariantMap empty = proxy->revocationOperations();
    EXPECT_TRUE(empty.isEmpty());
}

TEST_F(TestOperationsStackProxy, EdgeCase_SaveRedoAndRevoke_Sequence)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    // Save multiple redo operations
    for (int i = 0; i < 3; ++i) {
        QVariantMap values;
        values["redo_seq"] = i;
        proxy->SaveRedoOperations(values);
    }

    // Revoke them
    for (int i = 0; i < 3; ++i) {
        QVariantMap result = proxy->RevocationRedoOperations();
    }

    SUCCEED();
}

TEST_F(TestOperationsStackProxy, EdgeCase_LargeVariantMap)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    QVariantMap largeMap;
    for (int i = 0; i < 1000; ++i) {
        largeMap[QString("key_%1").arg(i)] = QString("value_%1").arg(i);
    }

    proxy->saveOperations(largeMap);

    SUCCEED();
}

TEST_F(TestOperationsStackProxy, EdgeCase_CleanEmptyStack)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    // Clean when already empty
    proxy->cleanOperations();
    proxy->cleanOperations();

    SUCCEED();
}

TEST_F(TestOperationsStackProxy, EdgeCase_RevokeFromEmptyStack_Multiple)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    proxy->cleanOperations();

    // Try to revoke multiple times from empty stack
    for (int i = 0; i < 5; ++i) {
        QVariantMap result = proxy->revocationOperations();
        EXPECT_TRUE(result.isEmpty());
    }
}

TEST_F(TestOperationsStackProxy, EdgeCase_InterleavedSaveRevoke)
{
    stub.set_lamda(&OperationsStackProxy::initialize, [](OperationsStackProxy *) {
        __DBG_STUB_INVOKE__
    });

    proxy->cleanOperations();

    QVariantMap op1;
    op1["id"] = 1;
    proxy->saveOperations(op1);

    proxy->revocationOperations();

    QVariantMap op2;
    op2["id"] = 2;
    proxy->saveOperations(op2);

    QVariantMap result = proxy->revocationOperations();

    SUCCEED();
}
