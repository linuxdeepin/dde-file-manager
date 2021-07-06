/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include <gtest/gtest.h>
#include "stub.h"
#include "stubext.h"
#include "testhelper.h"
#include "desktopinfo.h"
#include "dfmglobal.h"
#include "fileoperations/filejob.h"
#include "disomaster.h"

#include <QDialog>
#include <QFile>

using namespace DISOMasterNS;

#define private public
#include "dialogs/burnoptdialog.h"
#include "dialogs/burnoptdialog_p.h"

namespace  {
    class TestBurnOptDialog : public testing::Test{
    public:
        void SetUp() override
        {
            dlg = new BurnOptDialog("/dev/sr999");
            std::cout << "start TestBurnOptDialog";
        }
        void TearDown() override
        {
            delete dlg;
            dlg = nullptr;
            std::cout << "end TestBurnOptDialog";
        }
    public:
        BurnOptDialog *dlg;
    };
}

TEST_F(TestBurnOptDialog, tstConstructWithWayland) {
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(DFMGlobal, isWayLand), []{ return true; });
    stext.set_lamda(&DISOMasterNS::DISOMaster::getDevicePropertyCached, []{
        DISOMasterNS::DeviceProperty dp;
        dp.writespeed << "16x";
        return dp;
    });
    BurnOptDialog lDlg("/dev/srnnn");
}

TEST_F(TestBurnOptDialog, tstInnerSlot) {
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), []{ return QDialog::Rejected; });

    dlg->d_ptr->cb_checkdisc->setChecked(true);
    dlg->d_ptr->cb_eject->setChecked(true);
    dlg->d_ptr->cb_donotclose->setChecked(true);

    dlg->buttonClicked(0, "tst");
    TestHelper::runInLoop([]{;}, 300);

    bool (QFile::*exists_ori)() const = &QFile::exists;
    stext.set_lamda(exists_ori, []{ return true; });
    dlg->buttonClicked(0, "tst");
    TestHelper::runInLoop([]{;}, 300);

    int idx = 0;
    stext.set_lamda(ADDR(QComboBox, currentIndex), [&idx]{ return idx; });
    dlg->buttonClicked(0, "tst");
    TestHelper::runInLoop([]{;}, 300);

    idx = 1;
    dlg->buttonClicked(0, "tst");
    TestHelper::runInLoop([]{;}, 300);

    idx = 2;
    dlg->buttonClicked(0, "tst");
    TestHelper::runInLoop([]{;}, 300);

    idx = 3;
    dlg->buttonClicked(0, "tst");
    TestHelper::runInLoop([]{;}, 300);

    dlg->d_ptr->le_volname->setText("");
    dlg->buttonClicked(0, "tst");
    TestHelper::runInLoop([]{;}, 300);

    dlg->d_ptr->le_volname->setText("111");
    dlg->buttonClicked(0, "tst");
    TestHelper::runInLoop([]{;}, 300);

    stext.set_lamda(ADDR(FileJob, doISOBurn), []{ return; });
    stext.set_lamda(ADDR(FileJob, doISOImageBurn), []{ return; });
    dlg->buttonClicked(1, "tst");
    TestHelper::runInLoop([]{;}, 300);

    dlg->setISOImage(DUrl("file:///HelloUrl"));
    dlg->buttonClicked(1, "tst");
    TestHelper::runInLoop([]{;}, 300);

    dlg->setJobWindowId(idx);

    stext.set_lamda(ADDR(DesktopInfo, waylandDectected), []{ return true; });
    dlg->setDefaultVolName("hello iso");
}

TEST_F(TestBurnOptDialog, tstUDVersion) {
    EXPECT_TRUE(BurnOptDialog::isSupportedUDVersion("1.02"));
}

TEST_F(TestBurnOptDialog, tstUDMedia) {
    EXPECT_TRUE(BurnOptDialog::isSupportedUDMedium(5));
}
