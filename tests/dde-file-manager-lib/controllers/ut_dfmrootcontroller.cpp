// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "controllers/dfmrootcontroller.h"
#include "ddialog.h"
#include "dfmevent.h"
#include "app/define.h"
#include "stub.h"
#include "interfaces/drootfilemanager.h"
#include "models/dfmrootfileinfo.h"
#include "dfileservices.h"
#include "testhelper.h"

namespace  {
    class DFMRootControllerTest : public testing::Test
    {
    public:
        typedef int(*fptr)(Dtk::Widget::DDialog*);
        fptr pDDialogExec = (fptr)(&Dtk::Widget::DDialog::exec);
        Stub stub;

        virtual void SetUp() override
        {
            int (*stub_DDialog_exec)(void) = [](void)->int{return Dtk::Widget::DDialog::Accepted;};
            stub.set(pDDialogExec, stub_DDialog_exec);

            controller = new DFMRootController();
        }

        virtual void TearDown() override
        {
            delete controller;
            controller = nullptr;
        }

        DFMRootController *controller;
    };
}

TEST_F(DFMRootControllerTest, rename_file)
{
    TestHelper::runInLoop([](){});
    DUrl from;
    from.setScheme(DFMROOT_SCHEME);
    from.setPath("/downloads.userdir");

    DUrl to;
    to.setScheme(DFMROOT_SCHEME);
    to.setPath("/downloads.tst_userdir");

    bool success = controller->renameFile(dMakeEventPointer<DFMRenameEvent>(nullptr, from, to));
    EXPECT_FALSE(success);
}

TEST_F(DFMRootControllerTest, get_children)
{
    DUrl url;
    url.setScheme(DFMROOT_SCHEME);
    url.setPath("/");

    QStringList nameFilters;
    QDir::Filters filters;
    auto event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr,
                                                         url,
                                                         nameFilters,
                                                         filters);
    QList<DAbstractFileInfoPointer> list = controller->getChildren(event);
    EXPECT_TRUE(!list.empty());
}

TEST_F(DFMRootControllerTest, create_file_info)
{
    DUrl url;
    url.setScheme(DFMROOT_SCHEME);
    url.setPath("/");
    DAbstractFileInfoPointer fi = controller->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, url));
    EXPECT_TRUE(fi != nullptr);
}

TEST_F(DFMRootControllerTest, create_file_watcher)
{
    DUrl url;
    url.setScheme(DFMROOT_SCHEME);
    url.setPath("/");
    DAbstractFileWatcher *watcher = controller->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, url));
    EXPECT_TRUE(watcher != nullptr);
    if (watcher)
        delete watcher;

//    这段代码目前会阻塞CI原因未细查, 暂时先屏蔽
//    watcher->startWatcher();
//    system("touch /tmp/newfile");
//    system("echo 'test' >> /tmp/newfile");
//    system("mv /tmp/newfile /tmp/newfile2");
//    system("rm /tmp/newfile2");
//    for (int i=0; i<4; i++) {
//        qApp->processEvents();
//        sleep(1);
//    }
//    watcher->stopWatcher();
}

TEST_F(DFMRootControllerTest, load_disk_info)
{
    QString json;
    controller->loadDiskInfo(json);
}

TEST_F(DFMRootControllerTest, setLocalDiskAlias)
{
    EXPECT_FALSE(controller->setLocalDiskAlias(nullptr, ""));
    QList<DAbstractFileInfoPointer> ch = rootFileManager->getRootFile();

    for (auto c : ch) {
        if (static_cast<DFMRootFileInfo::ItemType>(c->fileType()) == DFMRootFileInfo::UDisksRoot) {
            auto rfi = dynamic_cast<DFMRootFileInfo *>(c.data());
            if (!rfi)
                continue;

            bool r = controller->setLocalDiskAlias(rfi, "test");
            EXPECT_TRUE(r);
            EXPECT_STREQ(rfi->udisksDispalyAlias().toStdString().c_str(), "test");

            r = controller->setLocalDiskAlias(dynamic_cast<DFMRootFileInfo *>(c.data()), "");
            EXPECT_TRUE(r);
            EXPECT_STREQ(rfi->udisksDispalyAlias().toStdString().c_str(), "");
        }
    }
}
