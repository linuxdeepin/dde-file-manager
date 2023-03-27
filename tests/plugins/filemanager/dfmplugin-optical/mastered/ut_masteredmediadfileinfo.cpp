// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    EXPECT_EQ(Qt::IgnoreAction, info->supportedOfAttributes(SupportedType::kDrop));
    stub.set_lamda(VADDR(FileInfo, supportedOfAttributes), [] { __DBG_STUB_INVOKE__ return Qt::IgnoreAction; });
    ret = true;
    EXPECT_EQ(Qt::IgnoreAction, info->supportedOfAttributes(SupportedType::kDrop));
}
