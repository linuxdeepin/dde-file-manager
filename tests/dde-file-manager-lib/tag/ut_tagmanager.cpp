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

#include <QStandardPaths>
#include <QProcess>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include <QDir>
#include "dfmevent.h"
#include "testhelper.h"

#define private public
#include "controllers/tagcontroller.h"
#include "tag/tagmanager.h"
#include "controllers/tagmanagerdaemoncontroller.h"
#include "stubext.h"

#define TAG_FILE_PATH_STR "test_tag_"
#define TAG_NAME_A "a"
#define TAG_NAME_A_NEW "a_new"
#define TAG_NAME_B "b"
#define TAG_TXT_FILE "tag_test.txt";

using namespace stub_ext;

namespace  {
    class TestTagManager : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestTagManager" << std::endl;

            m_pManager = new TagManager();
            m_pDaemon = new TagManagerDaemonController();
            m_pDaemon->disconnect();

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
            qApp->processEvents();
            delete m_pDaemon;
            m_pDaemon = nullptr;
            delete m_pManager;
            m_pManager = nullptr;

            std::cout << "end TestTagManager" << std::endl;
        }
    public:
        TagManager *m_pManager = nullptr;
        TagManagerDaemonController *m_pDaemon = nullptr;
        QString tempDirPath_A;
        QString tempDirPath_B;
        QString tempTxtFilePath;
        DUrl m_tagUrl;
        DUrl m_tagFileUrl;
    };
}

TEST_F(TestTagManager, test_prepare)
{
    QProcess::execute("mkdir " + tempDirPath_A);
    QProcess::execute("mkdir " + tempDirPath_B);
    QProcess::execute("touch " + tempTxtFilePath);
}

TEST_F(TestTagManager, test_daemon)
{
    ASSERT_NE(m_pDaemon, nullptr);

    EXPECT_NO_FATAL_FAILURE(m_pDaemon->onAddNewTags(QDBusVariant()));
    EXPECT_NO_FATAL_FAILURE(m_pDaemon->onChangeTagColor(QVariantMap()));
    EXPECT_NO_FATAL_FAILURE(m_pDaemon->onChangeTagName(QVariantMap()));
    EXPECT_NO_FATAL_FAILURE(m_pDaemon->onDeleteTags(QDBusVariant()));
    EXPECT_NO_FATAL_FAILURE(m_pDaemon->onFilesWereTagged(QVariantMap()));
    EXPECT_NO_FATAL_FAILURE(m_pDaemon->onUntagFiles(QVariantMap()));
}

TEST_F(TestTagManager, can_make_files_tags)
{
    ASSERT_NE(m_pManager, nullptr);

    QStringList tags { TAG_NAME_A, TAG_NAME_B };
    DUrlList files { DUrl::fromLocalFile(tempDirPath_A), DUrl::fromLocalFile(tempDirPath_B) };

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, []{return QVariant(true);});

    EXPECT_TRUE(m_pManager->makeFilesTags(tags, files));
}

TEST_F(TestTagManager, can_getALLTags)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QMap<QString, QVariant>({{QString(TAG_NAME_A), QVariant(QString("Red"))}, {QString(TAG_NAME_B), QVariant(QString("Orange"))}}));});
    EXPECT_TRUE(!m_pManager->getAllTags().isEmpty());
}

TEST_F(TestTagManager, can_getTagsThroughFiles)
{
    ASSERT_NE(m_pManager, nullptr);

    DUrlList files { DUrl::fromLocalFile(tempDirPath_A), DUrl::fromLocalFile(tempDirPath_B) };
    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QStringList({tempDirPath_A, tempDirPath_B}));});

    EXPECT_TRUE(!m_pManager->getTagsThroughFiles(files).isEmpty());
}

TEST_F(TestTagManager, can_getTagColor)
{
    ASSERT_NE(m_pManager, nullptr);

    QStringList tags { TAG_NAME_A, TAG_NAME_B };
    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QMap<QString, QVariant>({{QString(TAG_NAME_A), QVariant(QString("Red"))}, {QString(TAG_NAME_B), QVariant(QString("Orange"))}}));});
    EXPECT_TRUE(!m_pManager->getTagColor(tags).isEmpty());
}

