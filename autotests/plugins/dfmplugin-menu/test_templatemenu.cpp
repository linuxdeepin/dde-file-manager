// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "templatemenuscene/templatemenu.h"
#include "templatemenuscene/private/templatemenu_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <dfm-io/dfmio_utils.h>

#include <DDesktopEntry>

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

static void writeTextFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        file.write(content.toLocal8Bit());
}

class UT_TemplateMenu : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        menu = new TemplateMenu();
        d = menu->d.data();
    }

    virtual void TearDown() override
    {
        qDeleteAll(menu->actionList());
        delete menu;
        menu = nullptr;
        d = nullptr;
        stub.clear();
    }

    // 将模板目录探测全部指向受控的临时目录
    void redirectTemplateDirs(const QString &userTemplates,
                              const QStringList &systemData = QStringList(),
                              const QString &userData = QString())
    {
        stub.set_lamda(&DFMIO::DFMUtils::userSpecialDir, [userTemplates](DFMIO::DGlibUserDirectory dir) -> QString {
            __DBG_STUB_INVOKE__
            if (dir == DFMIO::DGlibUserDirectory::kUserDirectoryTemplates)
                return userTemplates;
            return QString();
        });
        stub.set_lamda(&DFMIO::DFMUtils::systemDataDirs, [systemData]() -> QStringList {
            __DBG_STUB_INVOKE__
            return systemData;
        });
        stub.set_lamda(&DFMIO::DFMUtils::userDataDir, [userData]() -> QString {
            __DBG_STUB_INVOKE__
            return userData;
        });
    }

    // 用纯字符串实现的 FileInfo 替代真实 InfoFactory，保证确定性
    void stubInfoFactory()
    {
        stub.set_lamda(&InfoFactory::create<FileInfo>,
                       [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                           __DBG_STUB_INVOKE__
                           return QSharedPointer<FileInfo>(new FileInfo(url));
                       });
    }

protected:
    TemplateMenu *menu { nullptr };
    TemplateMenuPrivate *d { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_TemplateMenu, Constructor_InitializesCorrectly)
{
    EXPECT_NE(menu, nullptr);
    EXPECT_TRUE(menu->actionList().isEmpty());
}

TEST_F(UT_TemplateMenu, ActionList_ReturnsActionList)
{
    // 未加载模板文件时，actionList 确定为空
    auto actions = menu->actionList();
    EXPECT_TRUE(actions.isEmpty());
}

TEST_F(UT_TemplateMenu, LoadTemplateFile_UserTemplateDir_CreatesActionsForNormalFiles)
{
    QTemporaryDir userDir;
    ASSERT_TRUE(userDir.isValid());
    writeTextFile(userDir.filePath("textfile.txt"), "template content");
    writeTextFile(userDir.filePath("empty.md"), "");

    redirectTemplateDirs(userDir.path());
    stubInfoFactory();

    menu->loadTemplateFile();

    auto actions = menu->actionList();
    ASSERT_EQ(actions.size(), 2);
    EXPECT_TRUE(actions.at(0)->text() == "textfile" || actions.at(1)->text() == "textfile");
    for (const QAction *act : actions) {
        if (act->text() == "textfile")
            EXPECT_EQ(act->data().toString(), userDir.filePath("textfile.txt"));
    }
}

TEST_F(UT_TemplateMenu, LoadTemplateFile_SystemDataDir_CreatesActionsForDesktopEntriesOnly)
{
    QTemporaryDir userDir;
    QTemporaryDir systemDir;
    ASSERT_TRUE(userDir.isValid());
    ASSERT_TRUE(systemDir.isValid());

    // systemDataDir 下仅识别 <dir>/templates/*.desktop
    QDir templatesDir(systemDir.path() + "/templates");
    ASSERT_TRUE(templatesDir.mkpath("."));

    const QString targetFile = templatesDir.filePath("blank.doc");
    writeTextFile(targetFile, "blank document");
    writeTextFile(templatesDir.filePath("ignored.txt"), "not a desktop entry");

    const QString desktopContent = QString("[Desktop Entry]\n"
                                           "Type=Application\n"
                                           "Name=Blank Doc\n"
                                           "Icon=document-new\n"
                                           "URL=%1\n")
                                           .arg(targetFile);
    writeTextFile(templatesDir.filePath("blank.desktop"), desktopContent);

    redirectTemplateDirs("", { systemDir.path() });
    stubInfoFactory();

    menu->loadTemplateFile();

    auto actions = menu->actionList();
    ASSERT_EQ(actions.size(), 1);
    EXPECT_EQ(actions.at(0)->text(), "Blank Doc");
    EXPECT_EQ(actions.at(0)->data().toString(), targetFile);
}

