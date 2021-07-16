#include <gtest/gtest.h>
#include "stub.h"
#include "stubext.h"
#include <ddiskmanager.h>
#include <ddiskdevice.h>
#include <dblockdevice.h>

#include "interfaces/dfmstandardpaths.h"
#define protected public
#define private public
#include "models/dfmrootfileinfo.h"
#include "models/dfmrootfileinfo_p.h"


namespace {
QString idVersion_stub(void *) {
    return "FAT32";
}

QByteArrayList symlinks_stub(void *) {
    QByteArrayList bal;
    bal << QByteArray("/dev/disk/by-label/TEST\\x00");
    return bal;
}

class TestDFMRootFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMRootFileInfo";
        info = new DFMRootFileInfo(DUrl("dfmroot:///desktop.userdir"));
    }

    void TearDown() override
    {
        std::cout << "end TestDFMRootFileInfo";
        delete info;
    }

public:
    DFMRootFileInfo *info;
};
} // namespace

TEST_F(TestDFMRootFileInfo, exist)
{
    ASSERT_TRUE(info->exists());
}

TEST_F(TestDFMRootFileInfo, canRename)
{
    info->canRename();
    DFMRootFileInfo *sda1 = new DFMRootFileInfo(DUrl("dfmroot:///sda1.localdisk"));
    sda1->canRename();
    delete sda1;
}

TEST_F(TestDFMRootFileInfo, suffix)
{
    EXPECT_STREQ("userdir", info->suffix().toStdString().c_str());
}

TEST_F(TestDFMRootFileInfo, fileDisplayName)
{
    EXPECT_STREQ("Desktop", info->fileDisplayName().toStdString().c_str());
}

TEST_F(TestDFMRootFileInfo, canShare)
{
    ASSERT_FALSE(info->canShare());
}

TEST_F(TestDFMRootFileInfo, canFetch)
{
    ASSERT_FALSE(info->canFetch());
}

TEST_F(TestDFMRootFileInfo, isReadable)
{
    ASSERT_TRUE(info->isReadable());
}

TEST_F(TestDFMRootFileInfo, isWritable)
{
    ASSERT_FALSE(info->isWritable());
}

TEST_F(TestDFMRootFileInfo, isExecutable)
{
    ASSERT_FALSE(info->isExecutable());
}

TEST_F(TestDFMRootFileInfo, isHidden)
{
    ASSERT_FALSE(info->isHidden());
}

TEST_F(TestDFMRootFileInfo, isRelative)
{
    ASSERT_FALSE(info->isRelative());
}

TEST_F(TestDFMRootFileInfo, isAbsolute)
{
    ASSERT_TRUE(info->isAbsolute());
}

TEST_F(TestDFMRootFileInfo, isShared)
{
    ASSERT_FALSE(info->isShared());
}

TEST_F(TestDFMRootFileInfo, isTaged)
{
    ASSERT_FALSE(info->isTaged());
}

TEST_F(TestDFMRootFileInfo, isWritableShared)
{
    ASSERT_FALSE(info->isWritableShared());
}

TEST_F(TestDFMRootFileInfo, isAllowGuestShared)
{
    ASSERT_FALSE(info->isAllowGuestShared());
}

TEST_F(TestDFMRootFileInfo, isFile)
{
    ASSERT_TRUE(info->isFile());
}

TEST_F(TestDFMRootFileInfo, isDir)
{
    ASSERT_FALSE(info->isDir());
}

TEST_F(TestDFMRootFileInfo, fileType)
{
    EXPECT_EQ(257, info->fileType());
    DFMRootFileInfo *ftp = new DFMRootFileInfo(DUrl("ftp:///testfile.gvfsmp"));
    ftp->fileType();
    delete ftp;
    DFMRootFileInfo *smb = new DFMRootFileInfo(DUrl("smb:///testfile.gvfsmp"));
    smb->fileType();
    delete smb;
    DFMRootFileInfo *mtp = new DFMRootFileInfo(DUrl("mtp:///testfile.gvfsmp"));
    mtp->fileType();
    delete mtp;
    DFMRootFileInfo *gphoto = new DFMRootFileInfo(DUrl("gphoto2:///testfile.gvfsmp"));
    gphoto->fileType();
    delete gphoto;
}

TEST_F(TestDFMRootFileInfo, filesCount)
{
    EXPECT_EQ(0, info->filesCount());
}

TEST_F(TestDFMRootFileInfo, iconName)
{
    EXPECT_STREQ("user-desktop", info->iconName().toStdString().c_str());
    DFMRootFileInfo *sda1 = new DFMRootFileInfo(DUrl("dfmroot:///sda1.localdisk"));
    sda1->iconName();
    stub_ext::StubExt st;
    st.set_lamda(ADDR(DDiskDevice, mediaCompatibility), []{ return QStringList{"optical"}; });
    sda1->iconName();
    st.reset(ADDR(DDiskDevice, mediaCompatibility));
    st.set_lamda(VADDR(DFMRootFileInfo, fileType), []{
        DFMRootFileInfo::ItemType type = DFMRootFileInfo::ItemType::UDisksRemovable;
        return static_cast<DFMRootFileInfo::FileType>(type);
    });
    sda1->iconName();
    sda1->d_ptr->mps << QByteArray("/\0", 2);
    sda1->iconName();

    delete sda1;
}