TEST_F(TestTagManager, can_getTagColorName)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QMap<QString, QVariant>({{QString(TAG_NAME_A), QVariant(QString("Red"))}}));});
    EXPECT_TRUE(!m_pManager->getTagColorName(TAG_NAME_A).isEmpty());
}

TEST_F(TestTagManager, can_getTagIconNameByString)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QMap<QString, QVariant>({{QString(TAG_NAME_A), QVariant(QString("Red"))}}));});
    EXPECT_TRUE(!m_pManager->getTagIconName(QString(TAG_NAME_A)).isEmpty());
}

TEST_F(TestTagManager, can_getTagIconNameByColor)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QMap<QString, QVariant>({{QString(TAG_NAME_A), QVariant(QString("Red"))}}));});
    QString colorName = m_pManager->getTagColorName(TAG_NAME_A);
    QColor color = m_pManager->getColorByColorName(colorName);
    if (color.isValid())
        EXPECT_TRUE(!m_pManager->getTagIconName(color).isEmpty());
}

TEST_F(TestTagManager, can_getFilesThroughTag)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QStringList({tempDirPath_A}));});
    EXPECT_TRUE(!m_pManager->getFilesThroughTag(TAG_NAME_A).isEmpty());
}

TEST_F(TestTagManager, can_getColorByDisplayName)
{
    ASSERT_NE(m_pManager, nullptr);

    EXPECT_TRUE(!m_pManager->getColorByDisplayName(QObject::tr("Orange")).isEmpty());
}

TEST_F(TestTagManager, can_getColorNameByColor)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(QMap<QString, QVariant>({{QString(TAG_NAME_A), QVariant(QString("Red"))}}));});
    QString colorName = m_pManager->getTagColorName(TAG_NAME_A);
    QColor color = m_pManager->getColorByColorName(colorName);
    EXPECT_EQ(m_pManager->getColorNameByColor(color), colorName);
}

TEST_F(TestTagManager, can_allTagOfDefaultColors)
{
    ASSERT_NE(m_pManager, nullptr);

    EXPECT_TRUE(!m_pManager->allTagOfDefaultColors().isEmpty());
}

TEST_F(TestTagManager, can_changeTagColor)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(true);});
    EXPECT_TRUE(m_pManager->changeTagColor(TAG_NAME_A, "Orange"));
}

TEST_F(TestTagManager, can_changeTagName)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(true);});
    QPair<QString, QString> pair(QString(TAG_NAME_A), QString(TAG_NAME_A_NEW));
    EXPECT_TRUE(m_pManager->changeTagName(pair));
}

TEST_F(TestTagManager, can_makeFilesTagThroughColor)
{
    ASSERT_NE(m_pManager, nullptr);

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(true);});
    EXPECT_TRUE(m_pManager->makeFilesTagThroughColor("#ff1c49", {DUrl::fromLocalFile(tempDirPath_A)}));
}

TEST_F(TestTagManager, can_remove_tags_of_files)
{
    ASSERT_NE(m_pManager, nullptr);

    QStringList tags { TAG_NAME_A };
    DUrlList files { DUrl::fromLocalFile(tempDirPath_A), DUrl::fromLocalFile(tempDirPath_B) };

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(true);});
    EXPECT_TRUE(m_pManager->removeTagsOfFiles(tags, files));
}

TEST_F(TestTagManager, can_delete_files)
{
    ASSERT_NE(m_pManager, nullptr);
    DUrlList files { DUrl::fromLocalFile(tempDirPath_A), DUrl::fromLocalFile(tempDirPath_B) };

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(true);});
    EXPECT_TRUE(m_pManager->deleteFiles(files));
}

TEST_F(TestTagManager, can_delete_tags)
{
    ASSERT_NE(m_pManager, nullptr);

    QStringList tags { TAG_NAME_A_NEW, TAG_NAME_B };

    StubExt stExt;
    stExt.set_lamda(&TagManagerDaemonController::disposeClientData, [&]{return QVariant(true);});
    EXPECT_TRUE(m_pManager->deleteTags(tags));
}

TEST_F(TestTagManager, test_wind_up)
{
    QProcess::execute("rm " + tempTxtFilePath);
    QProcess::execute("rm -r " + tempDirPath_A);
    QProcess::execute("rm -r " + tempDirPath_B);
}
