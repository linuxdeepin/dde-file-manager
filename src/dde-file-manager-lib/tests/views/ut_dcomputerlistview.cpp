#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#define protected public

#include "views/dcompleterlistview.h"


DFM_USE_NAMESPACE
namespace  {
    class TestDCompleterListView : public testing::Test
    {
    public:
        QSharedPointer<DCompleterListView> m_completer;

        virtual void SetUp() override
        {
            m_completer = QSharedPointer<DCompleterListView>(new DCompleterListView());
        }

        virtual void TearDown() override
        {
        }
    };
}

TEST_F(TestDCompleterListView, can_show_hide)
{
    m_completer->showMe();
    bool isHidden = m_completer->isHidden();
    EXPECT_FALSE(isHidden);

    m_completer->hideMe();
    isHidden = m_completer->isHidden();
    EXPECT_TRUE(isHidden);
}

TEST_F(TestDCompleterListView, tst_current_changed)
{
    static bool currentChanged = false;
    QObject::connect(m_completer.data(), &DCompleterListView::listCurrentChanged, [](){
        currentChanged = true;
    });

    QModelIndex current, previous;
    m_completer->currentChanged(current, previous);

    EXPECT_TRUE(currentChanged);
}

TEST_F(TestDCompleterListView, tst_selection_changed)
{
    static bool selectionChanged = false;
    QObject::connect(m_completer.data(), &DCompleterListView::listSelectionChanged, [](){
        selectionChanged = true;
    });

    QItemSelection selected, deselected;
    m_completer->selectionChanged(selected, deselected);

    EXPECT_TRUE(selectionChanged);
}

TEST_F(TestDCompleterListView, tst_press_event)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_completer->keyPressEvent(&event);
}



