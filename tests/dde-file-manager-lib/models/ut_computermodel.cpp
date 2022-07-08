/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include "ddialog.h"

#define private public
#define protected public
#include "models/computermodel.h"

DWIDGET_USE_NAMESPACE
namespace {
class TestComputerModel : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestComputerModel\n";
        model = QSharedPointer<ComputerModel>( new ComputerModel );
        typedef int(*fptr)(QDialog*);
        fptr pQDialogExec = (fptr)(&QDialog::exec);
        fptr pDDialogExec = (fptr)(&DDialog::exec);
        int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
        stl.set(pQDialogExec, stub_DDialog_exec);
        stl.set(pDDialogExec, stub_DDialog_exec);
    }

    void TearDown() override
    {
        std::cout << "end TestComputerModel\n";
    }

public:
    QSharedPointer<ComputerModel> model;
    Stub stl;
};
} // namespace

TEST_F(TestComputerModel, tstConstructors)
{
    Stub stl;
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);

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
    EXPECT_TRUE(num >= 0);
    num = model->columnCount();
    EXPECT_TRUE(num > 0);
}

TEST_F(TestComputerModel, tstIndex)
{
    int row = model->rowCount() - 1;
    if(row >= 0) {
        auto idx = model->index(row, 0);
        EXPECT_TRUE(idx.isValid());
    }
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
    EXPECT_NO_FATAL_FAILURE(!val.toString().isEmpty());

    val = model->data(idx, Qt::DecorationRole);
    EXPECT_NO_FATAL_FAILURE(val.value<QIcon>().isNull());

    val = model->data(idx, ComputerModel::IconNameRole);
    EXPECT_NO_FATAL_FAILURE(val.toString().isEmpty());

    val = model->data(idx, ComputerModel::FileSystemRole);
    EXPECT_NO_FATAL_FAILURE(val.toString().isEmpty());

    val = model->data(idx, ComputerModel::SizeInUseRole);
    EXPECT_NO_FATAL_FAILURE(val.toInt() == 0);

    val = model->data(idx, ComputerModel::SizeTotalRole);
    EXPECT_NO_FATAL_FAILURE(val.toInt() == 0);

    val = model->data(idx, ComputerModel::ICategoryRole);
    EXPECT_NO_FATAL_FAILURE(val.toInt() <= 4);

    val = model->data(idx, ComputerModel::OpenUrlRole);
    EXPECT_NO_FATAL_FAILURE(val.value<DUrl>().isValid());

    val = model->data(idx, ComputerModel::MountOpenUrlRole);
    EXPECT_NO_FATAL_FAILURE(val.value<DUrl>().isValid());

    val = model->data(idx, ComputerModel::ActionVectorRole);
    EXPECT_NO_FATAL_FAILURE(val.value<QVector<MenuAction>>().count() > 0);

    val = model->data(idx, ComputerModel::DFMRootUrlRole);
    EXPECT_NO_FATAL_FAILURE(val.value<DUrl>().isValid());

    val = model->data(idx, ComputerModel::VolumeTagRole);
    EXPECT_NO_FATAL_FAILURE(val.toString().startsWith("/dev"));

    val = model->data(idx, ComputerModel::ProgressRole);
    EXPECT_NO_FATAL_FAILURE(val.toInt() <= 1 && val.toInt() >= 0);

    val = model->data(idx, ComputerModel::SizeRole);
    EXPECT_NO_FATAL_FAILURE(val.toInt() <= 1 && val.toInt() >= 0);

    val = model->data(idx, ComputerModel::SchemeRole);
    EXPECT_NO_FATAL_FAILURE(val.toString().isEmpty());

    val = model->data(idx, ComputerModel::DiscUUIDRole);
    EXPECT_NO_FATAL_FAILURE(!val.toString().isEmpty());

    val = model->data(idx, ComputerModel::DiscOpticalRole);
    EXPECT_NO_FATAL_FAILURE(val.toBool());

    val = model->data(idx, Qt::UserRole);
    EXPECT_NO_FATAL_FAILURE(val.isNull());
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
    TestHelper::runInLoop([]{}, 200);
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
