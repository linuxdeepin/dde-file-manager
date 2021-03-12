/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
