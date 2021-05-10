/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include "dfmevent.h"

#include "testhelper.h"
#include "stubext.h"

#define private public
#include "tag/tagmanager.h"
#include "controllers/tagcontroller.cpp"

#define TAG_FILE_PATH_STR "test_tag_"
#define TAG_NAME_A "a"
#define TAG_NAME_B "b"
#define TAG_TXT_FILE "tag_test.txt";
#define NEW_TAG_TXT_FILE "new_tag_test.txt"; 

using namespace stub_ext;

namespace  {
class TestTagController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestTagController" << std::endl;

        m_pController = new TagController();
        m_pManager = new TagManager();

        tempDirPath_A = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + TAG_FILE_PATH_STR + TAG_NAME_A;
        tempDirPath_B = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + TAG_FILE_PATH_STR + TAG_NAME_B;
        tempTxtFilePath =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first()  + "/" + TAG_TXT_FILE;

        m_tagUrl.setScheme(TAG_SCHEME);
        m_tagUrl.setQuery("tagname=" + QString(TAG_NAME_A));
        m_tagUrl.setPath("/" + QString(TAG_NAME_A));

        m_tagFileUrl.setScheme(TAG_SCHEME);
        m_tagFileUrl.setTaggedFileUrl(tempTxtFilePath);
    }

    void TearDown() override
    {
        delete m_pManager;
        m_pManager = nullptr;

        delete m_pController;
        m_pController = nullptr;

        std::cout << "end TestTagController" << std::endl;
    }

public:
    TagController *m_pController = nullptr;
    TagManager *m_pManager = nullptr;
    QString tempDirPath_A;
    QString tempDirPath_B;
    QString tempTxtFilePath;
    DUrl m_tagUrl;
    DUrl m_tagFileUrl;
};
}

TEST_F(TestTagController, test_prepare)
{
    QProcess::execute("mkdir " + tempDirPath_A + " " + tempDirPath_B);
    QProcess::execute("touch " + tempTxtFilePath);

    ASSERT_NE(m_pManager, nullptr);
    QStringList tags { TAG_NAME_A, TAG_NAME_B };
    DUrlList files { DUrl::fromLocalFile(tempDirPath_A), DUrl::fromLocalFile(tempDirPath_B) };
    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, []{return QVariant(true);});
    EXPECT_NO_FATAL_FAILURE(m_pManager->makeFilesTags(tags, files));
}

TEST_F(TestTagController, can_create_file_info)
{
    ASSERT_NE(m_pController, nullptr);

    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, m_tagUrl);
    EXPECT_TRUE(m_pController->createFileInfo(event) != nullptr);
}

TEST_F(TestTagController, can_getChildren)
{
    ASSERT_NE(m_pController, nullptr);

    QSharedPointer<DFMGetChildrensEvent> event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, m_tagUrl, QStringList(), QDir::AllEntries | QDir::Hidden);
    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QStringList({tempDirPath_A, tempDirPath_B}));});
    EXPECT_TRUE(!m_pController->getChildren(event).isEmpty());

    m_tagUrl.setPath("/");
    auto eventRoot = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, m_tagUrl, QStringList(), QDir::AllEntries | QDir::Hidden);
    stExt.reset(&TagManagerDaemonController::disposeClientData);
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QMap<QString, QVariant>({{QString("a"), QVariant(QString("Red"))}, {QString("b"), QVariant(QString("Orange"))}}));});
    EXPECT_TRUE(!m_pController->getChildren(eventRoot).isEmpty());
}

TEST_F(TestTagController, can_creatFileWatcher)
{
    ASSERT_NE(m_pController, nullptr);

    auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, m_tagUrl);
    TaggedFileWatcher *watcher = static_cast<TaggedFileWatcher *>(m_pController->createFileWatcher(event));
    if (watcher) {
        watcher->addWatcher(m_tagUrl);
        watcher->removeWatcher(m_tagUrl);
    }
    EXPECT_TRUE(watcher != nullptr);

    if (watcher) {
        delete watcher;
        watcher = nullptr;
    }
}

TEST_F(TestTagController, can_open_file)
{
    ASSERT_NE(m_pController, nullptr);

    m_tagUrl.setTaggedFileUrl(tempDirPath_A);
    auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, m_tagUrl);

    StubExt st;
    st.set_lamda(&DFileService::openFile, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->openFile(event));
}

TEST_F(TestTagController, can_open_files)
{
    ASSERT_NE(m_pController, nullptr);

    DUrlList list;
    m_tagUrl.setTaggedFileUrl(tempDirPath_A);
    list.append(m_tagUrl);
    m_tagUrl.setTaggedFileUrl(tempDirPath_B);
    list.append(m_tagUrl);

    auto event = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, list);

    StubExt st;
    st.set_lamda(&FileUtils::openFiles, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->openFiles(event));
}

TEST_F(TestTagController, can_setFileTags)
{
    auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, m_tagFileUrl, QList<QString>({"红色"}));

    StubExt st;
    st.set_lamda(&DFileService::setFileTags, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->setFileTags(event));
}

TEST_F(TestTagController, can_getTagsThroughFiles)
{
    auto event = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>({m_tagFileUrl}));

    StubExt st;
    st.set_lamda(&DFileService::getTagsThroughFiles, [](){ return QList<QString>(); });

    EXPECT_NO_FATAL_FAILURE(m_pController->getTagsThroughFiles(event));
}

