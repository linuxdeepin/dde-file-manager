/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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

#include "stubext.h"
#include "plugins/filemanager/dfmplugin-optical/mastered/masteredmediafileinfo.h"
#include "plugins/filemanager/dfmplugin-optical/mastered/masteredmediafileinfo_p.h"
#include "plugins/filemanager/dfmplugin-optical/utils/opticalhelper.h"

#include <gtest/gtest.h>

DPOPTICAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_MasteredMediaFileInfo : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&MasteredMediaFileInfoPrivate::backupInfo, [] { __DBG_STUB_INVOKE__ });
        info = new MasteredMediaFileInfo(QUrl("burn:///file/to/disc"));
    }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    MasteredMediaFileInfo *info { nullptr };
};

// TEST_F(UT_MasteredMediaFileInfo, ){}
TEST_F(UT_MasteredMediaFileInfo, SupportedDropActions)
{
    bool ret { false };
    stub.set_lamda(OpticalHelper::isBurnEnabled, [&] { __DBG_STUB_INVOKE__ return ret; });

    EXPECT_EQ(Qt::IgnoreAction, info->supportedAttributes(Support::kDrop));
    stub.set_lamda(VADDR(AbstractFileInfo, supportedAttributes), [] { __DBG_STUB_INVOKE__ return Qt::IgnoreAction; });
    ret = true;
    EXPECT_EQ(Qt::IgnoreAction, info->supportedAttributes(Support::kDrop));
}
