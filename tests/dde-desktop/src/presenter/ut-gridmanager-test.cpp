#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QScopedPointer>
#include <QScreen>

#define private public
#define protected public

#include "../dde-desktop/presenter/gridmanager.h"
#include "../dde-desktop/presenter/gridmanager.cpp"
#include "../dde-desktop/view/canvasgridview.h"
#include "../dde-desktop/view/canvasviewmanager.h"
using namespace testing;

namespace  {
        class GridManagerTest : public Test {
        public:
            GridManagerTest() : Test() {

            }

            void SetUp() override {
                Config::instance()->setConfig(Config::groupGeneral, Config::keyAutoMerge, true);
                m_grid = GridManager::instance();
                for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
                    if (1 == tpCanvas->screenNum()) {
                        m_canvasGridView = tpCanvas.data();
                        break;
                    }
                }
                qApp->processEvents();
                m_canvasGridView->selectAll();
                qApp->processEvents();
            }

            void TearDown() override {
                delete m_canvasGridView;
            }

            GridManager* m_grid{nullptr};
            QScopedPointer<CanvasViewManager> m_cvmgr{new CanvasViewManager(new BackgroundManager())};
            CanvasGridView *m_canvasGridView{nullptr};

        };
}

TEST_F(GridManagerTest, test_remove)
{
    m_canvasGridView->selectAll();
    QString filepath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    bool ret;
    DUrlList ulist = m_canvasGridView->selectedUrls();
    QString string;
    if (ulist.size()) {
        string = ulist[0].toString();
    }
    else {
        QFile fd;
        string = filepath + '/' + "test.txt";
        fd.setFileName(string);
        if (!fd.exists()) {
            fd.open(QIODevice::ReadWrite|QIODevice::Text);
            fd.close();
        }
    }
    QMap<QString, QPoint> map;

    QPoint point = m_grid->position(m_canvasGridView->m_screenNum, string);
    m_grid->d->m_itemGrids.remove(m_canvasGridView->m_screenNum);
    ret = m_grid->d->remove(m_canvasGridView->m_screenNum, point, string);
    if (m_grid->d->m_itemGrids.contains(m_canvasGridView->m_screenNum) && !m_grid->d->m_itemGrids.value(m_canvasGridView->m_screenNum).contains(string)) {
        EXPECT_FALSE(ret);
    }

    point = m_grid->position(m_canvasGridView->m_screenNum, string);
    map[string] = point;
    m_grid->d->m_itemGrids.insert(m_canvasGridView->m_screenNum, map);

    m_grid->d->m_cellStatus.remove(m_canvasGridView->m_screenNum);
    ret = m_grid->remove(m_canvasGridView->m_screenNum, point, string);

    if (!m_grid->d->m_cellStatus.contains(m_canvasGridView->m_screenNum)) {
        EXPECT_FALSE(ret);
    }

}

TEST_F(GridManagerTest, test_rangeitems)
{
    m_canvasGridView->selectAll();

    DUrlList urllist = m_canvasGridView->selectedUrls();
    QStringList strlist;
    for (auto str : urllist) strlist << str.toString();
    auto item = m_grid->d->m_itemGrids.value(m_canvasGridView->m_screenNum);
    QPoint point;
    QMap<QString, QPoint> mp;

    for (auto str : strlist) {
        point = m_grid->position(m_canvasGridView->m_screenNum, str);
        mp[str] = point;
        m_grid->d->m_itemGrids.insert(m_canvasGridView->m_screenNum, mp);
    }

    strlist << QString("test");
    strlist = m_grid->d->rangeItems(m_canvasGridView->m_screenNum, strlist);

    bool issort = true;
    int Mn = INT_MIN;

    foreach (auto item, strlist) {
        if (m_grid->d->m_itemGrids.contains(m_canvasGridView->m_screenNum) && m_grid->d->m_itemGrids.value(m_canvasGridView->m_screenNum).contains(item)) {
            if (m_grid->d->m_itemGrids.value(m_canvasGridView->m_screenNum).value(item).x() < Mn) {
                issort = false;
            }
            Mn = m_grid->d->m_itemGrids.value(m_canvasGridView->m_screenNum).value(item).x();
         }
    }

    EXPECT_TRUE(issort);
}