TEST_F(UT_TemplateMenu, LoadTemplateFile_DeduplicatesSameFileName_AcrossDirs)
{
    QTemporaryDir userDir;
    QTemporaryDir systemDir;
    ASSERT_TRUE(userDir.isValid());
    ASSERT_TRUE(systemDir.isValid());

    // 用户模板目录中的 doc.txt 先被处理
    writeTextFile(userDir.filePath("doc.txt"), "user template");

    QDir templatesDir(systemDir.path() + "/templates");
    ASSERT_TRUE(templatesDir.mkpath("."));

    // desktop 项 URL 指向另一个同名文件 doc.txt，应被去重
    const QString duplicatedFile = templatesDir.filePath("doc.txt");
    writeTextFile(duplicatedFile, "dup");
    const QString desktopContent = QString("[Desktop Entry]\n"
                                           "Type=Application\n"
                                           "Name=Duplicated\n"
                                           "URL=%1\n")
                                           .arg(duplicatedFile);
    writeTextFile(templatesDir.filePath("dup.desktop"), desktopContent);

    redirectTemplateDirs(userDir.path(), { systemDir.path() });
    stubInfoFactory();

    menu->loadTemplateFile();

    auto actions = menu->actionList();
    ASSERT_EQ(actions.size(), 1);
    EXPECT_EQ(actions.at(0)->data().toString(), userDir.filePath("doc.txt"));
}

TEST_F(UT_TemplateMenu, LoadTemplateFile_EmptyDirs_NoActions)
{
    QTemporaryDir userDir;
    QTemporaryDir systemDir;
    ASSERT_TRUE(userDir.isValid());
    ASSERT_TRUE(systemDir.isValid());

    redirectTemplateDirs(userDir.path(), { systemDir.path() });

    menu->loadTemplateFile();
    EXPECT_TRUE(menu->actionList().isEmpty());
}

TEST_F(UT_TemplateMenu, CreateActionByNormalFile_CreatesActionWithCompleteBaseName)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString filePath = dir.filePath("hello-world.txt");
    writeTextFile(filePath, "hello");

    stubInfoFactory();

    d->createActionByNormalFile(filePath);

    ASSERT_EQ(d->templateActions.size(), 1);
    EXPECT_EQ(d->templateActions.at(0)->text(), "hello-world");
    EXPECT_EQ(d->templateActions.at(0)->data().toString(), filePath);
}

TEST_F(UT_TemplateMenu, CreateActionByNormalFile_EmptyPath_NoAction)
{
    d->createActionByNormalFile(QString());
    EXPECT_TRUE(d->templateActions.isEmpty());
}

TEST_F(UT_TemplateMenu, CreateActionByNormalFile_NullFileInfo_NoAction)
{
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    d->createActionByNormalFile("/tmp/whatever-file.txt");
    EXPECT_TRUE(d->templateActions.isEmpty());
}

TEST_F(UT_TemplateMenu, CreateActionByNormalFile_DuplicateFileName_Skipped)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString filePath = dir.filePath("same-name.txt");
    writeTextFile(filePath, "content");

    stubInfoFactory();

    d->createActionByNormalFile(filePath);
    d->createActionByNormalFile(filePath);

    EXPECT_EQ(d->templateActions.size(), 1);
}

