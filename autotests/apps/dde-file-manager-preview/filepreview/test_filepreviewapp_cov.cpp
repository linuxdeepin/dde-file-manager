// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "previewsingleapplication.h"
#include "previewlibrary.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

// counters exported by the fake libdfm-preview.so (also linked into the test)
extern "C" {
int covFakeInitCount();
int covFakeShowCount();
quint64 covFakeLastWinId();
}

class PreviewAppTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
        savedCwd = QDir::currentPath();
        // PreviewLibrary::load() dlopen()s a *copied* .so (different inode),
        // so the in-process counters above cannot observe those calls; the
        // fake library appends every call to this log file instead
        logPath = dir->filePath("fake-log.txt");
        QFile::remove(logPath);
        savedLogEnv = qEnvironmentVariableIsSet("COV_FAKE_LOG") ? qgetenv("COV_FAKE_LOG") : QByteArray();
        logEnvWasSet = qEnvironmentVariableIsSet("COV_FAKE_LOG");
        qputenv("COV_FAKE_LOG", logPath.toUtf8());
    }

    void TearDown() override
    {
        if (logEnvWasSet)
            qputenv("COV_FAKE_LOG", savedLogEnv);
        else
            qunsetenv("COV_FAKE_LOG");
        QDir::setCurrent(savedCwd);
        dir.reset();
        // closeServer() deletes the QLocalServer member while
        // setSingleInstance() dereferences it unconditionally: keep a live
        // server object around for the tests that follow
        auto *app = qobject_cast<PreviewSingleApplication *>(qApp);
        if (app && !app->localServer)
            app->localServer = new QLocalServer(app);
    }

    QStringList fakeLogLines() const
    {
        QFile f(logPath);
        if (!f.open(QIODevice::ReadOnly))
            return {};
        return QString::fromUtf8(f.readAll()).split('\n', Qt::SkipEmptyParts);
    }

    // build <dir>/work + <dir>/libdfm-preview/libdfm-preview.so and chdir to
    // <dir>/work so PreviewLibrary::load() finds the fake library
    void enterFakeLibSandbox()
    {
        const QString work = dir->filePath("work");
        const QString libDir = dir->filePath("libdfm-preview");
        ASSERT_TRUE(QDir().mkpath(work));
        ASSERT_TRUE(QDir().mkpath(libDir));
        const QString target = libDir + "/libdfm-preview.so";
        ASSERT_TRUE(QFile::copy(QString(UT_FAKE_DFM_PREVIEW_SO), target)) << target.toStdString();
        QDir::setCurrent(work);
    }

    // write the JSON payload processArgs() consumes
    QString writeArgsJson(const QString &windowId, const QStringList &selectUrls)
    {
        QJsonObject root;
        root.insert("windowId", windowId);
        root.insert("selectUrls", QJsonArray::fromStringList(selectUrls));
        root.insert("dirUrls", QJsonArray());
        const QString path = dir->filePath("args.json");
        QFile f(path);
        EXPECT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(QJsonDocument(root).toJson());
        f.close();
        return path;
    }

    std::unique_ptr<QTemporaryDir> dir;
    QString savedCwd;
    QString logPath;
    QByteArray savedLogEnv;
    bool logEnvWasSet = false;
};

// ---------------------------------------------------------------------------
// PreviewLibrary
// ---------------------------------------------------------------------------
TEST_F(PreviewAppTest, LibraryLoad_WithoutLibrary_Fails)
{
    // Arrange: cwd has no sibling libdfm-preview directory
    const QString work = dir->filePath("emptywork");
    ASSERT_TRUE(QDir().mkpath(work));
    QDir::setCurrent(work);

    // Act
    PreviewLibrary lib;
    const bool ok = lib.load();

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(lib.previewLib, nullptr);
}

TEST_F(PreviewAppTest, LibraryLoad_WithFakeLibrary_ResolvesAndCallsInit)
{
    // Arrange
    enterFakeLibSandbox();
    PreviewLibrary lib;

    // Act
    const bool ok = lib.load();

    // Assert
    EXPECT_TRUE(ok);
    ASSERT_NE(lib.previewLib, nullptr);
    EXPECT_NE(lib.initFunc, nullptr);
    EXPECT_NE(lib.showFunc, nullptr);
    // the init entry point ran inside the loaded library (observed through
    // the log file: the dlopen'd copy has its own globals)
    EXPECT_TRUE(fakeLogLines().contains(QString("init 0")));

    // Act: showPreview forwards to the resolved function
    lib.showPreview(321, { QUrl::fromLocalFile("/tmp/x") }, {});

    // Assert
    EXPECT_TRUE(fakeLogLines().contains(QString("show 321")));

    // Act: loading twice is a no-op, unload clears the handle
    EXPECT_TRUE(lib.load());
    lib.unload();

    // Assert
    EXPECT_EQ(lib.previewLib, nullptr);
    EXPECT_EQ(lib.showFunc, nullptr);

    // Act: after unload the library can be loaded again
    EXPECT_TRUE(lib.load());
    lib.unload();
}

