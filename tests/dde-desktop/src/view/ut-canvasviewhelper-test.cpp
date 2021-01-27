#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QStandardPaths>
#include <QScreen>
#include <QTimer>

#define private public
#define protected public

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
        class CanvasViewHelperTest : public Test{
        public:
            CanvasViewHelperTest() : Test() {
                for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
                    if(1 == tpCanvas->screenNum()){
                        m_view = tpCanvas.data();
                        break;
                    }
                }
                //在桌面创建文件
                path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                path = path + '/' + "test.txt";
                QFile file(path);
                if (!file.exists())
                {
                    file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
                    file.close();
                }
            }
            void SetUp() override {
                m_canvas = new CanvasViewHelper(m_view);
            }

            void TearDown() override {
                delete m_canvas;
                delete m_view;
            }
            QString path;
            CanvasViewHelper* m_canvas = nullptr;
            CanvasGridView* m_view = nullptr;
            QScopedPointer<CanvasViewManager> m_cvmgr{new CanvasViewManager(new BackgroundManager())};
        };
}


TEST_F(CanvasViewHelperTest, test_onRequestSelectFiles)
{
    DUrl url(QUrl::fromLocalFile(path));
    QList<DUrl> list;
    qApp->processEvents();
    m_view->selectAll();
    qApp->processEvents();
    QEventLoop loop;
    QTimer::singleShot(100, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();
    for (auto url : m_view->selectedUrls()) list << url;
    list << url;
    m_canvas->onRequestSelectFiles(list);
    EXPECT_TRUE(m_view->isSelected(m_view->firstIndex()));
}

TEST_F(CanvasViewHelperTest, test_handleSelectEvent)
{
    DUrlList ulist;
    ulist << DUrl::fromLocalFile(path);
    DFMUrlListBaseEvent event(m_canvas, ulist);
    m_canvas->handleSelectEvent(event);
    DFMUrlListBaseEvent event1(m_view, ulist);
    m_canvas->handleSelectEvent(event1);
    QModelIndex index;
    for (auto url : ulist) {
        index = m_view->model()->index(url, 0);
        if( index.isValid()) {
            EXPECT_TRUE(m_view->isSelected(index));
        }
    }
}

//test
TEST_F(CanvasViewHelperTest, test_edit)
{
    DFMEvent event(m_canvas);
    Stub stu, stu1;
    static QString mypath = path;
    static bool judge = false;
    m_canvas->edit(event);

    DUrlList(*mylist)() = [](){
        DUrlList ulist;
        ulist << DUrl(QUrl::fromLocalFile(mypath));
        judge = true;
        return ulist;
    } ;
    stu.set(ADDR(DFMEvent, fileUrlList), mylist);
    m_canvas->edit(event);

    DUrlList(*mylist1)() = [](){
        DUrlList ulist;
        ulist << DUrl::fromLocalFile("");
        return ulist;
    } ;
    stu.reset(mylist1);
    m_canvas->edit(event);

    bool(*myjudge)() = [](){
        return true;
    };
    stu1.set(ADDR(DUrl, isEmpty), myjudge);
    m_canvas->edit(event);
    EXPECT_TRUE(judge);
}

TEST_F(CanvasViewHelperTest, test_selecturls)
{
    qApp->processEvents();
    m_view->selectAll();
    qApp->processEvents();

    DUrlList ulist = m_canvas->selectedUrls();
    EXPECT_TRUE(ulist.size() > 0);
}

TEST_F(CanvasViewHelperTest, test_initStyleOption)
{
    qApp->processEvents();
    m_view->selectAll();
    qApp->processEvents();

    stub_ext::StubExt tub;
    stub_ext::StubExt tub1;
    QStyleOptionViewItem* option = new QStyleOptionViewItem;
    tub.set_lamda(VADDR(DFileViewHelper, isTransparent), [](){return false;});
    tub1.set_lamda(VADDR(CanvasViewHelper, selectedIndexsCount), [](){return 5;});
    option->state = QStyle::State_HasFocus;
    option->showDecorationSelected = true;
    QModelIndex index = m_view->firstIndex();
    m_canvas->initStyleOption(option, index);

    tub.reset(VADDR(DFileViewHelper, isTransparent));
    tub.set_lamda(VADDR(DFileViewHelper, isTransparent), [](){return true;});
    m_canvas->itemDelegate();
    m_canvas->initStyleOption(option, m_view->firstIndex());

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
