#include <gtest/gtest.h>
#include "stub.h"
#include <ddiskmanager.h>

#define private public
#include "models/masteredmediafileinfo.h"


namespace {
class TestMasteredMediaFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMasteredMediaFileInfo";

        info = new MasteredMediaFileInfo(DUrl("burn:///dev/sr0/disc_files/test.fileS"));
    }

    void TearDown() override
    {
        std::cout << "end TestMasteredMediaFileInfo";
        delete info;
        info = nullptr;
    }

public:
    MasteredMediaFileInfo *info;
};
} // namespace

TEST_F(TestMasteredMediaFileInfo, fileDoNotExist)
{
    EXPECT_FALSE(info->exists());
}

TEST_F(TestMasteredMediaFileInfo, fileIsReadable)
{
    info->isReadable();
}

TEST_F(TestMasteredMediaFileInfo, fileIsWritable)
{
    info->isWritable();
}

TEST_F(TestMasteredMediaFileInfo, fileIsDir)
{
    info->isDir();
}

TEST_F(TestMasteredMediaFileInfo, filesCount)
{
    info->filesCount();
}

//TEST_F(TestMasteredMediaFileInfo, fileDisplayName) {
//    EXPECT_STREQ("test", info->fileDisplayName().toStdString().c_str());
//}

TEST_F(TestMasteredMediaFileInfo, fileCanRedirectUrl)
{
    info->canRedirectionFileUrl();
}

TEST_F(TestMasteredMediaFileInfo, redirectUrl)
{
    EXPECT_STRNE("/test.file", info->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestMasteredMediaFileInfo, mimeDataUrl)
{
    EXPECT_STRNE("/test.file", info->mimeDataUrl().path().toStdString().c_str());
}

TEST_F(TestMasteredMediaFileInfo, canIteratorDir)
{
    EXPECT_TRUE(info->canIteratorDir());
}

TEST_F(TestMasteredMediaFileInfo, parentUrlIsRoot)
{
    //    EXPECT_STREQ(QDir("~/").absolutePath().toStdString().c_str(), info->parentUrl().path().toStdString().c_str());
    EXPECT_FALSE(info->parentUrl().path().isEmpty());
}

TEST_F(TestMasteredMediaFileInfo, goToUrlWhenDeleted)
{
    //    EXPECT_STREQ(QDir("~/").absolutePath().toStdString().c_str(), info->goToUrlWhenDeleted().path().toStdString().c_str());
    EXPECT_FALSE(info->parentUrl().path().isEmpty());
}

TEST_F(TestMasteredMediaFileInfo, toLocalFile)
{
    EXPECT_STREQ("", info->toLocalFile().toStdString().c_str());
}

TEST_F(TestMasteredMediaFileInfo, canDrop)
{
    EXPECT_TRUE(info->canDrop());
}

TEST_F(TestMasteredMediaFileInfo, canTag)
{
    EXPECT_FALSE(info->canTag());
}

TEST_F(TestMasteredMediaFileInfo, canRename)
{
    info->refresh();
    EXPECT_FALSE(info->canRename());
}

TEST_F(TestMasteredMediaFileInfo, tstMenuActionList)
{
    EXPECT_TRUE(info->menuActionList(DAbstractFileInfo::MenuType::SpaceArea).count() > 0);

    typedef bool (*MMFI_isSomething)(MasteredMediaFileInfo *);
    MMFI_isSomething isWritable = (MMFI_isSomething)(&MasteredMediaFileInfo::isWritable);
    MMFI_isSomething isVirtualEntry = (MMFI_isSomething)(&MasteredMediaFileInfo::isVirtualEntry);
    bool (*isWritable_stub)() = [](){ return false; };
    bool (*isVirtualEntry_stub)() = [](){ return true; };
    Stub st;
    st.set(isWritable, isWritable_stub);
    st.set(isVirtualEntry, isVirtualEntry_stub);
    EXPECT_TRUE(info->disableMenuActionList().count() > 0);
}

TEST_F(TestMasteredMediaFileInfo, tstGetVolTag)
{
    EXPECT_TRUE(info->getVolTag("/dev/sr0") == "sr0");
}

TEST_F(TestMasteredMediaFileInfo, tstGoToUrlWhenDeleted)
{
    info->goToUrlWhenDeleted();
    Stub st;
    QStringList (*resolveDeviceNode_stub)(QString, QVariantMap) = [](QString, QVariantMap){ return QStringList() << "/dev/sr0"; };
    st.set(&DDiskManager::resolveDeviceNode, resolveDeviceNode_stub);
    info->m_backerUrl = DUrl("file:///home");
    info->goToUrlWhenDeleted();
}

TEST_F(TestMasteredMediaFileInfo, tstFileDisplayName)
{
    info->fileDisplayName();
}

TEST_F(TestMasteredMediaFileInfo, tstExtraProperties)
{
    info->extraProperties();
}
