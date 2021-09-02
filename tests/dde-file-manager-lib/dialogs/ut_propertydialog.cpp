/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

#include "stub.h"
#include "dfmglobal.h"
#include "io/dfilestatisticsjob.h"
#include "views/windowmanager.h"
#include "plugins/pluginmanager.h"
#include "../plugininterfaces/menu/menuinterface.h"
#include "interfaces/dabstractfileinfo.h"
#include "io/dstorageinfo.h"
#include "controllers/vaultcontroller.h"
#include "io/dfilestatisticsjob.h"
#include "app/define.h"
#include "dfileservices.h"

#define protected public
#define private public
#include "dialogs/propertydialog.h"
#include "dfileservices.h"
#include "views/dfilemanagerwindow.h"

namespace  {
    class TestDFMRoundBackground : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pWidget = new QWidget();
            m_pTester = new DFMRoundBackground(m_pWidget, 20);
            std::cout << "start TestDFMRoundBackground";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            if(m_pWidget){
                delete m_pWidget;
                m_pWidget = nullptr;
            }
            std::cout << "end TestDFMRoundBackground";
        }
    public:
        DFMRoundBackground  *m_pTester;
        QWidget *m_pWidget;
    };
}

TEST_F(TestDFMRoundBackground, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestDFMRoundBackground, testEventFilter)
{
    QEvent event(QEvent::Paint);
    EXPECT_NO_FATAL_FAILURE(m_pTester->eventFilter(m_pWidget, &event));
}

namespace  {
    class TestNameTextEdit : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new NameTextEdit();
            std::cout << "start TestNameTextEdit";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestNameTextEdit";
        }
    public:
        NameTextEdit    *m_pTester;
    };
}

TEST_F(TestNameTextEdit, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestNameTextEdit, testSetOrGerIsCanceled)
{
    bool isCanceled = true;
    m_pTester->setIsCanceled(isCanceled);
    bool result = m_pTester->isCanceled();
    EXPECT_TRUE(result);
}

TEST_F(TestNameTextEdit, testSetPlainText)
{
    QString str("unit test");
    m_pTester->setPlainText(str);
    QString str1 = m_pTester->toPlainText();
    EXPECT_TRUE(str1 == "unit test");
}

TEST_F(TestNameTextEdit, testFocusOutEvent)
{
    QFocusEvent event(QEvent::FocusOut);
    EXPECT_NO_FATAL_FAILURE(m_pTester->focusOutEvent(&event));
}

TEST_F(TestNameTextEdit, testKeyPressEvent_Escape)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_pTester->keyPressEvent(&event);
    EXPECT_EQ(m_pTester->m_isCanceled, true);
}

TEST_F(TestNameTextEdit, testKeyPressEvent_Enter)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    m_pTester->keyPressEvent(&event);
    EXPECT_EQ(m_pTester->m_isCanceled, false);
}

namespace  {
    class TestGroupTitleLabel : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new GroupTitleLabel();
            std::cout << "start TestGroupTitleLabel";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestGroupTitleLabel";
        }
    public:
        GroupTitleLabel *m_pTester;
    };
}

TEST_F(TestGroupTitleLabel, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

namespace  {
    class TestSectionKeyLabel : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new SectionKeyLabel();
            std::cout << "start TestSectionKeyLabel";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestSectionKeyLabel";
        }
    public:
        SectionKeyLabel *m_pTester;
    };
}

TEST_F(TestSectionKeyLabel, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

namespace  {
    class TestSectionValueLabel : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new SectionValueLabel();
            std::cout << "start TestSectionValueLabel";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestSectionValueLabel";
        }
    public:
        SectionValueLabel   *m_pTester;
    };
}

TEST_F(TestSectionValueLabel, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

namespace  {
    class TestLinkSectionValueLabel : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new LinkSectionValueLabel();
            std::cout << "start TestLinkSectionValueLabel";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestLinkSectionValueLabel";
        }
    public:
        LinkSectionValueLabel   *m_pTester;
    };
}

TEST_F(TestLinkSectionValueLabel, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestLinkSectionValueLabel, testSetOrGetLinkTargetUrl)
{
    DUrl url("file:///home");
    m_pTester->setLinkTargetUrl(url);
    DUrl result = m_pTester->linkTargetUrl();
    EXPECT_EQ(url, result);
}

TEST_F(TestLinkSectionValueLabel, testMouseReleaseEvent)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0),Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->mouseReleaseEvent(&event));
}