TEST_F(TestTagController, can_removeTagsOfFile)
{
    auto event = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, m_tagFileUrl, QList<QString>({"红色"}));

    StubExt st;
    st.set_lamda(&DFileService::removeTagsOfFile, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->removeTagsOfFile(event));
}

TEST_F(TestTagController, can_openFileByApp)
{
    auto event = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", m_tagFileUrl);

    StubExt st;
    st.set_lamda(&DFileService::openFileByApp, [](){ return true; });

    EXPECT_TRUE(m_pController->openFileByApp(event));
}

TEST_F(TestTagController, can_openFileLocation)
{
    auto tagEvent = dMakeEventPointer<DFMOpenFileLocation>(nullptr, m_tagUrl);
    auto fileEvent = dMakeEventPointer<DFMOpenFileLocation>(nullptr, m_tagFileUrl);

    StubExt st;
    st.set_lamda(&DFileService::openFileLocation, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->openFileLocation(tagEvent));
    EXPECT_NO_FATAL_FAILURE(m_pController->openFileLocation(fileEvent));
}

TEST_F(TestTagController, can_writeFilesToClipboard)
{
    auto event = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << m_tagFileUrl);

    StubExt st;
    st.set_lamda(&DFileService::writeFilesToClipboard, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->writeFilesToClipboard(event));
}

TEST_F(TestTagController, can_setPermissions)
{
    auto event = dMakeEventPointer<DFMSetPermissionEvent>(nullptr, m_tagFileUrl, (QFileDevice::ReadOther | QFileDevice::ReadGroup | QFileDevice::ReadOwner));
    StubExt st;
    st.set_lamda(&DFileService::setPermissions, [](){ return true; });
    EXPECT_NO_FATAL_FAILURE(m_pController->setPermissions(event));
}

TEST_F(TestTagController, can_addToBookmark)
{
    auto event = dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, m_tagUrl);
    StubExt st;
    st.set_lamda(&DFileService::addToBookmark, [](){ return true; });
    EXPECT_NO_FATAL_FAILURE(m_pController->addToBookmark(event));
}

TEST_F(TestTagController, can_removeBookmark)
{
    auto event = dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, m_tagUrl);
    StubExt st;
    st.set_lamda(&DFileService::removeBookmark, [](){ return true; });
    EXPECT_NO_FATAL_FAILURE(m_pController->removeBookmark(event));
}

TEST_F(TestTagController, can_createSymlink)
{
    QString symlinkPath = QDir::currentPath() + "/tag_symlink.txt";
    auto event = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, m_tagFileUrl, DUrl::fromLocalFile(symlinkPath));
    EXPECT_TRUE(m_pController->createSymlink(event));
    QProcess::execute("rm -f " + symlinkPath);
}

TEST_F(TestTagController, can_shareFolder)
{
    auto tagEvent = dMakeEventPointer<DFMFileShareEvent>(nullptr, m_tagUrl, "tag_test");
    auto fileEvent = dMakeEventPointer<DFMFileShareEvent>(nullptr, m_tagFileUrl, "tag_file_test");

    StubExt st;
    st.set_lamda(&DFileService::shareFolder, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->shareFolder(tagEvent));
    EXPECT_NO_FATAL_FAILURE(m_pController->shareFolder(fileEvent));
}

TEST_F(TestTagController, can_unShareFolder)
{
    auto tagEvent = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, m_tagUrl);
    auto fileEvent = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, m_tagFileUrl);

    StubExt st;
    st.set_lamda(&DFileService::unShareFolder, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->unShareFolder(tagEvent));
    EXPECT_NO_FATAL_FAILURE(m_pController->unShareFolder(fileEvent));

}

TEST_F(TestTagController, can_openInTerminal)
{
    auto tagEvent = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, m_tagUrl);
    auto fileEvent = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, m_tagFileUrl);

    StubExt st;
    st.set_lamda(&DFileService::openInTerminal, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->openInTerminal(tagEvent));
    EXPECT_NO_FATAL_FAILURE(m_pController->openInTerminal(fileEvent));
}

TEST_F(TestTagController, can_renameFile)
{
    QString temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + NEW_TAG_TXT_FILE;
    DUrl to;
    to.setScheme(SEARCH_SCHEME);
    to.setTaggedFileUrl(temp);
    auto event = dMakeEventPointer<DFMRenameEvent>(nullptr, m_tagFileUrl, to);

    StubExt st;
    st.set_lamda(&DFileService::renameFile, [](){ return true; });

    EXPECT_NO_FATAL_FAILURE(m_pController->renameFile(event));
}

TEST_F(TestTagController, test_wind_up)
{
    ASSERT_NE(m_pManager, nullptr);
    QStringList tags { TAG_NAME_A, TAG_NAME_B };
    StubExt st;
    st.set_lamda(&TagManagerDaemonController::disposeClientData, [](){ return QVariant(true); });
    EXPECT_NO_FATAL_FAILURE(m_pManager->deleteTags(tags));

    QString temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + NEW_TAG_TXT_FILE;
    QProcess::execute("rm " + temp);
    QProcess::execute("rm -rf " + tempDirPath_A + " " + tempDirPath_B);
}
