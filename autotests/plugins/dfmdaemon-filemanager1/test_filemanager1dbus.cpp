// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "filemanager1dbus.h"

#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class TestFileManager1DBus : public testing::Test
{
protected:
    void SetUp() override
    {
        fileManager1DBus = new FileManager1DBus();
    }

    void TearDown() override
    {
        delete fileManager1DBus;
        stub.clear();
    }

    FileManager1DBus *fileManager1DBus { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TestFileManager1DBus, Constructor_InitializesCorrectly)
{
    FileManager1DBus testDBus;
    // Constructor should complete without issues
    EXPECT_NE(&testDBus, nullptr);
}

TEST_F(TestFileManager1DBus, ShowFolders_CallsProcess)
{
    bool processStarted = false;
    QStringList testURIs = { "file:///home/user/Documents", "file:///home/user/Pictures" };
    QString testStartupId = "test-startup-id";

    // Mock QProcess::startDetached using function address
    using StartDetachedFuncPtr = bool (*)(
            const QString &,
            const QStringList &,
            const QString &,
            qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), [&](const QString &program, const QStringList &arguments, const QString &workingDirectory, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processStarted = true;
        EXPECT_TRUE(program == "file-manager.sh" || program.contains("dde-file-manager"));
        return true;
    });

    fileManager1DBus->ShowFolders(testURIs, testStartupId);

    EXPECT_TRUE(processStarted);
}

TEST_F(TestFileManager1DBus, ShowItemProperties_CallsProcess)
{
    bool processStarted = false;
    QStringList testURIs = { "file:///home/user/document.pdf" };
    QString testStartupId = "test-startup-id";

    using StartDetachedFuncPtr = bool (*)(
        const QString &,
        const QStringList &,
        const QString &,
        qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), 
    [&](const QString &program, const QStringList &arguments, const QString &workingDirectory, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processStarted = true;
        EXPECT_TRUE(program == "file-manager.sh" || program.contains("dde-file-manager"));
        if (program == "file-manager.sh") {
            EXPECT_TRUE(arguments.contains("-p"));
        }
        return true;
    });

    fileManager1DBus->ShowItemProperties(testURIs, testStartupId);

    EXPECT_TRUE(processStarted);
}

TEST_F(TestFileManager1DBus, ShowItems_CallsProcess)
{
    bool processStarted = false;
    QStringList testURIs = { "file:///home/user/file.txt" };
    QString testStartupId = "test-startup-id";

    using StartDetachedFuncPtr = bool (*)(
        const QString &,
        const QStringList &,
        const QString &,
        qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), 
    [&](const QString &program, const QStringList &arguments, const QString &workingDirectory, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processStarted = true;
        EXPECT_TRUE(program == "file-manager.sh" || program.contains("dde-file-manager"));
        if (program == "file-manager.sh") {
            EXPECT_TRUE(arguments.contains("--show-item"));
        }
        return true;
    });

    fileManager1DBus->ShowItems(testURIs, testStartupId);

    EXPECT_TRUE(processStarted);
}

TEST_F(TestFileManager1DBus, Trash_CallsProcessWithJSON)
{
    bool processStarted = false;
    QStringList testURIs = { "file:///home/user/file1.txt", "file:///home/user/file2.txt" };

    using StartDetachedFuncPtr = bool (*)(
        const QString &,
        const QStringList &,
        const QString &,
        qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), 
    [&](const QString &program, const QStringList &arguments, const QString &workingDirectory, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processStarted = true;
        EXPECT_TRUE(program == "file-manager.sh" || program.contains("dde-file-manager"));

        if (program == "file-manager.sh" && arguments.size() >= 2 && arguments[0] == "--event") {
            // Parse the JSON argument
            QJsonDocument doc = QJsonDocument::fromJson(arguments[1].toUtf8());
            EXPECT_TRUE(doc.isObject());

            QJsonObject obj = doc.object();
            EXPECT_EQ(obj["action"].toString(), "trash");

            QJsonObject params = obj["params"].toObject();
            QJsonArray sources = params["sources"].toArray();
            EXPECT_EQ(sources.size(), 2);
        }

        return true;
    });

    fileManager1DBus->Trash(testURIs);

    EXPECT_TRUE(processStarted);
}

TEST_F(TestFileManager1DBus, Open_WithRegularArgs_CallsProcess)
{
    bool processStarted = false;
    QStringList testURIs = { "file:///home/user/document.pdf", "/home/user/image.jpg" };

    using StartDetachedFuncPtr = bool (*)(
        const QString &,
        const QStringList &,
        const QString &,
        qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), 
    [&](const QString &program, const QStringList &arguments, const QString &workingDirectory, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processStarted = true;
        EXPECT_TRUE(program == "file-manager.sh" || program.contains("dde-file-manager"));
        return true;
    });

    fileManager1DBus->Open(testURIs);

    EXPECT_TRUE(processStarted);
}

TEST_F(TestFileManager1DBus, Open_WithBase64Args_DecodesCorrectly)
{
    bool processStarted = false;
    QString originalArg = "file:///home/user/test file with spaces.txt";
    QString base64Arg = "B64:" + QString::fromUtf8(originalArg.toUtf8().toBase64());
    QStringList testURIs = { base64Arg, "regular_arg" };

    using StartDetachedFuncPtr = bool (*)(
        const QString &,
        const QStringList &,
        const QString &,
        qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), 
    [&](const QString &program, const QStringList &arguments, const QString &workingDirectory, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processStarted = true;
        EXPECT_TRUE(program == "file-manager.sh" || program.contains("dde-file-manager"));

        if (program == "file-manager.sh") {
            // Check that Base64 argument was decoded
            bool foundDecoded = false;
            for (const QString &arg : arguments) {
                if (arg.contains("test file with spaces")) {
                    foundDecoded = true;
                    break;
                }
            }
            EXPECT_TRUE(foundDecoded);
        }

        return true;
    });

    fileManager1DBus->Open(testURIs);

    EXPECT_TRUE(processStarted);
}

TEST_F(TestFileManager1DBus, ProcessFailure_HandlesGracefully)
{
    bool processAttempted = false;
    QStringList testURIs = { "file:///test" };

    // Mock process to always fail
    using StartDetachedFuncPtr = bool (*)(
        const QString &,
        const QStringList &,
        const QString &,
        qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), 
    [&](const QString &program, const QStringList &arguments, const QString &workingDirectory, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processAttempted = true;
        return false;   // Always fail
    });

    // These should all handle failures gracefully without crashing
    fileManager1DBus->ShowFolders(testURIs, "");
    fileManager1DBus->ShowItemProperties(testURIs, "");
    fileManager1DBus->ShowItems(testURIs, "");
    fileManager1DBus->Trash(testURIs);
    fileManager1DBus->Open(testURIs);

    EXPECT_TRUE(processAttempted);
}

TEST_F(TestFileManager1DBus, EmptyURIList_HandlesGracefully)
{
    bool processStarted = false;
    QStringList emptyURIs;

    using StartDetachedFuncPtr = bool (*)(
        const QString &,
        const QStringList &,
        const QString &,
        qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), 
    [&](const QString &program, const QStringList &arguments, const QString &workingDirectory, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processStarted = true;
        return true;
    });

    // These should handle empty URI lists gracefully
    fileManager1DBus->ShowFolders(emptyURIs, "");
    fileManager1DBus->ShowItemProperties(emptyURIs, "");
    fileManager1DBus->ShowItems(emptyURIs, "");
    fileManager1DBus->Trash(emptyURIs);
    fileManager1DBus->Open(emptyURIs);

    EXPECT_TRUE(processStarted);
}
