#include "app/define.h"
#include "interfaces/drootfilemanager.h"
#include "dabstractfilewatcher.h"
#include "dfileservices.h"
#include <gtest/gtest.h>
#include <QTimer>
#include <QIcon>

#include "stub.h"
#include "stubext.h"
#include "testhelper.h"

#define private public
#define protected public
#include "models/computermodel.h"


namespace {
class TestComputerModel : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestComputerModel\n";
        model = new ComputerModel;
    }

    void TearDown() override
    {
        std::cout << "end TestComputerModel\n";
        delete model;
    }

public:
    ComputerModel *model;
};
} // namespace

TEST_F(TestComputerModel, tstConstructors)
{
    QEventLoop loop;
    QTimer t;
    t.setInterval(2000);
    QObject::connect(&t, &QTimer::timeout, &loop, [&loop]{
        loop.exit();
    });

    DRootFileManager::instance()->startQuryRootFile();
    t.start();
    loop.exec();

    emit DRootFileManager::instance()->hideSystemPartition();
    t.start();
    loop.exec();

    emit DRootFileManager::instance()->rootFileWather()->subfileCreated(DUrl("file:///home"));
    t.start();
    loop.exec();

    ComputerModel tmp;
}

TEST_F(TestComputerModel, tstParent)
{
    auto idx = model->parent(QModelIndex());
    EXPECT_FALSE(idx.isValid());
}

TEST_F(TestComputerModel, tstRowNColCount)
{
    auto num = model->rowCount();
    EXPECT_FALSE(num > 0);
    num = model->columnCount();
    EXPECT_EQ(1, num);
    num = model->itemCount();
    EXPECT_FALSE(num > 0);
}

TEST_F(TestComputerModel, tstIndex)
{
    int row = model->rowCount() - 1;
    auto idx = model->index(row, 0);
    EXPECT_FALSE(idx.isValid());
}

TEST_F(TestComputerModel, tstData)
{
    QEventLoop loop;
    QTimer t;
    t.setInterval(2000);
    QObject::connect(&t, &QTimer::timeout, &loop, [&loop]{
        loop.exit();
    });

    DRootFileManager::instance()->startQuryRootFile();
    t.start();
    loop.exec();

    auto idx = model->index(model->rowCount() - 1, 0);
    auto val = model->data(idx, Qt::DisplayRole);
    EXPECT_TRUE(!val.toString().isEmpty());

    val = model->data(idx, Qt::DecorationRole);
    EXPECT_FALSE(val.value<QIcon>().isNull());

    val = model->data(idx, ComputerModel::IconNameRole);
    EXPECT_TRUE(!val.toString().isEmpty());

    val = model->data(idx, ComputerModel::FileSystemRole);
    EXPECT_TRUE(val.toString().isEmpty());

    val = model->data(idx, ComputerModel::SizeInUseRole);
    EXPECT_TRUE(val.toInt() == 0);

    val = model->data(idx, ComputerModel::SizeTotalRole);
    EXPECT_TRUE(val.toInt() == 0);

    val = model->data(idx, ComputerModel::ICategoryRole);
    EXPECT_TRUE(val.toInt() <= 4);

    val = model->data(idx, ComputerModel::OpenUrlRole);
    EXPECT_TRUE(val.value<DUrl>().isValid());

    val = model->data(idx, ComputerModel::MountOpenUrlRole);
    EXPECT_TRUE(val.value<DUrl>().isValid());

    val = model->data(idx, ComputerModel::ActionVectorRole);
    EXPECT_FALSE(val.value<QVector<MenuAction>>().count() > 0);

    val = model->data(idx, ComputerModel::DFMRootUrlRole);
    EXPECT_TRUE(val.value<DUrl>().isValid());

    val = model->data(idx, ComputerModel::VolumeTagRole);
    EXPECT_FALSE(val.toString().startsWith("/dev"));

    val = model->data(idx, ComputerModel::ProgressRole);
    EXPECT_TRUE(val.toInt() <= 1 && val.toInt() >= 0);

    val = model->data(idx, ComputerModel::SizeRole);
    EXPECT_TRUE(val.toInt() <= 1 && val.toInt() >= 0);

    val = model->data(idx, ComputerModel::SchemeRole);
    EXPECT_TRUE(!val.toString().isEmpty());

    val = model->data(idx, ComputerModel::DiscUUIDRole);
    EXPECT_FALSE(!val.toString().isEmpty());

    val = model->data(idx, ComputerModel::DiscOpticalRole);
    EXPECT_FALSE(val.toBool());

    val = model->data(idx, Qt::UserRole);
    EXPECT_TRUE(val.isNull());
}

TEST_F(TestComputerModel, tstGetRootFile)
{
    QEventLoop loop;
    QTimer t;
    t.setInterval(2000);
    QObject::connect(&t, &QTimer::timeout, &loop, [&loop]{
        loop.exit();
    });

    DRootFileManager::instance()->startQuryRootFile();
    t.start();
    loop.exec();
    model->getRootFile();
}

TEST_F(TestComputerModel, tstSetDataNFlags)
{
    QEventLoop loop;
    QTimer t;
    t.setInterval(2000);
    QObject::connect(&t, &QTimer::timeout, &loop, [&loop]{
        loop.exit();
    });

    DRootFileManager::instance()->startQuryRootFile();
    t.start();
    loop.exec();

    auto idx = model->index(0, 0);
    auto rst = model->setData(idx, Qt::EditRole);
    EXPECT_FALSE(rst);

    auto flgs = model->flags(idx);
    EXPECT_TRUE(flgs & Qt::ItemFlag::ItemNeverHasChildren);
}

TEST_F(TestComputerModel, tstOnGetRootFile)
{
    DAbstractFileInfoPointer p;
    model->onGetRootFile(p);
    p = fileService->createFileInfo(nullptr, DUrl("file:///home"));
    model->onGetRootFile(p);
}

TEST_F(TestComputerModel, tstOnOpticalChanged)
{
    model->onOpticalChanged();
    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop]{
        loop.exit();
    });
    loop.exec();
}

TEST_F(TestComputerModel, tstLambdaSlots)
{
    model->m_watcher->fileAttributeChanged(DUrl("file:///home"));
    QEventLoop loop;
    QTimer::singleShot(200, nullptr, [&loop]{
        loop.exit();
    });
    loop.exec();

    DRootFileManager::instance()->queryRootFileFinsh();
    TestHelper::runInLoop([]{}, 200);

    model->m_watcher->fileAttributeChanged(DUrl("dfmroot:///Desktop.userdir"));
    TestHelper::runInLoop([]{}, 200);

    stub_ext::StubExt st;
    st.set_lamda(ADDR(ComputerModel, findItem), []{ return -1; });
    model->m_watcher->subfileCreated(DUrl("dfmroot:///sda.localdisk"));
    TestHelper::runInLoop([]{}, 200);
}
