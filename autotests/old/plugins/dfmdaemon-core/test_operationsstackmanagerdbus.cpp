// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "operationsstackmanagerdbus.h"

#include <QVariantMap>
#include <QStringList>

class TestOperationsStackManagerDbus : public testing::Test
{
protected:
    void SetUp() override
    {
        operationsManager = new OperationsStackManagerDbus();
    }

    void TearDown() override
    {
        delete operationsManager;
        stub.clear();
    }

    OperationsStackManagerDbus *operationsManager { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TestOperationsStackManagerDbus, Constructor_InitializesCorrectly)
{
    OperationsStackManagerDbus testManager;
    
    // Constructor should create empty stacks
    QVariantMap result = testManager.RevocationOperations();
    EXPECT_TRUE(result.isEmpty());
    
    QVariantMap redoResult = testManager.RevocationRedoOperations();
    EXPECT_TRUE(redoResult.isEmpty());
}

TEST_F(TestOperationsStackManagerDbus, SaveOperations_SingleOperation_Success)
{
    QVariantMap testOperation;
    testOperation["action"] = "copy";
    testOperation["source"] = "/home/user/file1.txt";
    testOperation["target"] = "/home/user/backup/file1.txt";

    operationsManager->SaveOperations(testOperation);

    QVariantMap result = operationsManager->RevocationOperations();
    EXPECT_EQ(result, testOperation);
}

TEST_F(TestOperationsStackManagerDbus, SaveOperations_MultipleOperations_LIFO_Order)
{
    QVariantMap operation1;
    operation1["action"] = "copy";
    operation1["id"] = 1;

    QVariantMap operation2;
    operation2["action"] = "move";
    operation2["id"] = 2;

    QVariantMap operation3;
    operation3["action"] = "delete";
    operation3["id"] = 3;

    operationsManager->SaveOperations(operation1);
    operationsManager->SaveOperations(operation2);
    operationsManager->SaveOperations(operation3);

    // Should return in LIFO order (Last In, First Out)
    QVariantMap result1 = operationsManager->RevocationOperations();
    EXPECT_EQ(result1["id"].toInt(), 3);

    QVariantMap result2 = operationsManager->RevocationOperations();
    EXPECT_EQ(result2["id"].toInt(), 2);

    QVariantMap result3 = operationsManager->RevocationOperations();
    EXPECT_EQ(result3["id"].toInt(), 1);

    // Stack should be empty now
    QVariantMap result4 = operationsManager->RevocationOperations();
    EXPECT_TRUE(result4.isEmpty());
}

TEST_F(TestOperationsStackManagerDbus, SaveOperations_ExceedsMaxSize_RemovesOldest)
{
    // Add 101 operations (exceeding the limit of 100)
    for (int i = 0; i < 101; ++i) {
        QVariantMap operation;
        operation["action"] = "test";
        operation["id"] = i;
        operationsManager->SaveOperations(operation);
    }

    // Should have exactly 100 operations, with the oldest (id=0) removed
    int count = 0;
    QVariantMap result;
    do {
        result = operationsManager->RevocationOperations();
        if (!result.isEmpty()) {
            count++;
            // The first operation retrieved should be id=100 (newest)
            if (count == 1) {
                EXPECT_EQ(result["id"].toInt(), 100);
            }
            // The last operation retrieved should be id=1 (oldest remaining)
            if (count == 100) {
                EXPECT_EQ(result["id"].toInt(), 1);
            }
        }
    } while (!result.isEmpty());

    EXPECT_EQ(count, 100);
}

TEST_F(TestOperationsStackManagerDbus, CleanOperations_EmptiesStack)
{
    QVariantMap operation1;
    operation1["action"] = "copy";
    operation1["id"] = 1;

    QVariantMap operation2;
    operation2["action"] = "move";
    operation2["id"] = 2;

    operationsManager->SaveOperations(operation1);
    operationsManager->SaveOperations(operation2);

    operationsManager->CleanOperations();

    QVariantMap result = operationsManager->RevocationOperations();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOperationsStackManagerDbus, RevocationOperations_EmptyStack_ReturnsEmpty)
{
    QVariantMap result = operationsManager->RevocationOperations();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOperationsStackManagerDbus, SaveRedoOperations_SingleOperation_Success)
{
    QVariantMap testOperation;
    testOperation["action"] = "redo_copy";
    testOperation["source"] = "/home/user/file1.txt";
    testOperation["target"] = "/home/user/backup/file1.txt";

    operationsManager->SaveRedoOperations(testOperation);

    QVariantMap result = operationsManager->RevocationRedoOperations();
    EXPECT_EQ(result, testOperation);
}

TEST_F(TestOperationsStackManagerDbus, SaveRedoOperations_MultipleOperations_LIFO_Order)
{
    QVariantMap operation1;
    operation1["action"] = "redo_copy";
    operation1["id"] = 1;

    QVariantMap operation2;
    operation2["action"] = "redo_move";
    operation2["id"] = 2;

    QVariantMap operation3;
    operation3["action"] = "redo_delete";
    operation3["id"] = 3;

    operationsManager->SaveRedoOperations(operation1);
    operationsManager->SaveRedoOperations(operation2);
    operationsManager->SaveRedoOperations(operation3);

    // Should return in LIFO order
    QVariantMap result1 = operationsManager->RevocationRedoOperations();
    EXPECT_EQ(result1["id"].toInt(), 3);

    QVariantMap result2 = operationsManager->RevocationRedoOperations();
    EXPECT_EQ(result2["id"].toInt(), 2);

    QVariantMap result3 = operationsManager->RevocationRedoOperations();
    EXPECT_EQ(result3["id"].toInt(), 1);

    // Stack should be empty now
    QVariantMap result4 = operationsManager->RevocationRedoOperations();
    EXPECT_TRUE(result4.isEmpty());
}

TEST_F(TestOperationsStackManagerDbus, SaveRedoOperations_ExceedsMaxSize_RemovesOldest)
{
    // Add 101 redo operations (exceeding the limit of 100)
    for (int i = 0; i < 101; ++i) {
        QVariantMap operation;
        operation["action"] = "redo_test";
        operation["id"] = i;
        operationsManager->SaveRedoOperations(operation);
    }

    // Should have exactly 100 operations, with the oldest (id=0) removed
    int count = 0;
    QVariantMap result;
    do {
        result = operationsManager->RevocationRedoOperations();
        if (!result.isEmpty()) {
            count++;
            // The first operation retrieved should be id=100 (newest)
            if (count == 1) {
                EXPECT_EQ(result["id"].toInt(), 100);
            }
            // The last operation retrieved should be id=1 (oldest remaining)
            if (count == 100) {
                EXPECT_EQ(result["id"].toInt(), 1);
            }
        }
    } while (!result.isEmpty());

    EXPECT_EQ(count, 100);
}

TEST_F(TestOperationsStackManagerDbus, RevocationRedoOperations_EmptyStack_ReturnsEmpty)
{
    QVariantMap result = operationsManager->RevocationRedoOperations();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestOperationsStackManagerDbus, CleanOperationsByUrl_NotImplemented_DoesNothing)
{
    // Add some operations first
    QVariantMap operation1;
    operation1["action"] = "copy";
    operation1["source"] = "/home/user/file1.txt";
    operation1["id"] = 1;

    QVariantMap operation2;
    operation2["action"] = "move";
    operation2["source"] = "/home/user/file2.txt";
    operation2["id"] = 2;

    operationsManager->SaveOperations(operation1);
    operationsManager->SaveOperations(operation2);

    QStringList urls;
    urls << "/home/user/file1.txt" << "/home/user/file2.txt";

    // Call the method (currently not implemented)
    operationsManager->CleanOperationsByUrl(urls);

    // Operations should still be there since method is not implemented
    QVariantMap result1 = operationsManager->RevocationOperations();
    EXPECT_EQ(result1["id"].toInt(), 2);

    QVariantMap result2 = operationsManager->RevocationOperations();
    EXPECT_EQ(result2["id"].toInt(), 1);
}

TEST_F(TestOperationsStackManagerDbus, MixedOperations_UndoAndRedo_IndependentStacks)
{
    // Add operations to both stacks
    QVariantMap undoOp1;
    undoOp1["action"] = "copy";
    undoOp1["id"] = 1;

    QVariantMap undoOp2;
    undoOp2["action"] = "move";
    undoOp2["id"] = 2;

    QVariantMap redoOp1;
    redoOp1["action"] = "redo_copy";
    redoOp1["id"] = 10;

    QVariantMap redoOp2;
    redoOp2["action"] = "redo_move";
    redoOp2["id"] = 20;

    operationsManager->SaveOperations(undoOp1);
    operationsManager->SaveRedoOperations(redoOp1);
    operationsManager->SaveOperations(undoOp2);
    operationsManager->SaveRedoOperations(redoOp2);

    // Verify undo stack
    QVariantMap undoResult1 = operationsManager->RevocationOperations();
    EXPECT_EQ(undoResult1["id"].toInt(), 2);

    QVariantMap undoResult2 = operationsManager->RevocationOperations();
    EXPECT_EQ(undoResult2["id"].toInt(), 1);

    // Verify redo stack
    QVariantMap redoResult1 = operationsManager->RevocationRedoOperations();
    EXPECT_EQ(redoResult1["id"].toInt(), 20);

    QVariantMap redoResult2 = operationsManager->RevocationRedoOperations();
    EXPECT_EQ(redoResult2["id"].toInt(), 10);

    // Both stacks should be empty now
    EXPECT_TRUE(operationsManager->RevocationOperations().isEmpty());
    EXPECT_TRUE(operationsManager->RevocationRedoOperations().isEmpty());
}

TEST_F(TestOperationsStackManagerDbus, ComplexOperationData_PreservesAllFields)
{
    QVariantMap complexOperation;
    complexOperation["action"] = "complex_operation";
    complexOperation["source"] = "/home/user/source/";
    complexOperation["target"] = "/home/user/target/";
    complexOperation["timestamp"] = 1642680000;
    complexOperation["size"] = 1024 * 1024;  // 1MB
    complexOperation["permissions"] = 755;
    
    QStringList fileList;
    fileList << "file1.txt" << "file2.txt" << "file3.txt";
    complexOperation["files"] = fileList;

    QVariantMap metadata;
    metadata["user"] = "testuser";
    metadata["application"] = "file-manager";
    complexOperation["metadata"] = metadata;

    operationsManager->SaveOperations(complexOperation);

    QVariantMap result = operationsManager->RevocationOperations();
    
    EXPECT_EQ(result["action"].toString(), "complex_operation");
    EXPECT_EQ(result["source"].toString(), "/home/user/source/");
    EXPECT_EQ(result["target"].toString(), "/home/user/target/");
    EXPECT_EQ(result["timestamp"].toInt(), 1642680000);
    EXPECT_EQ(result["size"].toInt(), 1024 * 1024);
    EXPECT_EQ(result["permissions"].toInt(), 755);
    EXPECT_EQ(result["files"].toStringList(), fileList);
    
    QVariantMap resultMetadata = result["metadata"].toMap();
    EXPECT_EQ(resultMetadata["user"].toString(), "testuser");
    EXPECT_EQ(resultMetadata["application"].toString(), "file-manager");
} 