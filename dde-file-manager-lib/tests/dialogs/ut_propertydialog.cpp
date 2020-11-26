

#include <gtest/gtest.h>
#include <QWidget>
#include <QHBoxLayout>

#define protected public
#define private public
#include "dialogs/propertydialog.h"
#include "dfileservices.h"

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

}

TEST_F(TestDFMRoundBackground, testEventFilter)
{
    QEvent event(QEvent::Paint);
    m_pTester->eventFilter(m_pWidget, &event);
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
}

TEST_F(TestNameTextEdit, testFocusOutEvent)
{
    QFocusEvent event(QEvent::FocusOut);
    m_pTester->focusOutEvent(&event);
}

TEST_F(TestNameTextEdit, testKeyPressEvent_Escape)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_pTester->keyPressEvent(&event);
}

TEST_F(TestNameTextEdit, testKeyPressEvent_Enter)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    m_pTester->keyPressEvent(&event);
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

}

TEST_F(TestLinkSectionValueLabel, testSetOrGetLinkTargetUrl)
{
    DUrl url("file:///home");
    const char *strSrc = url.toString().toStdString().c_str();
    m_pTester->setLinkTargetUrl(url);
    DUrl result = m_pTester->linkTargetUrl();
    EXPECT_STREQ(strSrc, result.toString().toStdString().c_str());
}

TEST_F(TestLinkSectionValueLabel, testMouseReleaseEvent)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0),Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    m_pTester->mouseReleaseEvent(&event);
}

namespace  {
    class TestPropertyDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pWidget = new QWidget();
            DFMEvent event(DFMEvent::OpenFile, m_pWidget);
            QString strPath = QDir::homePath() + QDir::separator() + "TestPropertyDialog.txt";
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
            m_pTester = nullptr;
            if(m_pWidget){
                delete m_pWidget;
            }
            m_pWidget = nullptr;
            std::cout << "end TestPropertyDialog";
        }
    public:
        PropertyDialog  *m_pTester;
        QWidget *m_pWidget;
    };
}

TEST_F(TestPropertyDialog, testInit)
{

}

TEST_F(TestPropertyDialog, testStartComputerFolderSize)
{
    QString strPath = QDir::homePath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    m_pTester->startComputerFolderSize(url);
}

TEST_F(TestPropertyDialog, testToggleFileExecutable)
{
    m_pTester->toggleFileExecutable(false);
}

TEST_F(TestPropertyDialog, testUpdateInfo)
{
    m_pTester->updateInfo();
}

TEST_F(TestPropertyDialog, testUpdateFolderSize)
{
    m_pTester->updateFolderSize(1);
}

TEST_F(TestPropertyDialog, testRenameFile)
{
    m_pTester->renameFile();
}

TEST_F(TestPropertyDialog, testShowTextShowFrame)
{
    m_pTester->showTextShowFrame();
}

TEST_F(TestPropertyDialog, testOnChildrenRemoved)
{
    QString strPath = QDir::homePath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    m_pTester->onChildrenRemoved(url);
}

TEST_F(TestPropertyDialog, testFlickFolderToSidebar)
{
    QString strPath = QDir::homePath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    m_pTester->flickFolderToSidebar(url);
}

TEST_F(TestPropertyDialog, testOnOpenWithBntsChecked)
{
    QAbstractButton *w(nullptr);
    m_pTester->onOpenWithBntsChecked(w);
}

TEST_F(TestPropertyDialog, testOnHideFileCheckboxChecked)
{
    m_pTester->onHideFileCheckboxChecked(true);
}

TEST_F(TestPropertyDialog, testOnCancelShare)
{
    m_pTester->onCancelShare();
}

TEST_F(TestPropertyDialog, testMousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_pTester->mousePressEvent(&event);
}

TEST_F(TestPropertyDialog, testGetFileCount)
{
    m_pTester->getFileCount();
}

TEST_F(TestPropertyDialog, testGetFileSize)
{
    m_pTester->getFileSize();
}

TEST_F(TestPropertyDialog, testRaise)
{
    m_pTester->raise();
}

TEST_F(TestPropertyDialog, testHideEvent)
{
    QHideEvent event;
    m_pTester->hideEvent(&event);
}

TEST_F(TestPropertyDialog, testResizeEvent)
{
    QResizeEvent event(QSize(350, 120), QSize(350, 120));
    m_pTester->resizeEvent(&event);
}

TEST_F(TestPropertyDialog, testExpandGroup)
{
    m_pTester->expandGroup();
}

TEST_F(TestPropertyDialog, testContentHeight)
{
    m_pTester->contentHeight();
}

TEST_F(TestPropertyDialog, testGetDialogHeight)
{
    m_pTester->getDialogHeight();
}

TEST_F(TestPropertyDialog, testCreateShareInfoFrame)
{
    QString strPath = QDir::homePath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    m_pTester->createShareInfoFrame(info);
}

TEST_F(TestPropertyDialog, testCreateLocalDeviceInfoWidget)
{
    QString strPath = QDir::homePath() + QDir::separator() + "TestPropertyDialog.txt";
    DUrl url("file://" + strPath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    m_pTester->createLocalDeviceInfoWidget(info);
}

TEST_F(TestPropertyDialog, testCreateInfoFrame)
{
    QList<QPair<QString, QString> > properties;
    QPair<QString, QString> pair1("key1", "value1");
    QPair<QString, QString> pair2("key2", "value2");
    properties << pair1 << pair2;
    m_pTester->createInfoFrame(properties);
}
