#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#define protected public

#include "../../model/dfileselectionmodel.h"

#include <QEventLoop>
#include <QObject>
#include <QTimer>
#include <QItemSelectionModel>

TEST(DfileSelectionModelTest, dfile_selection_model)
{
    DFileSelectionModel dfile;
    DFileSelectionModel dfile1(nullptr, nullptr);
    EXPECT_EQ(true, dfile.m_timer.isSingleShot());
    EXPECT_TRUE(dfile1.m_timer.isSingleShot());
}

TEST(DfileSelectionModelTest, select)
{
    DFileSelectionModel dfile;
    QItemSelection item;
    dfile.select(item, QItemSelectionModel::SelectionFlags(QItemSelectionModel::Clear));
    EXPECT_EQ(true, dfile.m_selectedList.isEmpty());
    EXPECT_EQ(dfile.m_firstSelectedIndex, QModelIndex());
    EXPECT_EQ(dfile.m_lastSelectedIndex, QModelIndex());
    EXPECT_EQ(dfile.m_currentCommand, QItemSelectionModel::Clear);
    dfile.select(item, QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect));
    QTimer timer;
    timer.start(300);
    QEventLoop loop;
    QObject::connect(&dfile.m_timer, &QTimer::timeout, [&]{
        loop.exit();
    });
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
        EXPECT_TRUE(false);
        loop.exit();
    });
    loop.exec();
}

TEST(DfileSelectionModelTest, selected_indexes)
{
    DFileSelectionModel dfile;
    QItemSelection item;
    int count = dfile.m_selectedList.count();
    EXPECT_TRUE(dfile.m_selectedList.isEmpty());
    dfile.select(item, QItemSelectionModel::SelectionFlags(QItemSelectionModel::Clear));
    dfile.selectedIndexes();
    int ret = dfile.m_selectedList.count();
    EXPECT_EQ(ret, count);
}

TEST(DfileSelectionModelTest, selected_count)
{
    DFileSelectionModel dfile;
    QItemSelection item;
    dfile.select(item, QItemSelectionModel::SelectionFlags(QItemSelectionModel::Clear));
    dfile.selectedIndexes();
    int ret = dfile.m_selectedList.count();
    int result = dfile.selectedCount();
    EXPECT_EQ(ret, result);
}
