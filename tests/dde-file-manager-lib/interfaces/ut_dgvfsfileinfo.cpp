#include <gtest/gtest.h>
#include "testhelper.h"
#include "stub.h"
#include "stubext.h"

#include <QStandardPaths>
#include <QIcon>
#define protected public
#define private public
#include "interfaces/dgvfsfileinfo.h"
#include "private/dgvfsfileinfo_p.h"
#include "private/dfileinfo_p.h"

namespace {
class DGvfsFileInfoTest : public testing::Test
{
public:
    void SetUp() override
    {
        QProcess::execute("mkdir " + QDir::currentPath()+"/123");
        QProcess::execute("touch " + QDir::currentPath()+"/123/1.txt");
        QProcess::execute("touch " + QDir::currentPath()+"/123/2.txt");
        QProcess::execute("ln -s "+QDir::currentPath()+"/123/2.txt " +QDir::currentPath()+"/123/22.txt");
        m_dirPathStr = QDir::currentPath()+"/123";
        m_filePathStr = QDir::currentPath()+"/123/1.txt";
        m_filePathStr2 = QDir::currentPath()+"/123/2.txt";
        m_symlinkPathStr = QDir::currentPath()+"/123/22.txt";

        m_pDirInfo = new DGvfsFileInfo(m_dirPathStr);
        m_pFileInfo = new DGvfsFileInfo(QFileInfo(m_filePathStr));
        m_pSymLinkInfo = new DGvfsFileInfo(QFileInfo(m_symlinkPathStr));
        m_fileinfo.reset(new DGvfsFileInfo(m_filePathStr2));
    }

    void TearDown() override
    {
        if (m_pDirInfo != nullptr) {
            delete m_pDirInfo;
            m_pDirInfo = nullptr;
        }

        if (m_pSymLinkInfo != nullptr) {
            delete m_pSymLinkInfo;
            m_pSymLinkInfo = nullptr;
        }

        if (m_pFileInfo != nullptr) {
            delete m_pFileInfo;
            m_pFileInfo = nullptr;
        }
        QProcess::execute("rm -rf  " + m_dirPathStr);
    }

public:
    QString m_filePathStr;
    QString m_filePathStr2;
    QString m_dirPathStr;
    QString m_symlinkPathStr;

    DFileInfo *m_pDirInfo;
    DFileInfo *m_pFileInfo;
    DFileInfo *m_pSymLinkInfo;
    QSharedPointer<DGvfsFileInfo> m_fileinfo;
};
} // namespace


TEST_F(DGvfsFileInfoTest, test_fileinfo_creator)
{
    DUrl fileurl;
    fileurl.setScheme(FILE_SCHEME);
    fileurl.setPath(m_filePathStr);
    m_fileinfo.reset(new DGvfsFileInfo(fileurl,QMimeType()));
    m_fileinfo.reset(new DGvfsFileInfo(fileurl));
    m_fileinfo.reset(new DGvfsFileInfo(QFileInfo(m_filePathStr),QMimeType()));
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_canRename)
{
    EXPECT_TRUE(m_fileinfo->canRename());
    EXPECT_TRUE(m_fileinfo->canRename());
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_isWritable)
{
    EXPECT_TRUE(m_fileinfo->isWritable());
    EXPECT_TRUE(m_fileinfo->isWritable());
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_makeAbsolute)
{
    EXPECT_FALSE(m_fileinfo->makeAbsolute());
    EXPECT_FALSE(m_fileinfo->makeAbsolute());
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_isSymLink)
{
    EXPECT_FALSE(m_fileinfo->isSymLink());
    EXPECT_FALSE(m_fileinfo->isSymLink());
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_isDir)
{
    EXPECT_FALSE(m_fileinfo->isDir());
    EXPECT_FALSE(m_fileinfo->isDir());
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_lastRead)
{
    EXPECT_FALSE(m_fileinfo->lastRead().toString().isEmpty());
    EXPECT_FALSE(m_fileinfo->lastRead().toString().isEmpty());
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_lastModified)
{
    EXPECT_FALSE(m_fileinfo->lastModified().toString().isEmpty());
    EXPECT_FALSE(m_fileinfo->lastModified().toString().isEmpty());
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_ownerId)
{
    EXPECT_TRUE(m_fileinfo->ownerId() != 0);
    EXPECT_TRUE(m_fileinfo->ownerId() != 0);
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_size)
{
    EXPECT_TRUE(m_fileinfo->size() == 0);
    EXPECT_TRUE(m_fileinfo->size() == 0);
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_filesCount)
{
    EXPECT_FALSE(m_pDirInfo->filesCount() == 0);
    EXPECT_FALSE(m_pDirInfo->filesCount() == 0);
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_refresh)
{
    EXPECT_NO_FATAL_FAILURE(m_fileinfo->refresh());
    EXPECT_NO_FATAL_FAILURE(m_fileinfo->refresh(true));
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_makeToActive)
{
    EXPECT_NO_FATAL_FAILURE(m_fileinfo->makeToActive());
}

TEST_F(DGvfsFileInfoTest, test_fileinfo_inode)
{
    EXPECT_TRUE(m_fileinfo->inode() != 0);
}

#ifndef __arm__
TEST_F(DGvfsFileInfoTest, test_fileinfo_fileIcon)
{
    DFileInfo *parent = static_cast<DFileInfo *>(m_fileinfo.data());
    EXPECT_NO_FATAL_FAILURE(m_fileinfo->fileIcon().isNull());

    parent->d_func()->needThumbnail = true;
    {
    stub_ext::StubExt st;
    st.set_lamda(&QIcon::isNull, []{ return false; });
    EXPECT_NO_FATAL_FAILURE(m_fileinfo->fileIcon().isNull());
    }

//    {
//    stub_ext::StubExt st;
//    st.set_lamda(&DAbstractFileInfo::isActive, []{ return true; });
//    parent->d_func()->needThumbnail = true;
//    EXPECT_FALSE(m_fileinfo->fileIcon().isNull());
//    }

    DGvfsFileInfo *info = new DGvfsFileInfo("/tmp/");
    DGvfsFileInfo *info_home = new DGvfsFileInfo("file:///home");
    parent->d_func()->needThumbnail = false;
    {
    stub_ext::StubExt st;
    st.set_lamda(&QIcon::isNull, []{ return true; });
    st.set_lamda(VADDR(DGvfsFileInfo, isSymLink), []{ return true; });
    st.set_lamda(VADDR(DGvfsFileInfo, symLinkTarget), []{ return DUrl("file:///home"); });
    EXPECT_NO_FATAL_FAILURE(info->fileIcon().isNull());

    EXPECT_NO_FATAL_FAILURE(info_home->fileIcon().isNull());
    }
    {
        stub_ext::StubExt st;
        st.set_lamda(VADDR(DGvfsFileInfo, isShared), []{ return true; });
        EXPECT_NO_FATAL_FAILURE(info->additionalIcon().isEmpty());
    }
    {
        stub_ext::StubExt st;
        st.set_lamda(VADDR(DGvfsFileInfo, isSymLink), []{ return true; });
        EXPECT_NO_FATAL_FAILURE(info->additionalIcon().isEmpty());
    }
    delete info;
    delete info_home;
}
#endif
