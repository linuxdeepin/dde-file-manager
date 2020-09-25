#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QAbstractItemModel>
#include <QMouseEvent>
#include "models/computermodel.h"
#include "views/computerview.h"

#include "views/computerviewitemdelegate.h"
#include "dfmevent.h"
#include "views/dfilemanagerwindow.h"

DFM_USE_NAMESPACE
namespace  {
    class TestComputerViewItemDelegate : public testing::Test
    {
    public:
        ComputerViewItemDelegate *m_computerViewItemDelegate;
        QSharedPointer<DFileManagerWindow> m_window;
        ComputerView *m_computerView;

        virtual void SetUp() override
        {
            m_window = QSharedPointer<DFileManagerWindow>(new DFileManagerWindow());
            m_window->cd(DUrl(COMPUTER_ROOT));
            m_computerView = dynamic_cast<ComputerView*>(m_window->getFileView());

            m_computerViewItemDelegate = dynamic_cast<ComputerViewItemDelegate*>(m_computerView->view()->itemDelegate());
        }

        virtual void TearDown() override
        {
        }
    };
}


TEST_F(TestComputerViewItemDelegate, tst_paint)
{
    QPainter painter;
    QStyleOptionViewItem option;
    ComputerModel model;
    QModelIndex index;
    model.setData(index, ComputerModelItemData::Category::cat_splitter, ComputerModel::DataRoles::ICategoryRole);
    m_computerViewItemDelegate->paint(&painter, option, index);

    model.setData(index, ComputerModelItemData::Category::cat_widget, ComputerModel::DataRoles::ICategoryRole);
    m_computerViewItemDelegate->paint(&painter, option, index);

    model.setData(index, ComputerModelItemData::Category::cat_user_directory, ComputerModel::DataRoles::ICategoryRole);
    m_computerViewItemDelegate->paint(&painter, option, index);
}

TEST_F(TestComputerViewItemDelegate, tst_sizeHint)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QSize size = m_computerViewItemDelegate->sizeHint(option, index);
    EXPECT_NE(0, size.width());
    EXPECT_NE(0, size.height());
}

TEST_F(TestComputerViewItemDelegate, tst_createEditor)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QWidget *widget = m_computerViewItemDelegate->createEditor(nullptr, option, index);
    EXPECT_NE(nullptr, widget);
}


TEST_F(TestComputerViewItemDelegate, tst_setEditorData)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QWidget *widget = m_computerViewItemDelegate->createEditor(nullptr, option, index);

    m_computerViewItemDelegate->setEditorData(widget, index);
}

TEST_F(TestComputerViewItemDelegate, tst_setModelData)
{
    //! somthing to do.
}

TEST_F(TestComputerViewItemDelegate, tst_updateEditorGeometry)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QWidget *widget = m_computerViewItemDelegate->createEditor(nullptr, option, index);

    m_computerViewItemDelegate->updateEditorGeometry(widget, option, index);
}