namespace {

void waitDataEvent(CanvasGridView *view)
{
    if (!view)
        return;

    auto model = view->model();
    if (!model)
        return;

    QEventLoop loop;
    QObject::connect(model, &DFileSystemModel::sigJobFinished, &loop,&QEventLoop::quit,Qt::QueuedConnection);
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    view->delayCustom(0);
}
}
#define waitData(view) waitDataEvent(view); \
ASSERT_EQ(m_canvasGridView->model()->rowCount(), GridManager::instance()->allItems().size());

TEST_F(GridManagerTest, test_currentscreenmove)
{
    waitData(m_canvasGridView);
    m_canvasGridView->setIconByLevel(1);

    QList<DUrl> list;
    bool movestatus;
    QString url = m_grid->firstItemId(m_canvasGridView->m_screenNum);
    DUrl temp = DUrl(url);
    QPoint fpoint =  m_grid->position(m_canvasGridView->m_screenNum, url);
    list << temp;

    m_canvasGridView->selectAll();

    DUrlList urllist = m_canvasGridView->selectedUrls();
    ASSERT_FALSE(urllist.isEmpty());
    QStringList strlist;
    int emptyCount = m_grid->emptyPostionCount(m_canvasGridView->m_screenNum);
    ASSERT_GT(emptyCount, 0);
    for (int i = 0; i < emptyCount && i < urllist.size(); ++i)
        strlist << urllist[i].toString();

    movestatus = m_grid->move(m_canvasGridView->m_screenNum, strlist, url, fpoint.x() + 1, fpoint.y() + 1);
    emit m_grid->d->m_desktopSettings->valueChanged("desktop-computer", "1");

    m_grid->d->reAutoArrage();
    EXPECT_TRUE(movestatus);
    m_grid->move(m_canvasGridView->m_screenNum, strlist, url, fpoint.x(), fpoint.y());
}

TEST_F(GridManagerTest, test_forwardfindempty)
{
    QList<DUrl> list;
    QString url = m_grid->firstItemId(m_canvasGridView->m_screenNum);
    DUrl temp = DUrl(url);
    QPoint fpoint =  m_grid->d->m_itemGrids.value(m_canvasGridView->m_screenNum).value(url);
    list << temp;
    QPair<int, QPoint> empty;
    QPair<int, QPoint> emptypoint = m_grid->forwardFindEmpty(m_canvasGridView->m_screenNum, fpoint);
    if (m_grid->emptyPostionCount(m_canvasGridView->m_screenNum)) {
        EXPECT_NE(emptypoint, empty);
    }
    else {
        EXPECT_EQ(emptypoint, empty);
    }
}

TEST_F(GridManagerTest, test_sortmaindesktopfile)
{
    m_canvasGridView->selectAll();
    QStringList strlist;
    DUrlList ulist;
    int size;
    ulist = m_canvasGridView->selectedUrls();
    m_grid->d->readProfiles();
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);
    QDir dir(desktopUrl.toString());
    strlist << dir.filePath("dde-home.desktop") << dir.filePath("dde-trash.desktop");
    size = strlist.size();

    m_grid->sortMainDesktopFile(strlist, DFileSystemModel::FileMimeTypeRole, Qt::AscendingOrder);
    EXPECT_TRUE(strlist[0] == dir.filePath("dde-trash.desktop"));

    m_grid->sortMainDesktopFile(strlist, DFileSystemModel::FileMimeTypeRole, Qt::DescendingOrder);
    EXPECT_TRUE(strlist.last() == dir.filePath("dde-trash.desktop"));
    EXPECT_EQ(strlist.size(), size);

}

TEST_F(GridManagerTest, test_takeemptypos)
{
    m_grid->d->m_cellStatus.insert(m_canvasGridView->screenNum(), {1,1,1});
    QPair<int, QPoint> temp;
    temp = m_grid->d->takeEmptyPos();
    QPair<int, QPoint> compare;

    compare.first = m_grid->d->screenCode().last();
    compare.second = m_grid->d->overlapPos(compare.first);
    EXPECT_EQ(compare.first, temp.first);
}