// ---------------------------------------------------------------------------
// PreviewSingleApplication
// ---------------------------------------------------------------------------
TEST_F(PreviewAppTest, UserServerName_BuiltFromRuntimeLocationAndKey)
{
    // Arrange
    const QString runtime = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);

    // Act
    const QString name = PreviewSingleApplication::userServerName("cov-server");

    // Assert
    EXPECT_TRUE(name.startsWith(runtime));
    EXPECT_TRUE(name.endsWith(QString("cov-server")));
}

TEST_F(PreviewAppTest, SetSingleInstance_ListensOnUserServer)
{
    // Arrange: the app object from main() is the single instance
    auto *app = qobject_cast<PreviewSingleApplication *>(qApp);
    ASSERT_NE(app, nullptr);

    // Act
    const bool listening = app->setSingleInstance("cov-single");

    // Assert
    EXPECT_TRUE(listening);
    EXPECT_EQ(app->localServer->serverName(),
              PreviewSingleApplication::userServerName("cov-single"));

    // Cleanup: a QLocalServer can only listen once, free it for the tests
    // that follow (handleNewClient / closeServer)
    EXPECT_NO_FATAL_FAILURE(app->closeServer());
}

TEST_F(PreviewAppTest, ProcessArgs_MalformedInputs_ReturnEarly)
{
    // Arrange / Act / Assert: too few arguments
    EXPECT_NO_FATAL_FAILURE(PreviewSingleApplication::processArgs({ "only-one" }));

    // missing temp file
    EXPECT_NO_FATAL_FAILURE(PreviewSingleApplication::processArgs(
            { "arg0", dir->filePath("does-not-exist.json") }));

    // invalid json payload
    const QString bad = dir->filePath("bad.json");
    QFile f(bad);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("not json at all");
    f.close();
    EXPECT_NO_FATAL_FAILURE(PreviewSingleApplication::processArgs({ "arg0", bad }));
}

TEST_F(PreviewAppTest, ProcessArgs_ValidJson_LoadsLibraryAndShowsPreview)
{
    // Arrange: fake library reachable from the current directory
    enterFakeLibSandbox();
    const QString json = writeArgsJson("4242", { "file:///tmp/first", "file:///tmp/second" });

    // Act
    PreviewSingleApplication::processArgs({ "arg0", json });

    // Assert: the preview library was loaded and the show entry point ran
    // with the window id from the payload (observed via the log file, the
    // dlopen'd library copy owns its own globals)
    EXPECT_TRUE(fakeLogLines().contains(QString("show 4242")));
    // the temporary payload file is removed after processing
    EXPECT_FALSE(QFile::exists(json));
}

TEST_F(PreviewAppTest, HandleNewClient_RoundTripThroughOwnServer)
{
    // Arrange: the app listens and ships its own arguments to itself
    auto *app = qobject_cast<PreviewSingleApplication *>(qApp);
    ASSERT_NE(app, nullptr);
    ASSERT_TRUE(app->setSingleInstance("cov-roundtrip"));
    QSignalSpy connectionSpy(app->localServer, &QLocalServer::newConnection);
    const int showCallsBefore = fakeLogLines().filter(QRegularExpression("^show ")).size();

    // Act
    app->handleNewClient("cov-roundtrip");
    QElapsedTimer timer;
    timer.start();
    while (connectionSpy.count() < 1 && timer.elapsed() < 5000)
        qApp->processEvents(QEventLoop::AllEvents, 20);
    QTest::qWait(50);   // let the readyRead -> readData -> processArgs chain run

    // Assert: the server accepted the client and processed the payload; the
    // binary's own arguments carry no json file so no preview is triggered
    EXPECT_GE(connectionSpy.count(), 1);
    EXPECT_EQ(fakeLogLines().filter(QRegularExpression("^show ")).size(), showCallsBefore);
}

TEST_F(PreviewAppTest, CloseServer_TearsDownAndFurtherClientCallsAreSafe)
{
    // Arrange: must run after the tests that used the server
    auto *app = qobject_cast<PreviewSingleApplication *>(qApp);
    ASSERT_NE(app, nullptr);
    ASSERT_NE(app->localServer, nullptr);

    // Act
    EXPECT_NO_FATAL_FAILURE(app->closeServer());

    // Assert
    EXPECT_EQ(app->localServer, nullptr);

    // Act: a second close is a no-op, a client without server fails safely
    EXPECT_NO_FATAL_FAILURE(app->closeServer());
    EXPECT_NO_FATAL_FAILURE(app->handleNewClient("cov-noserver"));
    qApp->processEvents();
}
