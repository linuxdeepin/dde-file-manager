#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QStandardPaths>
#include <QScreen>
#include <QTimer>

#define private public
#define protected public

#include <view/private/canvasviewprivate.h>
#include "view/canvasviewhelper.h"
#include "view/canvasgridview.h"
#include "view/backgroundmanager.h"
#include "view/canvasviewmanager.h"
#include "durl.h"
#include "stub.h"
#include "stubext.h"
#include "dfileviewhelper.h"
#include "dfilesystemmodel.h"
using namespace testing;
namespace  {
class CanvasViewHelperTest : public Test {
public:
    CanvasViewHelperTest() : Test()
    {
        for (auto tpCanvas : m_cvmgr->m_canvasMap.values()) {
            if (1 == tpCanvas->screenNum()) {
                m_view = tpCanvas.data();
                m_canvasHelper = tpCanvas.data()->d->fileViewHelper;
                break;
            }
        }
        //在桌面创建文件
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        path = path + '/' + "test.txt";
        QFile file(path);
        if (!file.exists()) {
            file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
            file.close();
        }

        if (!m_view)
            return;

        auto model = m_view->model();
        if (!model)
            return;

        QEventLoop loop;
        QObject::connect(model, &DFileSystemModel::sigJobFinished, &loop,&QEventLoop::quit,Qt::QueuedConnection);
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        loop.exec();
    }
    void SetUp() override {
    }

    void TearDown() override {
    }
    QString path;
    CanvasViewHelper *m_canvasHelper = nullptr;
    CanvasGridView *m_view = nullptr;
    QScopedPointer<CanvasViewManager> m_cvmgr{new CanvasViewManager(new BackgroundManager())};
};
}

//TEST_F(CanvasViewHelperTest, test_onRequestSelectFiles)
//{
//    QList<DUrl> list;
//    stub_ext::StubExt stub;
//    bool isSelected = false;
//    stub.set_lamda(VADDR(CanvasViewHelper, select), [&isSelected]() {isSelected = true;});

//    m_canvasHelper->onRequestSelectFiles(list);

//    EXPECT_TRUE(isSelected);
//}

TEST_F(CanvasViewHelperTest, test_handleSelectEvent)
{
    stub_ext::StubExt stu;
    bool judge = false;
    typedef void (*fptr)(CanvasViewHelper*,const QList<DUrl> &list);
    fptr testSelect = (fptr)(&CanvasViewHelper::select);
    stu.set_lamda(testSelect, [&judge](){ judge = true; });

    DUrlList ulist;
    ulist << DUrl::fromLocalFile(path);
    QObject obj;
    DFMUrlListBaseEvent eventOne(&obj, ulist);
    m_canvasHelper->handleSelectEvent(eventOne);
    EXPECT_TRUE(!judge);

    DFMUrlListBaseEvent eventTwo(m_view, ulist);
    m_canvasHelper->handleSelectEvent(eventTwo);
    EXPECT_TRUE(judge);
}
#ifndef __arm__
//test
TEST_F(CanvasViewHelperTest, test_edit)
{
    DFMEvent event(m_canvasHelper);
    Stub stu, stu1;
    static QString mypath = path;
    static bool judge = false;
    m_canvasHelper->edit(event);

    stub_ext::StubExt stub;
    typedef bool (*fptr)(CanvasGridView*,const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *even);
    fptr testEdit = (fptr)&CanvasGridView::edit;
    stub.set_lamda(testEdit, []()->bool{return true;});

    DUrlList(*mylist)() = []() {
        DUrlList ulist;
        ulist << DUrl(QUrl::fromLocalFile(mypath));
        judge = true;
        return ulist;
    } ;
    stu.set(ADDR(DFMEvent, fileUrlList), mylist);
    m_canvasHelper->edit(event);

    DUrlList(*mylist1)() = []() {
        DUrlList ulist;
        ulist << DUrl::fromLocalFile("");
        return ulist;
    } ;
    stu.reset(mylist1);
    m_canvasHelper->edit(event);

    bool(*myjudge)() = []() {
        return true;
    };
    stu1.set(ADDR(DUrl, isEmpty), myjudge);
    m_canvasHelper->edit(event);
    EXPECT_TRUE(judge);
}
#endif
TEST_F(CanvasViewHelperTest, test_selecturls)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(CanvasGridView, selectedUrls), []() {return DUrlList{DUrl("test")};});
    DUrlList ulist = m_canvasHelper->selectedUrls();
    EXPECT_EQ(ulist.size(), 1);
}

TEST_F(CanvasViewHelperTest, test_initStyleOption)
{
    qApp->processEvents();
    m_view->selectAll();
    qApp->processEvents();

    stub_ext::StubExt tub;
    stub_ext::StubExt tub1;
    QStyleOptionViewItem *option = new QStyleOptionViewItem;
    tub.set_lamda(VADDR(DFileViewHelper, isTransparent), []() {return false;});
    tub1.set_lamda(VADDR(CanvasViewHelper, selectedIndexsCount), []() {return 5;});
    option->state = QStyle::State_HasFocus;
    option->showDecorationSelected = true;
    QModelIndex index = m_view->firstIndex();
    m_canvasHelper->initStyleOption(option, index);

    tub.reset(VADDR(DFileViewHelper, isTransparent));
    tub.set_lamda(VADDR(DFileViewHelper, isTransparent), []() {return true;});
    m_canvasHelper->itemDelegate();
    m_canvasHelper->initStyleOption(option, m_view->firstIndex());

    EXPECT_EQ(option->textElideMode, Qt::ElideLeft);

    delete  option;
}

TEST_F(CanvasViewHelperTest, delete_file)
{
    QFile file(path);
    if (file.exists()) {
        QProcess::execute("rm " + path);
    }
}