namespace  {
    class TestPropertyDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pWidget = new QWidget();
            DFMEvent event(DFMEvent::OpenFile, m_pWidget);
            QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
            QFile file(strPath);
            if(!file.exists()){
                if(file.open(QIODevice::ReadWrite | QIODevice::Text))
                    file.close();
            }
            DUrl url("file://" + strPath);
            m_pTester = new PropertyDialog(event, url, m_pWidget);
            std::cout << "start TestPropertyDialog";
        }
        void TearDown() override
        {
            QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt.txt";
            QFile file(strPath);
            if(file.exists()){
                file.remove();
            }
            if(m_pWidget){
                delete m_pWidget;
                m_pWidget = nullptr;
            }
            std::cout << "end TestPropertyDialog";
        }
    public:
        PropertyDialog  *m_pTester;
        QWidget *m_pWidget;
    };
}

TEST_F(TestPropertyDialog, testInit)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    m_pWidget = new QWidget();
    DFMEvent event(DFMEvent::OpenFile, m_pWidget);
    PropertyDialog dlg(event, DUrl("file:///test1"));
    QString str = dlg.m_url.toString();
    EXPECT_TRUE(str == "file:///test1");
}

TEST_F(TestPropertyDialog, testInit2)
{
    m_pWidget = new QWidget();
    DFMEvent event(DFMEvent::OpenFile, m_pWidget);
    PropertyDialog dlg(event, DUrl("dfmroot:///test1"));
    QString str = dlg.m_url.toString();
    EXPECT_TRUE(str == "dfmroot:///test1");
}

TEST_F(TestPropertyDialog, testStartComputerFolderSize)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);

    void(*stub_start)(const DUrlList &) = [](const DUrlList &){
        int a = 0;
    };
    Stub stu;
    stu.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stub_start);

    EXPECT_NO_FATAL_FAILURE(m_pTester->startComputerFolderSize(url));
}

TEST_F(TestPropertyDialog, testToggleFileExecutable)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->toggleFileExecutable(false));
}

TEST_F(TestPropertyDialog, testToggleFileExecutable2)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->toggleFileExecutable(true));
}

TEST_F(TestPropertyDialog, testUpdateInfo)
{
    m_pTester->updateInfo();
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    QString str = m_pTester->m_url.toString();
    EXPECT_TRUE(str == url.toString());
}

TEST_F(TestPropertyDialog, testUpdateFolderSize)
{
    DFM_NAMESPACE::DFileStatisticsJob job;
    m_pTester->m_sizeWorker = &job;
    m_pTester->updateFolderSize(1);
    qint64 size = m_pTester->m_size;
    EXPECT_TRUE(size == 1);
}

TEST_F(TestPropertyDialog, testRenameFile)
{
    m_pTester->renameFile();
    QString str = m_pTester->m_edit->placeholderText();
    EXPECT_TRUE(str == "");
}

TEST_F(TestPropertyDialog, testRenameFile2)
{
    m_pTester->m_url = DUrl("file:///home");
    m_pTester->renameFile();
    QString str = m_pTester->m_edit->placeholderText();
    EXPECT_TRUE(str == "");
}

TEST_F(TestPropertyDialog, testShowTextShowFrame)
{
    Stub stl;
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);

    m_pTester->showTextShowFrame();
    QString str = m_pTester->m_edit->toPlainText();
    EXPECT_TRUE(str == "TestPropertyDialog.txt");
}

TEST_F(TestPropertyDialog, testShowTextShowFrame2)
{
    Stub stl;
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);

    m_pTester->m_edit->setPlainText("");
    m_pTester->showTextShowFrame();
    QString str = m_pTester->m_edit->toPlainText();
    EXPECT_TRUE(str == "");
}

TEST_F(TestPropertyDialog, testShowTextShowFrame3)
{
    Stub stl;
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);

    m_pTester->m_url = DUrl("file:///home");
    m_pTester->showTextShowFrame();
    QString str = m_pTester->m_edit->toPlainText();
    EXPECT_TRUE(str == "TestPropertyDialog.txt");
}

TEST_F(TestPropertyDialog, testShowTextShowFrame4)
{
    Stub stl;
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);

    m_pTester->m_url = DUrl("file:///home");

    bool(*stu_renameFile)(const QObject *, const DUrl &, const DUrl &, const bool) = [](const QObject *, const DUrl &, const DUrl &, const bool)->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFileService, renameFile), stu_renameFile);

    m_pTester->showTextShowFrame();
    QString str = m_pTester->m_edit->toPlainText();
    EXPECT_TRUE(str == "TestPropertyDialog.txt");
}

TEST_F(TestPropertyDialog, testOnChildrenRemoved)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    m_pTester->onChildrenRemoved(url);
}

TEST_F(TestPropertyDialog, testOnChildrenRemoved2)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    m_pTester->m_url = DUrl("usershare:///test1");
    EXPECT_NO_FATAL_FAILURE(m_pTester->onChildrenRemoved(url));
}

TEST_F(TestPropertyDialog, testFlickFolderToSidebar)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    EXPECT_NO_FATAL_FAILURE(m_pTester->flickFolderToSidebar(url));
}

