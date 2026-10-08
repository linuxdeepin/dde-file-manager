// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfilehandler_pms.cpp
 * @brief PMS sev-2 regression tests for LocalFileHandler
 *        (src/dfm-base/file/local/localfilehandler.cpp).
 *
 * Bug -> case mapping (fix commits verified via `git show`):
 *   - PMS:337839 (16a96cb9) LocalFileHandlerPrivate::isExecutableScript:
 *     the symlink-chasing loop must bail out with `if (!info) return false`
 *     so a broken symlink no longer dereferences a null FileInfo (crash).
 *   - PMS:337879 (16a96cb9) same fix commit: symlink cycles / dead targets
 *     terminate via the targetList loop guard and return false.
 *   - PMS:315769 (1acbdbf5) LocalFileHandlerPrivate::isFileExecutable:
 *     executability is decided from the file's executable attributes
 *     (plus a suffix blacklist), not from hand-rolled permission bits.
 *   - PMS:138405 (f4fce72c) LocalFileHandlerPrivate::openExcutableScriptFile:
 *     script execution / terminal / open-with flows must receive the
 *     script's directory as working directory.
 */

#include <gtest/gtest.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/localfilehandler.h>
#include "dfm-base/file/local/localfilehandler_p.h"
#include <dfm-base/file/local/syncfileinfo.h>
#include "dfm-base/file/local/private/syncfileinfo_p.h"
#include <dfm-base/utils/applaunchutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/dfm_global_defines.h>

#include "stubext.h"

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QIcon>
#include <mutex>

using namespace dfmbase;

class UT_LocalFileHandlerPms : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();
    }

    void TearDown() override
    {
        stub.clear();
    }

    QString makeFile(const QString &name, const QByteArray &content, QFileDevice::Permissions perms)
    {
        const QString path = rootPath + "/" + name;
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly))
            return QString();
        f.write(content);
        f.close();
        if (!QFile::setPermissions(path, perms))
            return QString();
        return path;
    }

    stub_ext::StubExt stub;
    QTemporaryDir tmpDir;
    QString rootPath;
    static std::once_flag flag;
};

std::once_flag UT_LocalFileHandlerPms::flag;

// PMS:337839 桌面双击失效的软链接（指向不存在目标）时文件管理器崩溃：
// isExecutableScript 在软链接循环中必须对空 FileInfo 直接返回 false
TEST_F(UT_LocalFileHandlerPms, BUG337839_BrokenSymlinkReturnsFalseNoCrash)
{
    // Arrange — a symlink whose target does not exist.
    LocalFileHandler handler;
    const QString linkPath = rootPath + "/broken_337839.sh";
    ASSERT_TRUE(QFile::link(rootPath + "/definitely-missing-target-337839", linkPath));
    ASSERT_TRUE(QFileInfo(linkPath).isSymLink());

    // Act / Assert — no crash and a plain "not executable" verdict.
    bool result = true;
    EXPECT_NO_FATAL_FAILURE({ result = handler.d->isExecutableScript(linkPath); });
    EXPECT_FALSE(result);
}

// PMS:337879 软链接成环/目标层层失效时不得死循环或崩溃，最终返回不可执行
TEST_F(UT_LocalFileHandlerPms, BUG337879_SymlinkCycleTerminates)
{
    // Arrange — linkA -> linkB -> linkA (a cycle).
    LocalFileHandler handler;
    const QString linkA = rootPath + "/cycle_a_337879";
    const QString linkB = rootPath + "/cycle_b_337879";
    ASSERT_TRUE(QFile::link(linkB, linkA));
    ASSERT_TRUE(QFile::link(linkA, linkB));

    // Act / Assert — the targetList loop guard must break the cycle.
    bool result = true;
    EXPECT_NO_FATAL_FAILURE({ result = handler.d->isExecutableScript(linkA); });
    EXPECT_FALSE(result);
}

