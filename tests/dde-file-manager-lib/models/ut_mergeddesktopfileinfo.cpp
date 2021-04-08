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

#include <gtest/gtest.h>

#define protected public

#include "models/mergeddesktopfileinfo.h"
#include "models/virtualentryinfo.h"
#include "controllers/mergeddesktopcontroller.h"
#include "stub.h"

namespace {
class TestMergedDesktopFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMergedDesktopFileInfo";

        info = new MergedDesktopFileInfo(DUrl("/"), DUrl("/"));
    }

    void TearDown() override
    {
        std::cout << "end TestMergedDesktopFileInfo";

        delete info;
    }

public:
    MergedDesktopFileInfo *info;
};
} // namespace

TEST_F(TestMergedDesktopFileInfo, parentUrl)
{
    EXPECT_STREQ("/", info->parentUrl().path().toStdString().c_str());
}

TEST_F(TestMergedDesktopFileInfo, iconName)
{
    EXPECT_STREQ("", info->iconName().toStdString().c_str());
}

TEST_F(TestMergedDesktopFileInfo, genericIconName)
{
    EXPECT_STREQ("-x-generic", info->genericIconName().toStdString().c_str());
}

TEST_F(TestMergedDesktopFileInfo, mimeDataUrl)
{
    EXPECT_STREQ("/", info->mimeDataUrl().path().toStdString().c_str());
}

TEST_F(TestMergedDesktopFileInfo, canRedirectUrl)
{
    EXPECT_FALSE(info->canRedirectionFileUrl());
}

TEST_F(TestMergedDesktopFileInfo, redirectedFileUrl)
{
    EXPECT_STREQ("/", info->redirectedFileUrl().path().toStdString().c_str());
}


namespace  {
class TestVirtualEntryInfo: public testing::Test
{
public:
    void SetUp() override
    {
        info = new VirtualEntryInfo(DUrl());
    }

    void TearDown() override
    {
        delete info;
        info = nullptr;
    }

    VirtualEntryInfo *info;
};
}

TEST_F(TestVirtualEntryInfo, tstIconName)
{
    Stub st;

    DMD_TYPES (*entryTypeByName_stub_application)(QString) = [](QString){ return DMD_TYPES::DMD_APPLICATION; };
    st.set(ADDR(MergedDesktopController, entryTypeByName), entryTypeByName_stub_application);
    EXPECT_TRUE(QStringLiteral("folder-applications-stack") == info->iconName());

    DMD_TYPES (*entryTypeByName_stub_documents)(QString) = [](QString){ return DMD_TYPES::DMD_DOCUMENT; };
    st.set(ADDR(MergedDesktopController, entryTypeByName), entryTypeByName_stub_documents);
    EXPECT_TRUE(QStringLiteral("folder-documents-stack") == info->iconName());

    DMD_TYPES (*entryTypeByName_stub_music)(QString) = [](QString){ return DMD_TYPES::DMD_MUSIC; };
    st.set(ADDR(MergedDesktopController, entryTypeByName), entryTypeByName_stub_music);
    EXPECT_TRUE(QStringLiteral("folder-music-stack") == info->iconName());

    DMD_TYPES (*entryTypeByName_stub_picture)(QString) = [](QString){ return DMD_TYPES::DMD_PICTURE; };
    st.set(ADDR(MergedDesktopController, entryTypeByName), entryTypeByName_stub_picture);
    EXPECT_TRUE(QStringLiteral("folder-images-stack") == info->iconName());

    DMD_TYPES (*entryTypeByName_stub_video)(QString) = [](QString){ return DMD_TYPES::DMD_VIDEO; };
    st.set(ADDR(MergedDesktopController, entryTypeByName), entryTypeByName_stub_video);
    EXPECT_TRUE(QStringLiteral("folder-video-stack") == info->iconName());

    DMD_TYPES (*entryTypeByName_stub_other)(QString) = [](QString){ return DMD_TYPES::DMD_OTHER; };
    st.set(ADDR(MergedDesktopController, entryTypeByName), entryTypeByName_stub_other);
    EXPECT_TRUE(QStringLiteral("folder-stack") == info->iconName());

    DMD_TYPES (*entryTypeByName_stub_folder)(QString) = [](QString){ return DMD_TYPES::DMD_FOLDER; };
    st.set(ADDR(MergedDesktopController, entryTypeByName), entryTypeByName_stub_folder);
    EXPECT_TRUE(QStringLiteral("folder-stack") == info->iconName());
}

TEST_F(TestVirtualEntryInfo, tstSimpleFuncs)
{
    EXPECT_TRUE(info->exists());
    EXPECT_TRUE(info->isDir());
    EXPECT_TRUE(info->isVirtualEntry());
    EXPECT_TRUE(info->isReadable());
    EXPECT_TRUE(info->isWritable());
    EXPECT_FALSE(info->canShare());

    EXPECT_TRUE(info->fileItemDisableFlags() == Qt::ItemIsDragEnabled);
    EXPECT_TRUE(info->compareFunByColumn(0) == nullptr);
}

TEST_F(TestVirtualEntryInfo, tstFileName)
{
    info->setUrl(DUrl("/entry/"));
    EXPECT_TRUE("Entry" == info->fileName());
    info->setUrl(DUrl("/entry/test"));
    EXPECT_FALSE("Entry" == info->fileName());

    info->setUrl(DUrl("/folder/"));
    EXPECT_TRUE("Folder" == info->fileName());
    info->setUrl(DUrl("/folder/test"));
    EXPECT_FALSE("Folder" == info->fileName());

    info->setUrl(DUrl("/mergeddesktop/"));
    EXPECT_TRUE("Merged Desktop" == info->fileName());

    info->setUrl(DUrl());
    EXPECT_TRUE(info->fileName().contains("(?)"));
}
