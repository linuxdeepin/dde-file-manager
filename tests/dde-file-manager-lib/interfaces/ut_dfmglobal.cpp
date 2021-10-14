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

#include "dfmglobal.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "stubext.h"
#include "addr_pri.h"

#include "testhelper.h"

#include "singleton.h"
#include "app/define.h"
#include "dthumbnailprovider.h"
#include "dialogs/dialogmanager.h"
#include "plugins/pluginmanager.h"
#include "shutil/mimesappsmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "../usershare/usersharemanager.h"
#include "tag/tagmanager.h"
#include <dfmstandardpaths.h>

#include <QPainter>
#include <QTextCodec>
#include <QApplication>

using namespace stub_ext;

namespace  {
class TestDFMGlobal : public testing::Test
{
public:
    void SetUp() override
    {
        m_filePath = TestHelper::createTmpFile(".txt");
    }
    void TearDown() override
    {
        TestHelper::deleteTmpFile(m_filePath);
    }

public:
    QString m_filePath;
};
}

//！ 测试文件名的命名规范
TEST_F(TestDFMGlobal, test_fileNameRule)
{
    // 剔除的字符 (^\\s+|[\"'/\\\\\[\\\]:|<>?*\r\n])
    QString strInput("\"\'/\\[]:|<>?*");
    QString strOutput = DFMGlobal::preprocessingFileName(strInput);
    EXPECT_STREQ(strOutput.toStdString().c_str(), "");
}

TEST_F(TestDFMGlobal, test_getDefineStr)
{
    EXPECT_STREQ(DFMGlobal::organizationName().toStdString().c_str(), "deepin");
    EXPECT_STREQ(DFMGlobal::applicationName().toStdString().c_str(), "test-dde-file-manager-lib");
    EXPECT_TRUE(DFMGlobal::applicationVersion().contains("."));
}

TEST_F(TestDFMGlobal, test_installTranslator)
{
    EXPECT_FALSE(DFMGlobal::installTranslator());
}

ACCESS_PRIVATE_FUN(DFMGlobal, void(), onClipboardDataChanged);

TEST_F(TestDFMGlobal, test_clipboard)
{
    auto onClipboardDataChanged = get_private_fun::DFMGlobalonClipboardDataChanged();
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::fetchUrlsFromClipboard().isEmpty());

    QUrl url(QUrl::fromLocalFile(m_filePath));
    DFMGlobal::setUrlsToClipboard({ url }, DFMGlobal::CopyAction);
    EXPECT_EQ(DFMGlobal::fetchClipboardAction(), DFMGlobal::CopyAction);
    onClipboardDataChanged;
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->clipboardFileUrlList().isEmpty());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->clipboardFileInodeList().isEmpty());
    EXPECT_EQ(DFMGlobal::instance()->clipboardAction(), DFMGlobal::CopyAction);

    DFMGlobal::setUrlsToClipboard({ url }, DFMGlobal::CutAction);
    EXPECT_EQ(DFMGlobal::fetchClipboardAction(), DFMGlobal::CutAction);
    onClipboardDataChanged;
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->clipboardFileUrlList().isEmpty());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->clipboardFileInodeList().isEmpty());
    EXPECT_EQ(DFMGlobal::instance()->clipboardAction(), DFMGlobal::CutAction);

    EXPECT_NO_FATAL_FAILURE(DFMGlobal::fetchUrlsFromClipboard().isEmpty());

    DFMGlobal::clearClipboard();

    EXPECT_TRUE(DFMGlobal::fetchUrlsFromClipboard().isEmpty());
}

TEST_F(TestDFMGlobal, test_pluginLibrary)
{
    DFMGlobal::initPluginManager();

    DFMGlobal::autoLoadDefaultPlugins();
    EXPECT_EQ(DFMGlobal::PluginLibraryPaths.count(), 1);

    DFMGlobal::addPluginLibraryPath(PluginManager::PluginDir());
    EXPECT_EQ(DFMGlobal::PluginLibraryPaths.count(), 2);

    DFMGlobal::addPluginLibraryPaths({ PluginManager::PluginDir() });
    EXPECT_EQ(DFMGlobal::PluginLibraryPaths.count(), 3);
}
TEST_F(TestDFMGlobal, test_initManagers)
{
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initSearchHistoryManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initBookmarkManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initFileMenuManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initFileSiganlManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initSystemPathManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initMimeTypeDisplayManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initNetworkManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initSecretManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initDialogManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initFileService());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initAppcontroller());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initDeviceListener());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initUserShareManager());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initOperatorRevocation());
#ifndef __arm__
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initBluetoothManager());
#endif
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::initRootFileManager());
}
TEST_F(TestDFMGlobal, test_initMimesAppsManager)
{
    TestHelper::runInLoop([&] {
        DFMGlobal::initMimesAppsManager();
    }, 150);
    EXPECT_NO_FATAL_FAILURE(mimeAppsManager->DesktopFiles.isEmpty());
}

