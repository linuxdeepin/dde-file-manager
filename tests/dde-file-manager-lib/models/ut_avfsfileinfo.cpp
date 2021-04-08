#include "models/avfsfileinfo.h"
#include "dabstractfileinfo.h"
#include <gtest/gtest.h>

#include "stub.h"
#include "shutil/fileutils.h"

namespace {
class TestAVFSFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestAVFSFileInfo";
        info = new AVFSFileInfo(DUrl("avfs:///test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestAVFSFileInfo";
        delete info;
    }

public:
    AVFSFileInfo *info;
};
} // namespace

TEST_F(TestAVFSFileInfo, canRename)
{
    EXPECT_FALSE(info->canRename());
}

TEST_F(TestAVFSFileInfo, isWritable)
{
    EXPECT_FALSE(info->isWritable());
}

TEST_F(TestAVFSFileInfo, canShare)
{
    EXPECT_FALSE(info->canShare());
}

TEST_F(TestAVFSFileInfo, canIteratorDir)
{
    EXPECT_TRUE(info->canIteratorDir());
}

TEST_F(TestAVFSFileInfo, isDir)
{
    EXPECT_FALSE(info->isDir());
    Stub st;
    bool (*isArchive_stub)(const QString &) = [](const QString &){ return true; };
    st.set(&FileUtils::isArchive, isArchive_stub);
    info->isDir();
}

TEST_F(TestAVFSFileInfo, canManageAuto)
{
    EXPECT_FALSE(info->canManageAuth());
}

TEST_F(TestAVFSFileInfo, toLocalFile)
{
    EXPECT_STREQ("/test.file", info->toLocalFile().toStdString().c_str());
}

TEST_F(TestAVFSFileInfo, parentUrl)
{
    EXPECT_STREQ("/", info->parentUrl().path().toStdString().c_str());
    Stub st;
    typedef DAbstractFileInfo::FileType (*vptr)();
    DAbstractFileInfo::FileType (*fileType_stub)() = [](){ return DAbstractFileInfo::FileType::Directory; };
    st.set((vptr)&DAbstractFileInfo::fileType, fileType_stub);
    info->parentUrl();
}

TEST_F(TestAVFSFileInfo, tstMenuActionList)
{
    QVector<MenuAction> menus = info->menuActionList(DAbstractFileInfo::SingleFile);
    EXPECT_TRUE(menus.count() > 0);
    menus = info->menuActionList(DAbstractFileInfo::MultiFiles);
    EXPECT_TRUE(menus.count() > 0);
    menus = info->menuActionList(DAbstractFileInfo::SpaceArea);
    EXPECT_TRUE(menus.count() > 0);
}

TEST_F(TestAVFSFileInfo, tstRealDirUrl)
{
    DUrl u = info->realDirUrl(DUrl("avfs:///test.file/"));
    EXPECT_TRUE(u.isValid());
    info->realFileUrl(DUrl("avfs:///test.file/"));
}
