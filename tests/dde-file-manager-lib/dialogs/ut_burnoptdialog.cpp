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

    stext.set_lamda(ADDR(FileJob, doOpticalBurnByChildProcess), []{ return; });
    stext.set_lamda(ADDR(FileJob, doOpticalImageBurnByChildProcess), []{ return; });
    dlg->buttonClicked(1, "tst");
    TestHelper::runInLoop([]{;}, 300);

    dlg->setISOImage(DUrl("file:///HelloUrl"));
    dlg->buttonClicked(1, "tst");
    TestHelper::runInLoop([]{;}, 300);

    dlg->setJobWindowId(idx);

    stext.set_lamda(ADDR(DesktopInfo, waylandDectected), []{ return true; });
    dlg->setDefaultVolName("hello iso");
}