TEST_F(GridManagerTest, test_add)
{
    bool ret;
    ret = m_grid->d->add(m_canvasGridView->m_screenNum, QPoint(), "");
    EXPECT_EQ(ret, false);

    m_canvasGridView->selectAll();
    m_grid->d->updateProfiles();

    DUrlList ulist =  m_canvasGridView->selectedUrls();
    QStringList strlist;
    QPoint point;
    QMap<QString, QPoint> mapitem;
    QMap<QPoint, QString> mapgrid;
    for (auto str : ulist) strlist << str.toString();

    for (auto str : strlist) {
        point = m_grid->position(m_canvasGridView->m_screenNum, str);
        mapitem[str] = point;
        mapgrid[point] = str;
        m_grid->d->m_itemGrids.insert(m_canvasGridView->m_screenNum, mapitem);
        m_grid->d->m_gridItems.insert(m_canvasGridView->m_screenNum, mapgrid);
        ret = m_grid->d->add(m_canvasGridView->m_screenNum, point, str);
        m_grid->d->m_itemGrids.clear();
        m_grid->dump();
        if (m_grid->d->m_itemGrids.value(m_canvasGridView->m_screenNum).contains(str)) {
            EXPECT_FALSE(ret);
        }
        ret = m_grid->d->add(m_canvasGridView->m_screenNum, point, str);
        if (point != m_grid->d->overlapPos(m_canvasGridView->m_screenNum)) {
             EXPECT_FALSE(ret);
        }

        break;
    }
}

TEST_F(GridManagerTest, test_addtooverlap)
{
    QString test("test");
    QMap<QString, QPoint> map;
    int result;
    result = m_grid->addToOverlap(test);
    EXPECT_EQ(result, 0);

    m_grid->d->m_overlapItems << test;
    result = m_grid->addToOverlap(test);
    EXPECT_EQ(result, 1);

    map.insert(test, QPoint());
    m_grid->d->m_itemGrids.insert(m_canvasGridView->m_screenNum, map);
    m_grid->d->m_overlapItems.clear();
    result = m_grid->addToOverlap(test);
    EXPECT_EQ(-1, result);
}

TEST_F(GridManagerTest, test_popoverlap)
{
    m_canvasGridView->selectAll();

    DUrlList ulist = m_canvasGridView->selectedUrls();
    for (auto str : ulist) m_grid->d->m_overlapItems.push_back(str.toString());
    m_grid->popOverlap();
    if (ulist.size() > 0) {
        EXPECT_TRUE(m_grid->d->m_overlapItems.size());
    }
    int count = m_grid->gridCount();
    EXPECT_TRUE(count > 0);

    bool mode;
    m_grid->setDisplayMode(false);
    EXPECT_EQ(false, m_grid->d->m_bSingleMode);

    m_grid->setDisplayMode(true);
    EXPECT_EQ(true, m_grid->d->m_bSingleMode);
}

TEST_F(GridManagerTest, test_differentscreenmove)
{
    QList<DUrl> list;
    QString url = m_grid->firstItemId(m_canvasGridView->m_screenNum);
    DUrl temp = DUrl(url);
    bool movestatus;
    list << temp;
    m_canvasGridView->selectAll();
    QPoint point = m_grid->position(m_canvasGridView->m_screenNum, url);

    DUrlList urllist = m_canvasGridView->selectedUrls();
    QStringList strlist;
    for (auto str : urllist) strlist << str.toString();
    movestatus = m_grid->move(m_canvasGridView->m_screenNum,m_canvasGridView->m_screenNum, strlist, url, point.x() + 1, point.y() + 1);


    EXPECT_TRUE(movestatus);
    m_grid->move(m_canvasGridView->m_screenNum,m_canvasGridView->m_screenNum, strlist, url, point.x(), point.y());
}
TEST_F(GridManagerTest, test_getemptyposmorearg)
{
   QPair<int, QPoint> emptypos;
   QPoint point;
   m_grid->d->m_screenFullStatus[m_canvasGridView->m_screenNum] = true;
   bool judge = m_grid->d->getEmptyPos(m_canvasGridView->m_screenNum, true, point);
   EXPECT_EQ(QPoint(), point);
   EXPECT_FALSE(judge);

   m_grid->d->m_screenFullStatus[m_canvasGridView->m_screenNum] = false;
   m_grid->d->m_cellStatus[m_canvasGridView->m_screenNum] = {0};
   judge = m_grid->d->getEmptyPos(m_canvasGridView->m_screenNum, true, point);
   EXPECT_TRUE(judge);
   judge = m_grid->d->getEmptyPos(m_canvasGridView->m_screenNum, false, point);
   EXPECT_TRUE(judge);
}

