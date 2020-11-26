#include <gtest/gtest.h>
#include <QIcon>
#include <QEvent>
#include <QRect>
#include <QPaintEvent>

#define protected public
#define private public
#include "dialogs/openwithdialog.h"

namespace  {
    class TestOpenWithDialogListItem : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new OpenWithDialogListItem(QIcon(), "OpenWithDialogListItem");
            std::cout << "start TestOpenWithDialogListItem";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestOpenWithDialogListItem";
        }
    public:
        OpenWithDialogListItem  *m_pTester;
    };
}

TEST_F(TestOpenWithDialogListItem, testInit)
{

}

TEST_F(TestOpenWithDialogListItem, testSetCheckedTrue)
{
    m_pTester->setChecked(true);
}

TEST_F(TestOpenWithDialogListItem, testSetCheckedFalse)
{
    m_pTester->setChecked(false);
}

TEST_F(TestOpenWithDialogListItem, testText)
{
    m_pTester->text();
}

TEST_F(TestOpenWithDialogListItem, testEnterEvent)
{
    QEvent event(QEvent::KeyPress);
    m_pTester->enterEvent(&event);
}

TEST_F(TestOpenWithDialogListItem, testLeaveEvent)
{
    QEvent event(QEvent::KeyPress);
    m_pTester->leaveEvent(&event);
}

TEST_F(TestOpenWithDialogListItem, testPaintEvent)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    m_pTester->paintEvent(&event);
}

namespace  {
    class TestOpenWithDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            DUrl url("file:///test3");
            m_pTester = new OpenWithDialog(url);
            std::cout << "start TestOpenWithDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestOpenWithDialog";
        }
    public:
        OpenWithDialog  *m_pTester;
    };
}

TEST_F(TestOpenWithDialog, testInit)
{

}

TEST_F(TestOpenWithDialog, testInit2)
{
    QList<DUrl> urllist;
    urllist << DUrl("file:///test1") << DUrl("file:///test2");
    OpenWithDialog dlg(urllist);
    dlg.show();
}

TEST_F(TestOpenWithDialog, testOpenFileByApp)
{
    m_pTester->openFileByApp();
}

TEST_F(TestOpenWithDialog, testUseOtherApplication)
{
    m_pTester->useOtherApplication();
}

TEST_F(TestOpenWithDialog, testCreateItem)
{
    m_pTester->createItem(QIcon::fromTheme("application-x-desktop"), "appName", "/appname");
}

TEST_F(TestOpenWithDialog, testShowEvent)
{
    QShowEvent event;
    m_pTester->showEvent(&event);
}

TEST_F(TestOpenWithDialog, testEventFilter)
{
    OpenWithDialogListItem item(QIcon(), "testItem");
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_pTester->eventFilter(&item, &event);
}
