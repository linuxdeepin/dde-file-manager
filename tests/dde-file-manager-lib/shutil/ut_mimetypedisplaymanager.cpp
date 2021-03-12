/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
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

#include "shutil/mimetypedisplaymanager.h"
#include "shutil/fileutils.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
    class TestMimeTypeDisplayManager : public testing::Test {
    public:
        void SetUp() override
        {
           mDisplayManager.reset(new MimeTypeDisplayManager());
           mDisplayManager->loadSupportMimeTypes();
        }
        void TearDown() override
        {
        }

    public:
        std::shared_ptr<MimeTypeDisplayManager> mDisplayManager = nullptr;

        QString mimeTypeDesktop = "application/x-desktop";
        QString mimeTypeDirectory = "inode/directory";
        QString mimeTypeAppexe = "application/x-executable" ;
        QString mimeTypeAppexe2 = "x-executable" ;
        QString mimeTypeVideo = "video/";
        QString mimeTypeAudio = "audio/";
        QString mimeTypeImage = "image/";
        QString mimeTypeText = "text/";
        QString mimeTypeArchive = "archive";
        QString mimeTypeBackups = "backup";
    };

}

TEST_F(TestMimeTypeDisplayManager, can_get_mimetype_enum)
{
    QStringList archivelist = mDisplayManager->supportArchiveMimetypes();
    QStringList videolisst = mDisplayManager->supportVideoMimeTypes();

    EXPECT_EQ("Application (application/x-desktop)", mDisplayManager->displayName(mimeTypeDesktop));
    EXPECT_EQ("application-default-icon", mDisplayManager->defaultIcon(mimeTypeDesktop));
    QMap<DAbstractFileInfo::FileType, QString> mapDisplayname = mDisplayManager->displayNames();

    EXPECT_EQ( DAbstractFileInfo::FileType::DesktopApplication, mDisplayManager->displayNameToEnum(mimeTypeDesktop));
    EXPECT_EQ( DAbstractFileInfo::FileType::Directory, mDisplayManager->displayNameToEnum(mimeTypeDirectory));
    EXPECT_EQ( DAbstractFileInfo::FileType::Executable, mDisplayManager->displayNameToEnum(mimeTypeAppexe));
    EXPECT_EQ( DAbstractFileInfo::FileType::Videos, mDisplayManager->displayNameToEnum(mimeTypeVideo));
    EXPECT_EQ( DAbstractFileInfo::FileType::Audios, mDisplayManager->displayNameToEnum(mimeTypeAudio));
    EXPECT_EQ( DAbstractFileInfo::FileType::Images, mDisplayManager->displayNameToEnum(mimeTypeImage));
    EXPECT_EQ( DAbstractFileInfo::FileType::Documents, mDisplayManager->displayNameToEnum(mimeTypeText));
//    EXPECT_EQ( DAbstractFileInfo::FileType::Archives, mDisplayManager->displayNameToEnum(mimeTypeArchive));
//    EXPECT_EQ( DAbstractFileInfo::FileType::Backups, mDisplayManager->displayNameToEnum(mimeTypeBackups));
    EXPECT_EQ( DAbstractFileInfo::FileType::Unknown, mDisplayManager->displayNameToEnum("nothing"));
}