TEST_F(TestDFMGlobal, test_initGvfsMountManager)
{
    TestHelper::runInLoop([&] {
        EXPECT_NO_FATAL_FAILURE(DFMGlobal::initGvfsMountManager());
    }, 200);
}

static bool called = false;
TEST_F(TestDFMGlobal, test_initTagManagerConnect)
{
    StubExt stExt;
    TagManager *tagManager = new TagManager();
    stExt.set_lamda(&TagManager::instance, [&]() {return tagManager;});

    TagManager::instance()->makeFilesTagThroughColor("Red", {DUrl::fromLocalFile(m_filePath)});
    DFMGlobal::initTagManagerConnect();

    Stub st;

    called = false;
    DUrl(*fromUserTaggedFile)(const QString &, const QString &) = [](const QString &, const QString &) {called = true; return DUrl();};
    st.set(ADDR(DUrl, fromUserTaggedFile), fromUserTaggedFile);
    QMap<QString, QString> old_and_new_color;
    old_and_new_color.insert("Red", "Orange");
    TagManager::instance()->changeTagColor(old_and_new_color);
    EXPECT_TRUE(called);

    called = false;
    TagManager::instance()->addNewTag({"Red"});
    EXPECT_TRUE(called);

    called = false;
    TagManager::instance()->deleteTag({"Red"});
    EXPECT_TRUE(called);

    called = false;
    TagManager::instance()->changeTagName({{"Red", "Orange"}});
    EXPECT_TRUE(called);

    called = false;
    DUrl(*fromLocalFile)(const QString &) = [](const QString &) {called = true; return DUrl();};
    st.set(ADDR(DUrl, fromLocalFile), fromLocalFile);
    TagManager::instance()->filesWereTagged({{m_filePath, {"Blue"}}});
    EXPECT_TRUE(called);

    called = false;
    TagManager::instance()->untagFiles({{m_filePath, {"Blue"}}});
    EXPECT_TRUE(called);

    if (tagManager) {
        delete tagManager;
        tagManager = nullptr;
    }
}

TEST_F(TestDFMGlobal, test_initThumbnailConnection)
{
    DFMGlobal::initThumbnailConnection();

    Stub st;

    called = false;
    DUrl(*fromLocalFile)(const QString &) = [](const QString &) {called = true; return DUrl();};
    st.set(ADDR(DUrl, fromLocalFile), fromLocalFile);
    DThumbnailProvider::instance()->createThumbnailFinished(m_filePath, QString());
    EXPECT_TRUE(called);
}

TEST_F(TestDFMGlobal, test_userInfo)
{
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::getUser());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::getUserId());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::isRootUser());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::isServerSys());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::isDesktopSys());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::isOpenAsAdmin());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::isDeveloperMode());
}

#ifndef __arm__
TEST_F(TestDFMGlobal, test_standardIcon)
{
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon).isNull());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->standardIcon(DFMGlobal::LockIcon).isNull());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->standardIcon(DFMGlobal::UnreadableIcon).isNull());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->standardIcon(DFMGlobal::ShareIcon).isNull());
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::instance()->standardIcon(static_cast<DFMGlobal::Icon>(4)).isNull());
}

TEST_F(TestDFMGlobal, test_wordWrapText)
{
    qreal height = 1;
    EXPECT_NO_FATAL_FAILURE(QString(DFMGlobal::wordWrapText(QString("file"), 117, QTextOption::WrapAtWordBoundaryOrAnywhere, QFont(), 28, &height)));

    QStringList lines;
    QPainter painter;
    QList<QRectF> boundingRegion;

    QTextLayout layout_0("dde-file-manager");
    DFMGlobal::elideText(&layout_0, QSizeF(117, 22), QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::ElideNone, 28, Qt::AlignHCenter, &lines,
                         &painter, QPointF(), QColor(), QPointF(0, 1), QBrush(Qt::blue), 4, &boundingRegion);
    EXPECT_NO_FATAL_FAILURE(lines.length());
    lines.clear();

    QTextLayout layout_1("dde-file-manager");
    DFMGlobal::elideText(&layout_1, QSizeF(100, 100), QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::ElideNone, 28, Qt::AlignHCenter, &lines,
                         &painter, QPointF(), QColor(), QPointF(0, 1), QBrush(Qt::blue), 4, &boundingRegion);
    EXPECT_NO_FATAL_FAILURE(lines.length());
    lines.clear();

    QTextLayout layout_2("dde-file-manager");
    DFMGlobal::elideText(&layout_2, QSizeF(100, 100), QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::ElideNone, 28, Qt::AlignHCenter, &lines,
                         &painter, QPointF(), QColor(Qt::red), QPointF(0, 1), QBrush(Qt::blue), 1, &boundingRegion);
    EXPECT_NO_FATAL_FAILURE(lines.length());
    lines.clear();

    QTextLayout layout_3("dde-file-manag");
    DFMGlobal::elideText(&layout_3, QSizeF(100, 300), QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::ElideNone, 25, Qt::AlignHCenter, &lines,
                         &painter, QPointF(), QColor(Qt::red), QPointF(0, 1), QBrush(Qt::blue), 1, &boundingRegion);
    EXPECT_NO_FATAL_FAILURE(lines.length());
    lines.clear();

    EXPECT_NO_FATAL_FAILURE(DFMGlobal::elideText(QString("dde-file-manager"), QSizeF(100, 100), QTextOption::WrapAtWordBoundaryOrAnywhere, QFont(), Qt::ElideNone, 25));
}
#endif