TEST_F(TestPropertyDialog, testFlickFolderToSidebar2)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);

    bool(*stu_openNewTab)(DUrl fileUrl) = [](DUrl fileUrl)->bool{
        Q_UNUSED(fileUrl);
        return true;
    };
    Stub stu2;
    stu2.set(ADDR(DFileManagerWindow, openNewTab), stu_openNewTab);
    void(*stu_initTitleBar)() = [](){};
    Stub stu3;
    stu3.set(ADDR(DFileManagerWindow, initTitleBar), stu_initTitleBar);
    static DFileManagerWindow * w = nullptr;
    QWidget*(*stub_getWindowById)(quint64) = [](quint64)->QWidget*{
        w = new DFileManagerWindow();
        return w;
    };

    Stub stub;
    stub.set(ADDR(WindowManager, getWindowById), stub_getWindowById);
    EXPECT_NO_FATAL_FAILURE(m_pTester->flickFolderToSidebar(url));

    if (w) {
        w->deleteLater();
        w = nullptr;
    }
}

TEST_F(TestPropertyDialog, testOnOpenWithBntsChecked)
{
    QPushButton w;
    EXPECT_NO_FATAL_FAILURE(m_pTester->onOpenWithBntsChecked(&w));
}

TEST_F(TestPropertyDialog, testOnHideFileCheckboxChecked)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->onHideFileCheckboxChecked(true));
}

TEST_F(TestPropertyDialog, testOnHideFileCheckboxChecked2)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->onHideFileCheckboxChecked(false));
}

TEST_F(TestPropertyDialog, testOnCancelShare)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->onCancelShare());
}

TEST_F(TestPropertyDialog, testMousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    bool(*stub_isVisible)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(QWidget, isVisible), stub_isVisible);

    EXPECT_NO_FATAL_FAILURE(m_pTester->mousePressEvent(&event));
}

TEST_F(TestPropertyDialog, testGetFileCount)
{
    int count = m_pTester->getFileCount();
    EXPECT_EQ(count, 1);
}

TEST_F(TestPropertyDialog, testGetFileSize)
{
    qint64 size = m_pTester->getFileSize();
    EXPECT_EQ(size, 0);
}

TEST_F(TestPropertyDialog, testRaise)
{
    EXPECT_NO_FATAL_FAILURE(m_pTester->raise());
}

TEST_F(TestPropertyDialog, testHideEvent)
{
    QHideEvent event;

    QPointer<QVariantAnimation> xani(new QVariantAnimation());
    m_pTester->m_xani = xani;

    QPointer<QVariantAnimation> gani(new QVariantAnimation());
    m_pTester->m_gani = gani;

    QPointer<QLabel> aniLabel(new QLabel(""));
    m_pTester->m_aniLabel = aniLabel;

    DFM_NAMESPACE::DFileStatisticsJob job;
    m_pTester->m_sizeWorker = &job;

    EXPECT_NO_FATAL_FAILURE(m_pTester->hideEvent(&event));
}

TEST_F(TestPropertyDialog, testResizeEvent)
{
    QResizeEvent event(QSize(350, 120), QSize(350, 120));
    EXPECT_NO_FATAL_FAILURE(m_pTester->resizeEvent(&event));
}

TEST_F(TestPropertyDialog, testExpandGroup)
{
    QList<DDrawer *> lst = m_pTester->expandGroup();
    EXPECT_EQ(lst.count(), 3);
}

TEST_F(TestPropertyDialog, testContentHeight)
{
    int height = m_pTester->contentHeight();
    EXPECT_NE(height, 0);
}

TEST_F(TestPropertyDialog, testGetDialogHeight)
{
    int height = m_pTester->getDialogHeight();
    EXPECT_NE(height, 0);
}

TEST_F(TestPropertyDialog, testCreateShareInfoFrame)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    ShareInfoFrame * pframe = m_pTester->createShareInfoFrame(info);
    EXPECT_NE(pframe, nullptr);
}

TEST_F(TestPropertyDialog, testCreateLocalDeviceInfoWidget)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    QList<QPair<QString, QString> > lst = m_pTester->createLocalDeviceInfoWidget(info);
    EXPECT_NE(lst.count(), 0);
}

TEST_F(TestPropertyDialog, testCreateLocalDeviceInfoWidget2)
{
    DAbstractFileInfoPointer info = DAbstractFileInfo::getFileInfo(DUrl("file:///test1"));
    QList<QPair<QString, QString> > lst = m_pTester->createLocalDeviceInfoWidget(info);
    EXPECT_EQ(lst.count(), 0);
}

TEST_F(TestPropertyDialog, testCreateLocalDeviceInfoWidget3)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    QList<QPair<QString, QString> > lst = m_pTester->createLocalDeviceInfoWidget(info);
    EXPECT_NE(lst.count(), 0);
}

