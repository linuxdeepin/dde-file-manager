// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gvfs/mountsecretdiskaskpassworddialog.h"

#include <gtest/gtest.h>

namespace {
class TestMountSecretDiskAskPasswordDialog: public testing::Test
{
public:
    MountSecretDiskAskPasswordDialog *m_dlg {nullptr};

    void SetUp() override
    {
        m_dlg = new MountSecretDiskAskPasswordDialog("test");
    }

    void TearDown() override
    {
        delete m_dlg;
    }

};
}

TEST_F(TestMountSecretDiskAskPasswordDialog, show)
{
    m_dlg->show();
    m_dlg->hide();
}
