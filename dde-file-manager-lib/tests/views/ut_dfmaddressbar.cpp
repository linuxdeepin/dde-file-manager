#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QtTest>
#include <QAction>
#include <QCompleter>
#include <QStandardPaths>
#include <durl.h>
#include <dspinner.h>
#include "views/dcompleterlistview.h"

#define private public
#include "views/dfmaddressbar.h"
DFM_USE_NAMESPACE

namespace  {
    class DFMAddressBarTest : public testing::Test
    {
    public:        
        virtual void SetUp() override
        {
            m_bar = new DFMAddressBar;
        }

        virtual void TearDown() override
        {
            delete m_bar;
        }
        DFMAddressBar *m_bar;
    };
}

TEST_F(DFMAddressBarTest,get_current_completer)
{
    ASSERT_NE(nullptr,m_bar);

    QCompleter *result = m_bar->completer();
    EXPECT_EQ(result, m_bar->urlCompleter);
}

TEST_F(DFMAddressBarTest,get_placeholder_text)
{
    ASSERT_NE(nullptr,m_bar);

    QString result = m_bar->placeholderText();
    EXPECT_EQ(result, m_bar->m_placeholderText);
}

TEST_F(DFMAddressBarTest,set_current_url)
{
    ASSERT_NE(nullptr,m_bar);

    auto desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktop("file://" + desktopPath);
    m_bar->setCurrentUrl(desktop);

    EXPECT_EQ(desktop, m_bar->currentUrl);
}

TEST_F(DFMAddressBarTest,set_current_completer)
{
    ASSERT_NE(nullptr,m_bar);

    QCompleter *completer = m_bar->completer();
    m_bar->setCompleter(completer);
    EXPECT_EQ(m_bar->urlCompleter, completer);
}

TEST_F(DFMAddressBarTest,set_placeholder_text)
{
    ASSERT_NE(nullptr,m_bar);

    QString placeholderTest("test_ut");
    m_bar->setPlaceholderText(placeholderTest);
    EXPECT_EQ(m_bar->m_placeholderText, placeholderTest);
}

TEST_F(DFMAddressBarTest,begin_play_animation)
{
    ASSERT_NE(nullptr,m_bar);

    m_bar->playAnimation();
    ASSERT_NE(nullptr, m_bar->animationSpinner);
    bool result = m_bar->animationSpinner->isPlaying();
    EXPECT_TRUE(result);
}

TEST_F(DFMAddressBarTest,end_play_animation)
{
    ASSERT_NE(nullptr,m_bar);

    m_bar->stopAnimation();
    EXPECT_EQ(nullptr, m_bar->animationSpinner);
}

TEST_F(DFMAddressBarTest,set_no_show)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->hide();
    EXPECT_EQ(true, m_bar->isHidden());
}

TEST_F(DFMAddressBarTest,call_init_ui)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->initUI();
    EXPECT_NE(nullptr, m_bar->indicator);
    EXPECT_NE(nullptr, m_bar->completerView);
    EXPECT_NE(nullptr, m_bar->animation);
}

TEST_F(DFMAddressBarTest,call_init_connections)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->initConnections();
}

TEST_F(DFMAddressBarTest,set_in_dicator)
{
    ASSERT_NE(nullptr,m_bar);
    DFMAddressBar::IndicatorType type = DFMAddressBar::Search;
    m_bar->setIndicator(type);
    EXPECT_EQ(m_bar->indicatorType, type);

    type = DFMAddressBar::JumpTo;
    m_bar->setIndicator(type);
    EXPECT_EQ(m_bar->indicatorType, type);
}

TEST_F(DFMAddressBarTest,update_indicator_icon)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->updateIndicatorIcon();
    EXPECT_NE(nullptr, m_bar->indicator);
}

TEST_F(DFMAddressBarTest,call_do_complete)
{
    ASSERT_NE(nullptr,m_bar);

    m_bar->doComplete();
    EXPECT_NE(nullptr, m_bar->completerView);
}

TEST_F(DFMAddressBarTest,clear_completer_model)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->clearCompleterModel();
    EXPECT_FALSE(m_bar->isHistoryInCompleterModel);
}

TEST_F(DFMAddressBarTest,update_completion_state)
{
    ASSERT_NE(nullptr,m_bar);

}

TEST_F(DFMAddressBarTest,append_complete_model)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(m_bar));

    QStringList stringList;
    stringList << QString("test1") << QString("test2");
    m_bar->appendToCompleterModel(stringList);
    EXPECT_GE(m_bar->completerModel.rowCount(), 2);
}