// PMS:315769 脚本可执行性按文件的可执行属性判断（含后缀黑名单）
// 注意：UT offscreen 环境拿不到 gio 的 access::can-execute 属性，
// 因此用 SyncFileInfoPrivate::isExecutable 的 stub 控制属性结果。
TEST_F(UT_LocalFileHandlerPms, BUG315769_ExecutableByAttributeNotPermissionBits)
{
    LocalFileHandler handler;

    // A shell script with owner-exec permission is executable.
    const QString runnable = makeFile("run_315769.sh", "#!/bin/bash\necho hi\n",
                                      QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    ASSERT_FALSE(runnable.isEmpty());

    // The same content without any exec permission is not executable.
    const QString plain = makeFile("plain_315769.sh", "#!/bin/bash\necho hi\n",
                                   QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    ASSERT_FALSE(plain.isEmpty());

    // Text suffixes are never regarded as executable, even with x bits.
    const QString textFile = makeFile("notes_315769.txt", "just text\n",
                                      QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    ASSERT_FALSE(textFile.isEmpty());

    // Drive the attribute layer: executable && readable => true.
    stub.set_lamda(&SyncFileInfoPrivate::isExecutable,
                   [](SyncFileInfoPrivate *) -> bool { return true; });
    stub.set_lamda(static_cast<QVariant (SyncFileInfoPrivate::*)(DFMIO::DFileInfo::AttributeID, bool *) const>(
                       &SyncFileInfoPrivate::attribute),
                   [](SyncFileInfoPrivate *, DFMIO::DFileInfo::AttributeID id, bool *ok) -> QVariant {
                       if (ok)
                           *ok = true;
                       return id == DFMIO::DFileInfo::AttributeID::kAccessCanRead;
                   });
    EXPECT_TRUE(handler.d->isFileExecutable(runnable));

    // Not executable => false even though readable.
    stub.set_lamda(&SyncFileInfoPrivate::isExecutable,
                   [](SyncFileInfoPrivate *) -> bool { return false; });
    EXPECT_FALSE(handler.d->isFileExecutable(plain));
    stub.clear();

    // Real attribute path: text suffixes are unexecutable (blacklist first),
    // even with x bits set.
    EXPECT_FALSE(handler.d->isFileExecutable(textFile));

    // Missing files are simply not executable.
    EXPECT_FALSE(handler.d->isFileExecutable(rootPath + "/ghost_315769.sh"));
}

// PMS:138405 脚本在固定目录下执行失败：执行脚本时必须以脚本所在目录为工作目录
TEST_F(UT_LocalFileHandlerPms, BUG138405_ScriptExecutedInItsOwnDirectory)
{
    // Arrange
    LocalFileHandler handler;
    QDir(rootPath).mkpath("dir138405");
    const QString scriptPath = rootPath + "/dir138405/tool.sh";
    QFile f(scriptPath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("#!/bin/bash\n");
    f.close();

    QString capturedProgram;
    QString capturedWorkdir;
    stub.set_lamda(static_cast<bool (AppLaunchUtils::*)(const QString &, const QStringList &,
                                                        const QString &, const QString &,
                                                        const QString &, const QStringMap &)>(
                       &AppLaunchUtils::executeCommand),
                   [&capturedProgram, &capturedWorkdir](AppLaunchUtils *, const QString &program,
                                                        const QStringList &, const QString &, const QString &workdir,
                                                        const QString &, const QStringMap &) -> bool {
                       capturedProgram = program;
                       capturedWorkdir = workdir;
                       return true;
                   });

    // Act — flag 1: run the script itself. executeCommand succeeds, so
    // the fallback QProcess::startDetached is never taken and the return
    // value stays false by contract.
    const bool ok = handler.d->openExcutableScriptFile(scriptPath, 1);

    // Assert — the working directory is the script's directory (with a
    // trailing separator), never the script file path itself.
    EXPECT_FALSE(ok);
    EXPECT_EQ(capturedProgram, scriptPath);
    const QString expectedWorkdir = QUrl(scriptPath).adjusted(QUrl::RemoveFilename).toString();
    EXPECT_EQ(capturedWorkdir, expectedWorkdir);
    EXPECT_TRUE(capturedWorkdir.endsWith('/'));
    EXPECT_FALSE(capturedWorkdir.contains(QStringLiteral("tool.sh")));
    EXPECT_TRUE(capturedWorkdir.contains(QStringLiteral("dir138405")));
}

// PMS:138405 flag=3（打开方式）必须走 doOpenFile 打开链路并传入脚本 URL
TEST_F(UT_LocalFileHandlerPms, BUG138405_OpenWithFlagGoesThroughDoOpenFile)
{
    // Arrange
    LocalFileHandler handler;
    const QString scriptPath = rootPath + "/openwith_138405.sh";
    QFile f(scriptPath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("#!/bin/bash\n");
    f.close();

    QUrl capturedUrl;
    stub.set_lamda(&LocalFileHandlerPrivate::doOpenFile,
                   [&capturedUrl](LocalFileHandlerPrivate *, const QUrl &url, const QString &) -> bool {
                       capturedUrl = url;
                       return true;
                   });

    // Act
    const bool ok = handler.d->openExcutableScriptFile(scriptPath, 3);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedUrl, QUrl::fromLocalFile(scriptPath));
}

// PMS:138405 flag=0 是无操作分支：不弹对话框、不执行、返回 false
TEST_F(UT_LocalFileHandlerPms, BUG138405_ZeroFlagIsNoop)
{
    // Arrange
    LocalFileHandler handler;
    const QString scriptPath = rootPath + "/noop_138405.sh";
    QFile f(scriptPath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("#!/bin/bash\n");
    f.close();

    bool executeCalled = false;
    stub.set_lamda(static_cast<bool (AppLaunchUtils::*)(const QString &, const QStringList &,
                                                        const QString &, const QString &,
                                                        const QString &, const QStringMap &)>(
                       &AppLaunchUtils::executeCommand),
                   [&executeCalled](AppLaunchUtils *, const QString &, const QStringList &,
                                    const QString &, const QString &, const QString &, const QStringMap &) -> bool {
                       executeCalled = true;
                       return true;
                   });

    // Act / Assert
    bool result = true;
    EXPECT_NO_FATAL_FAILURE({ result = handler.d->openExcutableScriptFile(scriptPath, 0); });
    EXPECT_FALSE(result);
    EXPECT_FALSE(executeCalled);
}

// PMS:293935 双击循环软链卡死：resolveSymlink 以 visitedPaths 检测环形链接，
// 命中环路立即终止解析（不再死循环），静默走正常打开流程；悬空软链则弹
// showBreakSymlinkDialog 提示且其返回值决定 openFiles 的返回（kUnknowType
// 视为用户已处理）。两种情况都必须快速返回，不挂起、不崩溃
TEST_F(UT_LocalFileHandlerPms, BUG293935_OpenSymlinkLoopTerminatesWithoutHang)
{
    stub_ext::StubExt stub;
    LocalFileHandler handler;

    // 循环软链：a -> b -> a（ELOOP，canonicalFilePath 为空）
    const QString linkA = tmpDir.path() + "/ut_loop_a";
    const QString linkB = tmpDir.path() + "/ut_loop_b";
    QFile::remove(linkA);
    QFile::remove(linkB);
    ASSERT_TRUE(QFile::link(linkA, linkB));
    ASSERT_TRUE(QFile::link(linkB, linkA));

    // 悬空软链：目标不存在
    const QString dangling = tmpDir.path() + "/ut_dangling_293935";
    QFile::remove(dangling);
    ASSERT_TRUE(QFile::link(tmpDir.path() + "/ut_no_such_target", dangling));

    int dialogCount = 0;
    int breakDialogCount = 0;
    stub.set_lamda(ADDR(DialogManager, showErrorDialog),
                   [&](DialogManager *, const QString &, const QString &) {
                       __DBG_STUB_INVOKE__
                       ++dialogCount;
                   });
    stub.set_lamda(ADDR(DialogManager, showUnableToVistDir),
                   [&](DialogManager *, const QString &) -> int {
                       __DBG_STUB_INVOKE__
                       ++breakDialogCount;
                       return 0;
                   });
    // 断链对话框会创建真实模态 DDialog（UT offscreen 无窗口管理器会阻塞），打桩：
    // 返回 kUnknowType 模拟用户直接关闭，契约上 openFiles 应视为已处理并返回 true
    stub.set_lamda(ADDR(DialogManager, showBreakSymlinkDialog),
                   [&](DialogManager *, const QString &, const QUrl &) -> GlobalEventType {
                       __DBG_STUB_INVOKE__
                       ++breakDialogCount;
                       return GlobalEventType::kUnknowType;
                   });
    stub.set_lamda(ADDR(NetworkUtils, checkFtpOrSmbBusy),
                   [](NetworkUtils *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    // 回归核心：环形链接解析必须终止（此前实现会无限跟随 a->b->a 卡死文件管理器）
    bool result = handler.openFiles({ QUrl::fromLocalFile(linkA) });
    EXPECT_EQ(dialogCount + breakDialogCount, 0);
    EXPECT_FALSE(result);

    // 悬空软链：弹出一次断链提示，kUnknowType 返回值 => openFiles 返回 true
    dialogCount = 0;
    breakDialogCount = 0;
    result = handler.openFiles({ QUrl::fromLocalFile(dangling) });
    EXPECT_EQ(dialogCount, 0);
    EXPECT_EQ(breakDialogCount, 1);
    EXPECT_TRUE(result);

    // 清理，避免影响其他用例的目录遍历
    QFile::remove(linkA);
    QFile::remove(linkB);
    QFile::remove(dangling);
}

// PMS:170333 trash/无效 URI 上取 MIME 崩溃：getFileMimetype 迁移为 URI 制后
// 对本地文件返回真实 MIME；对 trash 上不存在的条目与空 URI 必须返回空串，
// 不得通过 g_file_new_for_path 混用路径/URI 而崩溃
TEST_F(UT_LocalFileHandlerPms, BUG170333_GetFileMimetypeHandlesInvalidUris)
{
    LocalFileHandler handler;
    // 本地真实文件：text/plain
    const QString real = tmpDir.path() + "/ut_170333.txt";
    {
        QFile f(real);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("ut");
    }
    EXPECT_EQ(handler.d->getFileMimetype(QUrl::fromLocalFile(real)),
              QStringLiteral("text/plain"));

    // trash 上不存在的条目：返回空串，不崩溃
    EXPECT_EQ(handler.d->getFileMimetype(QUrl(QStringLiteral("trash:///ut_nonexistent_170333"))),
              QString());

    // 空 URI：返回空串，不崩溃
    EXPECT_EQ(handler.d->getFileMimetype(QUrl()), QString());
}
