#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QEventLoop>
#include <QObject>
#include <QTimer>
#include <QItemSelectionModel>
#include <QStandardPaths>
#include <QFile>
#include <QApplication>

#define private public
#define protected public

#include <dfilesystemmodel.h>

#include "model/dfileselectionmodel.h"
#include "durl.h"
#include "canvasgridview.h"
#include "canvasviewmanager.h"
#include "stubext.h"

TEST(DfileSelectionModelTest, dfile_selection_model)
{
    DFileSelectionModel dfile;
    DFileSelectionModel dfile1(nullptr, nullptr);
    EXPECT_EQ(true, dfile.m_timer.isSingleShot());
    EXPECT_TRUE(dfile1.m_timer.isSingleShot());
}

TEST(DfileSelectionModelTest, select)
{
    void(*foo_stub1)(void*) = [](void* obj){return;};
    stub_ext::StubExt stu;
    typedef int (*fptr)(QItemSelectionModel*, int);
    fptr A_foo = (fptr)((void(QItemSelectionModel::*)(const QItemSelection&, QItemSelectionModel::SelectionFlags))&QItemSelectionModel::select);
    Stub stub;
    stub.set(A_foo, foo_stub1);
    DFileSelectionModel dfile;
    QItemSelection item;
    dfile.select(item, QItemSelectionModel::SelectionFlags(QItemSelectionModel::Clear));
    EXPECT_EQ(true, dfile.m_selectedList.isEmpty());
    EXPECT_EQ(dfile.m_firstSelectedIndex, QModelIndex());
    EXPECT_EQ(dfile.m_lastSelectedIndex, QModelIndex());
    EXPECT_EQ(dfile.m_currentCommand, QItemSelectionModel::Clear);
    dfile.m_timer.start();
    dfile.select(item, QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect));
    dfile.select(item, QItemSelectionModel::SelectionFlags(QItemSelectionModel::NoUpdate | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect));
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


    QScopedPointer<CanvasViewManager> m_cvmgr{new CanvasViewManager(new BackgroundManager())};
    CanvasGridView *m_canvasGridView = nullptr;

    for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
        if(1 == tpCanvas->screenNum()){
            m_canvasGridView = tpCanvas.data();
            break;
        }
    }
    m_canvasGridView->selectAll();

    DUrlList ulist = m_canvasGridView->selectedUrls();
    if (ulist.size() < 0) {
        QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        path = path + '/' + "test.txt";
        QFile file(path);
        if (!file.exists()) {
            file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
            file.close();
            DUrl url(path);
            ulist << url;
        }
    }
    QItemSelection selection;
    for (auto url : ulist) {
        QItemSelectionRange item(m_canvasGridView->model()->index(url));
        selection << item;
    }
    dfile.m_selectedList.clear();
    int lenth = dfile.m_selectedList.size();
    dfile.select(selection, QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect));
    dfile.selectedIndexes();
    if (ulist.size() > 1) {
        EXPECT_TRUE(dfile.m_selectedList.size() > lenth);
    }
    else {
        EXPECT_TRUE(dfile.m_selectedList.size() == lenth);
    }

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

    QScopedPointer<CanvasViewManager> m_cvmgr{new CanvasViewManager(new BackgroundManager())};
    CanvasGridView *m_canvasGridView = nullptr;

    for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
        if(1 == tpCanvas->screenNum()){
            m_canvasGridView = tpCanvas.data();
            break;
        }
    }
    m_canvasGridView->selectAll();

    DUrlList ulist = m_canvasGridView->selectedUrls();
    if (ulist.size() < 0) {
        QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        path = path + '/' + "test.txt";
        QFile file(path);
        if (!file.exists()) {
            file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
            file.close();
        }
        DUrl url(path);
        ulist << url;
    }
    QItemSelection selection;
    for (auto url : ulist) {
        QItemSelectionRange item(m_canvasGridView->model()->index(url));
        selection << item;

    }
    dfile.m_currentCommand = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    int count = dfile.selectedCount();
    if (ulist.size()) {
        EXPECT_TRUE(count > 0);
    }
}

TEST(DfileSelectionModelTest, test_isselected)
{
    DFileSelectionModel dfile;
    QScopedPointer<CanvasViewManager> m_cvmgr{new CanvasViewManager(new BackgroundManager())};
    CanvasGridView *m_canvasGridView = nullptr;

    for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
        if(1 == tpCanvas->screenNum()){
            m_canvasGridView = tpCanvas.data();
            break;
        }
    }
    m_canvasGridView->selectAll();

    DUrlList ulist = m_canvasGridView->selectedUrls();
    DUrl url;
    if (ulist.size() < 0) {
        QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        path = path + '/' + "test.txt";
        QFile file(path);
        if (!file.exists()) {
            file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
            file.close();
        }
        url.setUrl(path);
        ulist << url;
    }
    QItemSelection selection;
    for (auto url : ulist) {
        QItemSelectionRange item(m_canvasGridView->model()->index(url));
        selection << item;

    }
    dfile.m_timer.start(50);
    dfile.select(selection, QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    dfile.select(selection, QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    dfile.m_currentCommand = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    dfile.m_selection = QItemSelection(QModelIndex(), QModelIndex());
    ulist << url;
    if (ulist.size())
        EXPECT_FALSE(dfile.isSelected(m_canvasGridView->model()->index(ulist[0])));
}

TEST(DfileSelectionModelTest, test_updateSelecteds)
{
    static bool isupdate = false;
    void(*foo_stub)(void*) = [](void* obj){isupdate = true; return;};
    DFileSelectionModel dfile;
    stub_ext::StubExt stu;
    typedef int (*fptr)(QItemSelectionModel*, int);
    fptr A_foo = (fptr)((void(QItemSelectionModel::*)(const QItemSelection&, QItemSelectionModel::SelectionFlags))&QItemSelectionModel::select);
    Stub stub;
    stub.set(A_foo, foo_stub);
    dfile.updateSelecteds();
    EXPECT_TRUE(isupdate);
}