TEST_F(UT_TemplateMenu, CreateActionByDesktopFile_CreatesActionFromEntry)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QDir templateDir(dir.path());

    const QString targetFile = templateDir.filePath("sheet.xls");
    writeTextFile(targetFile, "xlsx");

    const QString desktopPath = templateDir.filePath("sheet.desktop");
    const QString desktopContent = QString("[Desktop Entry]\n"
                                           "Type=Application\n"
                                           "Name=Sheet Template\n"
                                           "Icon=spreadsheet\n"
                                           "URL=%1\n")
                                           .arg(targetFile);
    writeTextFile(desktopPath, desktopContent);

    stubInfoFactory();

    d->createActionByDesktopFile(templateDir, desktopPath);

    ASSERT_EQ(d->templateActions.size(), 1);
    EXPECT_EQ(d->templateActions.at(0)->text(), "Sheet Template");
    EXPECT_EQ(d->templateActions.at(0)->data().toString(), targetFile);
}

TEST_F(UT_TemplateMenu, CreateActionByDesktopFile_InvalidArgs_NoAction)
{
    // 空路径
    d->createActionByDesktopFile(QDir("/tmp"), QString());
    EXPECT_TRUE(d->templateActions.isEmpty());

    // 目录不存在
    d->createActionByDesktopFile(QDir("/tmp/ut-not-exist-dir-xyz"), "/tmp/ut-not-exist-dir-xyz/a.desktop");
    EXPECT_TRUE(d->templateActions.isEmpty());
}

TEST_F(UT_TemplateMenu, CreateActionByDesktopFile_NullFileInfo_NoAction)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QDir templateDir(dir.path());

    const QString desktopPath = templateDir.filePath("broken.desktop");
    writeTextFile(desktopPath,
                  QString("[Desktop Entry]\n"
                          "Type=Application\n"
                          "Name=Broken\n"
                          "URL=/tmp/ut-nonexistent-target.doc\n"));

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    d->createActionByDesktopFile(templateDir, desktopPath);
    EXPECT_TRUE(d->templateActions.isEmpty());
}

TEST_F(UT_TemplateMenu, TraverseFolderToCreateActions_EmptyPath_NoAction)
{
    d->traverseFolderToCreateActions(QString(), false);
    EXPECT_TRUE(d->templateActions.isEmpty());
}

TEST_F(UT_TemplateMenu, TraverseFolderToCreateActions_NonExistentFolder_NoAction)
{
    d->traverseFolderToCreateActions("/tmp/ut-template-not-exist-xyz", false);
    EXPECT_TRUE(d->templateActions.isEmpty());
}

TEST_F(UT_TemplateMenu, TraverseFolderToCreateActions_OnlyTopLevelFilesAreLoaded)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    writeTextFile(dir.filePath("top-level.txt"), "top");
    ASSERT_TRUE(QDir(dir.filePath("nested")).mkpath("."));
    writeTextFile(dir.filePath("nested/nested-file.txt"), "nested");

    stubInfoFactory();

    d->traverseFolderToCreateActions(dir.path(), false);

    // 子目录中的文件不会被递归收集
    ASSERT_EQ(d->templateActions.size(), 1);
    EXPECT_EQ(d->templateActions.at(0)->text(), "top-level");
}

TEST_F(UT_TemplateMenu, TraverseFolderToCreateActions_DesktopEntryFolder_FiltersNonDesktopFiles)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QDir templateDir(dir.path() + "/templates");
    ASSERT_TRUE(templateDir.mkpath("."));

    const QString targetFile = templateDir.filePath("drawing.odg");
    writeTextFile(targetFile, "odg");
    writeTextFile(templateDir.filePath("plain.txt"), "should be ignored");
    const QString desktopContent = QString("[Desktop Entry]\n"
                                           "Type=Application\n"
                                           "Name=Drawing\n"
                                           "URL=%1\n")
                                           .arg(targetFile);
    writeTextFile(templateDir.filePath("drawing.desktop"), desktopContent);

    stubInfoFactory();

    d->traverseFolderToCreateActions(templateDir.path(), true);

    ASSERT_EQ(d->templateActions.size(), 1);
    EXPECT_EQ(d->templateActions.at(0)->text(), "Drawing");
}