TEST_F(TestPropertyDialog, testCreateInfoFrame)
{
    QList<QPair<QString, QString> > properties;
    QPair<QString, QString> pair1("key1", "value1");
    QPair<QString, QString> pair2("key2", "value2");
    properties << pair1 << pair2;
    QFrame *pframe = m_pTester->createInfoFrame(properties);
    EXPECT_NE(pframe, nullptr);

}

TEST_F(TestPropertyDialog, testGetRealUrl)
{
    m_pTester->m_url = DUrl("recent:///test1");
    DUrl url = m_pTester->getRealUrl();
    QString str = url.toString();
    EXPECT_TRUE(str == "file:///test1");
}

TEST_F(TestPropertyDialog, testCanChmod)
{
    DAbstractFileInfoPointer info(new DAbstractFileInfo(DUrl("burn:///test/test1")));
    bool b = m_pTester->canChmod(info);
    EXPECT_GE(b, false);
}

TEST_F(TestPropertyDialog, testLoadPluginExpandWidgets)
{
    static PropertyDialogExpandInfoInterface* p1 = nullptr;
    static PropertyDialogExpandInfoInterface* p2 = nullptr;
    static PropertyDialogExpandInfoInterface* p3 = nullptr;
    QList<PropertyDialogExpandInfoInterface*>(*stub_getExpandInfoInterfaces)() = []()->QList<PropertyDialogExpandInfoInterface*>{
        QList<PropertyDialogExpandInfoInterface*> lst;
            p1 = new PropertyDialogExpandInfoInterface();
            p2 = new PropertyDialogExpandInfoInterface();
            p3 = new PropertyDialogExpandInfoInterface();
            lst << p1 << p2 << p3;
            return lst;
    };
    Stub stu;
    stu.set(ADDR(PluginManager, getExpandInfoInterfaces), stub_getExpandInfoInterfaces);

    EXPECT_NO_FATAL_FAILURE(m_pTester->loadPluginExpandWidgets());

    if (p1) {
        delete p1;
        p1 = nullptr;
    }
    if (p2) {
        delete p2;
        p2 = nullptr;
    }
    if (p3) {
        delete p3;
        p3 = nullptr;
    }
}

//TEST_F(TestPropertyDialog, testCreateBasicInfoWidget)
//{
//    DAbstractFileInfoPointer info = DAbstractFileInfo::getFileInfo(DUrl("file:///home"));

//    bool(*stub_isSymLink)() = []()->bool{
//        return true;
//    };
//    typedef bool(*fptr)();
//    Stub stu;
//    stu.set((fptr)&DFileInfo::isSymLink, stub_isSymLink);

//    EXPECT_NO_FATAL_FAILURE(m_pTester->createBasicInfoWidget(info));
//}

//TEST_F(TestPropertyDialog, testCreateBasicInfoWidget2)
//{
//    bool(*stub_isEmpty)() = []()->bool{
//        return false;
//    };
//    Stub stu;
//    stu.set(ADDR(QString, isEmpty), stub_isEmpty);

//    DAbstractFileInfoPointer info = DAbstractFileInfo::getFileInfo(DUrl("file:///home"));
//    QFrame *pframe = m_pTester->createBasicInfoWidget(info);
//    EXPECT_NE(pframe, nullptr);
//}

TEST_F(TestPropertyDialog, testCreateBasicInfoWidget3)
{
    bool(*stub_isRecentFile)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DUrl, isRecentFile), stub_isRecentFile);

    DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, DUrl("file:///home"));
    QFrame *pframe = m_pTester->createBasicInfoWidget(info);
    EXPECT_NE(pframe, nullptr);
}

TEST_F(TestPropertyDialog, testCreateAuthorityManagementWidget)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    QFrame *pframe = m_pTester->createAuthorityManagementWidget(info);
    EXPECT_NE(pframe, nullptr);

    delete pframe;
}

TEST_F(TestPropertyDialog, testCreateAuthorityManagementWidget2)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);

    QByteArray(*stub_fileSystemType)() = []()->QByteArray{
        return QByteArray("vfat");
    };
    Stub stu;
    stu.set(ADDR(DStorageInfo, fileSystemType), stub_fileSystemType);

    QFrame *pframe = m_pTester->createAuthorityManagementWidget(info);
    EXPECT_NE(pframe, nullptr);

    delete pframe;
}

TEST_F(TestPropertyDialog, testCreateAuthorityManagementWidget3)
{
    QString strPath = QDir::currentPath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);

    bool(*stub_isVaultFile)(QString) = [](QString)->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(VaultController, isVaultFile), stub_isVaultFile);

    QFrame *pframe = m_pTester->createAuthorityManagementWidget(info);
    EXPECT_NE(pframe, nullptr);

    delete pframe;
}