TEST_F(TestDFMRootFileInfo, canRedirectionFileUrl)
{
    EXPECT_TRUE(info->canRedirectionFileUrl());
}

TEST_F(TestDFMRootFileInfo, redirectedFileUrl)
{
    EXPECT_STREQ(DFMStandardPaths::location(DFMStandardPaths::DesktopPath).toStdString().c_str(), info->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestDFMRootFileInfo, canDrop)
{
    EXPECT_FALSE(info->canDrop());
}

TEST_F(TestDFMRootFileInfo, tstSupportedDragNDropActions)
{
    EXPECT_TRUE(Qt::DropAction::IgnoreAction == info->supportedDragActions());
    EXPECT_TRUE(Qt::DropAction::IgnoreAction == info->supportedDropActions());
}

TEST_F(TestDFMRootFileInfo, tstGetVoltag)
{
    EXPECT_TRUE(!info->getVolTag().isEmpty());
}

TEST_F(TestDFMRootFileInfo, tstCheckMpsStr)
{
    EXPECT_FALSE(info->checkMpsStr("Test"));
}

TEST_F(TestDFMRootFileInfo, tstMenuActionList)
{
    EXPECT_TRUE(info->menuActionList().count() > 0);
    DFMRootFileInfo *gvfs = new DFMRootFileInfo(DUrl("dfmroot:///fakeDisk.gvfsmp"));
    gvfs->menuActionList();
    delete gvfs;

    stub_ext::StubExt st;
    st.set_lamda(ADDR(QVariant, toBool), []{ return true; });
    DFMRootFileInfo *sda1 = new DFMRootFileInfo(DUrl("dfmroot:///sda1.localdisk"));
    sda1->menuActionList();
    delete sda1;

    st.set_lamda(ADDR(DBlockDevice, readOnly), []{ return false; });
    st.set_lamda(ADDR(DDiskDevice, optical), []{ return true; });
    st.set_lamda(ADDR(DDiskDevice, media), []{ return "_rw"; });
    st.set_lamda(ADDR(DDiskDevice, ejectable), []{ return true; });
    st.set_lamda(ADDR(DDiskDevice, canPowerOff), []{ return true; });
    st.set_lamda(ADDR(DDiskDevice, mediaCompatibility), []{ return QStringList{"optical"}; });
    DFMRootFileInfo *sda = new DFMRootFileInfo(DUrl("dfmroot:///sda.localdisk"));
    sda->menuActionList();
    delete sda;
}

TEST_F(TestDFMRootFileInfo, tstConstructor)
{
    QStringList (*resolveDeviceNode_stub)(QString, QVariantMap) = [](QString, QVariantMap){ return QStringList(); };
    Stub st;
    st.set(ADDR(DDiskManager, resolveDeviceNode), resolveDeviceNode_stub);
    DFMRootFileInfo *localDisk = new DFMRootFileInfo(DUrl("dfmroot:///sdN.localdisk"));
    localDisk->fileDisplayName();
    localDisk->fileName();
    localDisk->redirectedFileUrl();
    localDisk->extraProperties();
    delete localDisk;

    DFMRootFileInfo *gvfs = new DFMRootFileInfo(DUrl("dfmroot:///fakeFile.gvfsmp"));
    gvfs->fileDisplayName();
    gvfs->iconName();
    gvfs->redirectedFileUrl();
    gvfs->extraProperties();
    delete gvfs;

    DFMRootFileInfo *fake = new DFMRootFileInfo(DUrl("dfmroot:///fakeFile.fakeSuffix"));
    fake->fileDisplayName();
    fake->iconName();
    fake->redirectedFileUrl();
    fake->extraProperties();
    delete fake;
}

TEST_F(TestDFMRootFileInfo, tstCheckCache)
{
    Stub st;
    st.set(ADDR(DBlockDevice, idVersion), idVersion_stub);
    DFMRootFileInfo *fat32 = new DFMRootFileInfo(DUrl("dfmroot:///sda.localdisk"));
    fat32->checkCache();
    delete fat32;
}

TEST_F(TestDFMRootFileInfo, tstCanSetAlias)
{
    EXPECT_FALSE(info->canSetAlias());
}

TEST_F(TestDFMRootFileInfo, tstUdisksDispalyAlias)
{
    EXPECT_STREQ(info->udisksDispalyAlias().toStdString().c_str(), "");
}

TEST_F(TestDFMRootFileInfo, tstGetUUID)
{
    EXPECT_STREQ(info->getUUID().toStdString().c_str(), "");
}