TEST_F(TestDFMGlobal, test_chineseToPinyin)
{
    EXPECT_EQ(DFMGlobal::toPinyin(QString("单元测试")), QString("dan1yuan2ce4shi4"));
}

TEST_F(TestDFMGlobal, test_startWithHanzi)
{
    EXPECT_TRUE(DFMGlobal::startWithHanzi(QString("单元测试")));
    EXPECT_FALSE(DFMGlobal::startWithHanzi(QString("danyuanceshi")));
    EXPECT_FALSE(DFMGlobal::startWithHanzi(QString()));
}

TEST_F(TestDFMGlobal, test_startWithSymbol)
{
    EXPECT_TRUE(DFMGlobal::startWithSymbol(QString("!@#test")));
    EXPECT_FALSE(DFMGlobal::startWithSymbol(QString("test")));
    EXPECT_FALSE(DFMGlobal::startWithSymbol(QString()));
}

TEST_F(TestDFMGlobal, test_keyIsPressed)
{
    EXPECT_FALSE(DFMGlobal::keyShiftIsPressed());
    EXPECT_FALSE(DFMGlobal::keyCtrlIsPressed());
}

TEST_F(TestDFMGlobal, test_fileNameCorrection)
{
    EXPECT_FALSE(DFMGlobal::fileNameCorrection(m_filePath));
    EXPECT_TRUE(DFMGlobal::fileNameCorrection(QByteArray::fromStdString(m_filePath.toStdString())));
}

TEST_F(TestDFMGlobal, test_isDesktopFile)
{
    DUrl url = DUrl::fromLocalFile(QString("/usr/share/applications/dde-control-center.desktop"));

    EXPECT_FALSE(DFMGlobal::isTrashDesktopFile(url));
    EXPECT_FALSE(DFMGlobal::isComputerDesktopFile(url));
    EXPECT_FALSE(DFMGlobal::isTrashDesktopFileUrl(url));
    EXPECT_FALSE(DFMGlobal::isComputerDesktopFileUrl(url));

    QString trashDesktop = DFMStandardPaths::location(DFMStandardPaths::DesktopPath) + "/dde-trash.desktop";
    QString computerDesktop = DFMStandardPaths::location(DFMStandardPaths::DesktopPath) + "/dde-computer.desktop";
    EXPECT_EQ(DFMGlobal::isTrashDesktopFileUrl(DUrl::fromLocalFile(trashDesktop)), QFile::exists(trashDesktop));
    EXPECT_EQ(DFMGlobal::isComputerDesktopFileUrl(DUrl::fromLocalFile(computerDesktop)), QFile::exists(computerDesktop));
}

TEST_F(TestDFMGlobal, test_isWayLand)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(QApplication,platformName),[](){return "wayland";});
    EXPECT_TRUE(DFMGlobal::isWayLand());

    stub.set_lamda(ADDR(QApplication,platformName),[](){return "x11";});
    EXPECT_FALSE(DFMGlobal::isWayLand());
}

TEST_F(TestDFMGlobal, test_showMultiFilesRenameDialog)
{
    StubExt st;
    st.set_lamda(&DialogManager::showMultiFilesRenameDialog, []() {});
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::showMultiFilesRenameDialog(QList<DUrl>()));
}

TEST_F(TestDFMGlobal, test_showPropertyDialog)
{
    StubExt st;
    st.set_lamda(&DialogManager::showPropertyDialog, []() {});
    EXPECT_NO_FATAL_FAILURE(DFMGlobal::showPropertyDialog(nullptr, QList<DUrl>()));
}

TEST_F(TestDFMGlobal, test_cutString)
{
    QString fileName("test QString DFMGlobal::cutString(const QString &text, int dataByteSize, const QTextCodec *codec) function.");
    fileName = DFMGlobal::cutString(fileName, 50, QTextCodec::codecForLocale());
    EXPECT_EQ(fileName.length(), 50);
}
