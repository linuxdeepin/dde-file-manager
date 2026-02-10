// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_protocolutils.cpp - ProtocolUtils namespace unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QRegularExpression>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;
using namespace ProtocolUtils;

/**
 * @brief ProtocolUtils namespace unit tests
 *
 * Test scope:
 * 1. Remote file detection (isRemoteFile)
 * 2. MTP file detection (isMTPFile)
 * 3. Gphoto file detection (isGphotoFile)
 * 4. FTP file detection (isFTPFile)
 * 5. SFTP file detection (isSFTPFile)
 * 6. SMB file detection (isSMBFile)
 * 7. Local file detection (isLocalFile)
 * 8. NFS file detection (isNFSFile)
 * 9. DAV file detection (isDavFile)
 * 10. DAVS file detection (isDavsFile)
 * 11. Invalid URL handling
 * 12. Edge cases and boundary conditions
 */
class ProtocolUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        
        // Mock DeviceProxyManager to avoid actual device operations
        stub.set_lamda(&DeviceProxyManager::isFileOfExternalBlockMounts, 
                      [](DeviceProxyManager *self, const QString &path) -> bool {
                          Q_UNUSED(self)
                          Q_UNUSED(path)
                          return false;
                      });
        
        stub.set_lamda(&DeviceProxyManager::isFileOfProtocolMounts, 
                      [](DeviceProxyManager *self, const QString &path) -> bool {
                          Q_UNUSED(self)
                          Q_UNUSED(path)
                          return false;
                      });
    }

    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
    }

    // Test stubbing utility
    stub_ext::StubExt stub;
    
    // Helper function to create test URLs
    QUrl createTestUrl(const QString &scheme, const QString &path) {
        QUrl url;
        url.setScheme(scheme);
        url.setPath(path);
        return url;
    }
    
    // Helper function to create local file URL
    QUrl createLocalFileUrl(const QString &localPath) {
        return QUrl::fromLocalFile(localPath);
    }
};

/**
 * @brief Test isRemoteFile function
 * Verify remote file detection for gvfs and smbmounts paths
 */
TEST_F(ProtocolUtilsTest, IsRemoteFile_ValidUrls)
{
    // Test gvfs paths for user
    QUrl gvfsUserUrl = createLocalFileUrl("/run/user/1000/gvfs/some-share");
    EXPECT_TRUE(isRemoteFile(gvfsUserUrl));
    
    // Test gvfs paths for root
    QUrl gvfsRootUrl = createLocalFileUrl("/root/.gvfs/some-share");
    EXPECT_TRUE(isRemoteFile(gvfsRootUrl));
    
    // Test smbmounts paths
    QUrl smbmountsUrl = createLocalFileUrl("/media/user/smbmounts/share");
    EXPECT_TRUE(isRemoteFile(smbmountsUrl));
    
    // Test run media smbmounts paths
    QUrl runSmbmountsUrl = createLocalFileUrl("/run/media/user/smbmounts/share");
    EXPECT_TRUE(isRemoteFile(runSmbmountsUrl));
    
    // Test non-remote local file
    QUrl localFileUrl = createLocalFileUrl("/home/user/document.txt");
    EXPECT_FALSE(isRemoteFile(localFileUrl));
}

/**
 * @brief Test isRemoteFile with invalid URLs
 * Verify handling of invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsRemoteFile_InvalidUrls)
{
    // Test invalid URL
    QUrl invalidUrl;
    EXPECT_FALSE(isRemoteFile(invalidUrl));
    
    // Test empty URL
    QUrl emptyUrl("");
    EXPECT_FALSE(isRemoteFile(emptyUrl));
    
    // Test URL with invalid scheme but valid local path
    QUrl urlWithInvalidPath = createLocalFileUrl("");
    EXPECT_FALSE(isRemoteFile(urlWithInvalidPath));
}

/**
 * @brief Test isMTPFile function
 * Verify MTP device file detection
 */
TEST_F(ProtocolUtilsTest, IsMTPFile_ValidUrls)
{
    // Test MTP gvfs path for user
    QUrl mtpUserUrl = createLocalFileUrl("/run/user/1000/gvfs/mtp:host=1234");
    EXPECT_TRUE(isMTPFile(mtpUserUrl));
    
    // Test MTP gvfs path for root
    QUrl mtpRootUrl = createLocalFileUrl("/root/.gvfs/mtp:host=5678");
    EXPECT_TRUE(isMTPFile(mtpRootUrl));
    
    // Test non-MTP file
    QUrl localFileUrl = createLocalFileUrl("/home/user/music.mp3");
    EXPECT_FALSE(isMTPFile(localFileUrl));
    
    // Test other gvfs path
    QUrl otherGvfsUrl = createLocalFileUrl("/run/user/1000/gvfs/smb-share");
    EXPECT_FALSE(isMTPFile(otherGvfsUrl));
}

/**
 * @brief Test isMTPFile with invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsMTPFile_InvalidUrls)
{
    QUrl invalidUrl;
    EXPECT_FALSE(isMTPFile(invalidUrl));
}

/**
 * @brief Test isGphotoFile function
 * Verify Gphoto camera file detection
 */
TEST_F(ProtocolUtilsTest, IsGphotoFile_ValidUrls)
{
    // Test Gphoto gvfs path for user
    QUrl gphotoUserUrl = createLocalFileUrl("/run/user/1000/gvfs/gphoto2:host=cam001");
    EXPECT_TRUE(isGphotoFile(gphotoUserUrl));
    
    // Test Gphoto gvfs path for root
    QUrl gphotoRootUrl = createLocalFileUrl("/root/.gvfs/gphoto2:host=cam002");
    EXPECT_TRUE(isGphotoFile(gphotoRootUrl));
    
    // Test non-Gphoto file
    QUrl localFileUrl = createLocalFileUrl("/home/user/photo.jpg");
    EXPECT_FALSE(isGphotoFile(localFileUrl));
    
    // Test other gvfs path
    QUrl otherGvfsUrl = createLocalFileUrl("/run/user/1000/gvfs/mtp:host=1234");
    EXPECT_FALSE(isGphotoFile(otherGvfsUrl));
}

/**
 * @brief Test isGphotoFile with invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsGphotoFile_InvalidUrls)
{
    QUrl invalidUrl;
    EXPECT_FALSE(isGphotoFile(invalidUrl));
}

/**
 * @brief Test isFTPFile function
 * Verify FTP file detection
 */
TEST_F(ProtocolUtilsTest, IsFTPFile_ValidUrls)
{
    // Test FTP gvfs path for user
    QUrl ftpUserUrl = createTestUrl("ftp", "/run/user/1000/gvfs/ftp:host=server.com");
    EXPECT_TRUE(isFTPFile(ftpUserUrl));
    
    // Test FTP gvfs path for root
    QUrl ftpRootUrl = createTestUrl("ftp", "/root/.gvfs/ftp:host=ftp.server.com");
    EXPECT_TRUE(isFTPFile(ftpRootUrl));
    
    // Test SFTP should also match (since regex uses s?ftp)
    QUrl sftpUserUrl = createTestUrl("sftp", "/run/user/1000/gvfs/sftp:host=server.com");
    EXPECT_TRUE(isFTPFile(sftpUserUrl));
    
    // Test non-FTP file
    QUrl localFileUrl = createTestUrl("file", "/home/user/document.txt");
    EXPECT_FALSE(isFTPFile(localFileUrl));
    
    // Test non-matching gvfs path
    QUrl otherGvfsUrl = createTestUrl("file", "/run/user/1000/gvfs/smb-share");
    EXPECT_FALSE(isFTPFile(otherGvfsUrl));
}

/**
 * @brief Test isFTPFile with invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsFTPFile_InvalidUrls)
{
    QUrl invalidUrl;
    EXPECT_FALSE(isFTPFile(invalidUrl));
}

/**
 * @brief Test isSFTPFile function
 * Verify SFTP file detection
 */
TEST_F(ProtocolUtilsTest, IsSFTPFile_ValidUrls)
{
    // Test SFTP gvfs path for user
    QUrl sftpUserUrl = createTestUrl("sftp", "/run/user/1000/gvfs/sftp:host=server.com");
    EXPECT_TRUE(isSFTPFile(sftpUserUrl));
    
    // Test SFTP gvfs path for root
    QUrl sftpRootUrl = createTestUrl("sftp", "/root/.gvfs/sftp:host=ssh.server.com");
    EXPECT_TRUE(isSFTPFile(sftpRootUrl));
    
    // Test non-SFTP file
    QUrl ftpUrl = createTestUrl("ftp", "/run/user/1000/gvfs/ftp:host=server.com");
    EXPECT_FALSE(isSFTPFile(ftpUrl));
    
    // Test non-matching path
    QUrl localFileUrl = createTestUrl("sftp", "/home/user/document.txt");
    EXPECT_FALSE(isSFTPFile(localFileUrl));
}

/**
 * @brief Test isSFTPFile with invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsSFTPFile_InvalidUrls)
{
    QUrl invalidUrl;
    EXPECT_FALSE(isSFTPFile(invalidUrl));
}

/**
 * @brief Test isSMBFile function
 * Verify SMB file detection
 */
TEST_F(ProtocolUtilsTest, IsSMBFile_ValidUrls)
{
    // Test SMB scheme URL
    QUrl smbSchemeUrl = createTestUrl(Global::Scheme::kSmb, "//server/share/file.txt");
    EXPECT_TRUE(isSMBFile(smbSchemeUrl));
    
    // Test SMB gvfs path for user
    QUrl smbUserUrl = createTestUrl("file", "/run/user/1000/gvfs/smb-share:server=server.com,user=user");
    EXPECT_TRUE(isSMBFile(smbUserUrl));
    
    // Test SMB gvfs path for root
    QUrl smbRootUrl = createTestUrl("file", "/root/.gvfs/smb-share:server=smb.server.com");
    EXPECT_TRUE(isSMBFile(smbRootUrl));
    
    // Test SMB mount paths
    QUrl smbMountUrl = createTestUrl("file", "/media/user/smbmounts/share");
    EXPECT_TRUE(isSMBFile(smbMountUrl));
    
    QUrl smbRunMountUrl = createTestUrl("file", "/run/media/user/smbmounts/share");
    EXPECT_TRUE(isSMBFile(smbRunMountUrl));
    
    // Test non-SMB file
    QUrl localFileUrl = createLocalFileUrl("/home/user/document.txt");
    EXPECT_FALSE(isSMBFile(localFileUrl));
}

/**
 * @brief Test isSMBFile with invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsSMBFile_InvalidUrls)
{
    QUrl invalidUrl;
    EXPECT_FALSE(isSMBFile(invalidUrl));
}

/**
 * @brief Test isNFSFile function
 * Verify NFS file detection
 */
TEST_F(ProtocolUtilsTest, IsNFSFile_ValidUrls)
{
    // Test NFS gvfs path for user
    QUrl nfsUserUrl = createTestUrl("file", "/run/user/1000/gvfs/nfs:server=server.com,path=/share");
    EXPECT_TRUE(isNFSFile(nfsUserUrl));
    
    // Test NFS gvfs path for root
    QUrl nfsRootUrl = createTestUrl("file", "/root/.gvfs/nfs:server=nfs.server.com,path=/testroot");
    EXPECT_TRUE(isNFSFile(nfsRootUrl));
    
    // Test non-NFS file
    QUrl localFileUrl = createLocalFileUrl("/home/user/document.txt");
    EXPECT_FALSE(isNFSFile(localFileUrl));
    
    // Test other gvfs path
    QUrl otherGvfsUrl = createTestUrl("file", "/run/user/1000/gvfs/smb-share");
    EXPECT_FALSE(isNFSFile(otherGvfsUrl));
}

/**
 * @brief Test isNFSFile with invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsNFSFile_InvalidUrls)
{
    QUrl invalidUrl;
    EXPECT_FALSE(isNFSFile(invalidUrl));
}

/**
 * @brief Test isDavFile function
 * Verify DAV (WebDAV) file detection
 */
TEST_F(ProtocolUtilsTest, IsDavFile_ValidUrls)
{
    // Test DAV gvfs path for user (ssl=false)
    QUrl davUserUrl = createTestUrl("file", "/run/user/1000/gvfs/dav:host=webdav.server.com,ssl=false");
    EXPECT_TRUE(isDavFile(davUserUrl));
    
    // Test DAV gvfs path for root
    QUrl davRootUrl = createTestUrl("file", "/root/.gvfs/dav:host=dav.server.com,ssl=false,prefix=/webdav");
    EXPECT_TRUE(isDavFile(davRootUrl));
    
    // Test non-DAV file
    QUrl localFileUrl = createLocalFileUrl("/home/user/document.txt");
    EXPECT_FALSE(isDavFile(localFileUrl));
    
    // Test DAVS file (should not match DAV)
    QUrl davsUrl = createTestUrl("file", "/run/user/1000/gvfs/dav:host=server.com,ssl=true");
    EXPECT_FALSE(isDavFile(davsUrl));
}

/**
 * @brief Test isDavFile with invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsDavFile_InvalidUrls)
{
    QUrl invalidUrl;
    EXPECT_FALSE(isDavFile(invalidUrl));
}

/**
 * @brief Test isDavsFile function
 * Verify DAVS (WebDAV over SSL) file detection
 */
TEST_F(ProtocolUtilsTest, IsDavsFile_ValidUrls)
{
    // Test DAVS gvfs path for user (ssl=true)
    QUrl davsUserUrl = createTestUrl("file", "/run/user/1000/gvfs/dav:host=webdav.server.com,ssl=true");
    EXPECT_TRUE(isDavsFile(davsUserUrl));
    
    // Test DAVS gvfs path for root
    QUrl davsRootUrl = createTestUrl("file", "/root/.gvfs/dav:host=dav.server.com,ssl=true,prefix=/webdav");
    EXPECT_TRUE(isDavsFile(davsRootUrl));
    
    // Test non-DAVS file
    QUrl localFileUrl = createLocalFileUrl("/home/user/document.txt");
    EXPECT_FALSE(isDavsFile(localFileUrl));
    
    // Test DAV file (should not match DAVS)
    QUrl davUrl = createTestUrl("file", "/run/user/1000/gvfs/dav:host=server.com,ssl=false");
    EXPECT_FALSE(isDavsFile(davUrl));
}

/**
 * @brief Test isDavsFile with invalid URLs
 */
TEST_F(ProtocolUtilsTest, IsDavsFile_InvalidUrls)
{
    QUrl invalidUrl;
    EXPECT_FALSE(isDavsFile(invalidUrl));
}

/**
 * @brief Test isLocalFile function
 * Verify local file detection with various scenarios
 */
TEST_F(ProtocolUtilsTest, IsLocalFile_ValidUrls)
{
    // Test normal local file
    QUrl localFileUrl = createLocalFileUrl("/home/user/document.txt");
    EXPECT_TRUE(isLocalFile(localFileUrl));
    
    // Test absolute local file
    QUrl absoluteUrl = createLocalFileUrl("/tmp/test.txt");
    EXPECT_TRUE(isLocalFile(absoluteUrl));
    
    // Test that remote files are not considered local
    QUrl remoteUrl = createLocalFileUrl("/run/user/1000/gvfs/smb-share");
    EXPECT_FALSE(isLocalFile(remoteUrl));
}

/**
 * @brief Test isLocalFile with external device mounts
 * Verify that external block device mounts are not considered local
 */
TEST_F(ProtocolUtilsTest, IsLocalFile_ExternalBlockMounts)
{
    // Mock DeviceProxyManager to return true for external block mounts
    stub.set_lamda(&DeviceProxyManager::isFileOfExternalBlockMounts, 
                  [](DeviceProxyManager *self, const QString &path) -> bool {
                      Q_UNUSED(self)
                      return path.contains("/media/user/usb");
                  });
    
    QUrl externalDeviceUrl = createLocalFileUrl("/media/user/usb/device/file.txt");
    EXPECT_FALSE(isLocalFile(externalDeviceUrl));
    
    // Test that other local files are still considered local
    QUrl normalLocalUrl = createLocalFileUrl("/home/user/document.txt");
    EXPECT_TRUE(isLocalFile(normalLocalUrl));
}

/**
 * @brief Test isLocalFile with protocol mounts
 * Verify that protocol mounts are not considered local
 */
TEST_F(ProtocolUtilsTest, IsLocalFile_ProtocolMounts)
{
    // Mock DeviceProxyManager to return true for protocol mounts
    stub.set_lamda(&DeviceProxyManager::isFileOfProtocolMounts, 
                  [](DeviceProxyManager *self, const QString &path) -> bool {
                      Q_UNUSED(self)
                      return path.contains("/run/user/1000/gvfs");
                  });
    
    QUrl protocolMountUrl = createLocalFileUrl("/run/user/1000/gvfs/sftp:host=server.com");
    EXPECT_FALSE(isLocalFile(protocolMountUrl));
    
    // Test that other local files are still considered local
    QUrl normalLocalUrl = createLocalFileUrl("/home/user/document.txt");
    EXPECT_TRUE(isLocalFile(normalLocalUrl));
}

/**
 * @brief Test isLocalFile with non-local URLs
 * Verify that non-local file URLs are not considered local
 */
TEST_F(ProtocolUtilsTest, IsLocalFile_NonLocalUrls)
{
    // Test non-local URL
    QUrl httpUrl = createTestUrl("http", "example.com/path");
    EXPECT_FALSE(isLocalFile(httpUrl));
    
    QUrl ftpUrl = createTestUrl("ftp", "ftp.server.com/path");
    EXPECT_FALSE(isLocalFile(ftpUrl));
}

/**
 * @brief Test isLocalFile edge cases
 * Verify edge case handling
 */
TEST_F(ProtocolUtilsTest, IsLocalFile_EdgeCases)
{
    // Test local file that's also a remote path (should be false)
    QUrl localRemotePathUrl = createLocalFileUrl("/run/user/1000/gvfs/smb-share");
    EXPECT_FALSE(isLocalFile(localRemotePathUrl));
    
    // Test local file with path that doesn't exist (still considered local)
    QUrl nonExistentLocalUrl = createLocalFileUrl("/nonexistent/path/file.txt");
    EXPECT_TRUE(isLocalFile(nonExistentLocalUrl));
}

/**
 * @brief Test consistency between protocol detection functions
 * Verify that functions correctly identify different protocols
 */
TEST_F(ProtocolUtilsTest, ProtocolDetection_Consistency)
{
    // Test SMB scheme detection
    QUrl smbUrl = createTestUrl(Global::Scheme::kSmb, "//server/share");
    EXPECT_TRUE(isSMBFile(smbUrl));
    EXPECT_FALSE(isFTPFile(smbUrl));
    EXPECT_FALSE(isSFTPFile(smbUrl));
    EXPECT_FALSE(isMTPFile(smbUrl));
    EXPECT_FALSE(isGphotoFile(smbUrl));
    EXPECT_FALSE(isNFSFile(smbUrl));
    EXPECT_FALSE(isDavFile(smbUrl));
    EXPECT_FALSE(isDavsFile(smbUrl));
    
    // Test MTP detection
    QUrl mtpUrl = createLocalFileUrl("/run/user/1000/gvfs/mtp:host=device");
    EXPECT_TRUE(isMTPFile(mtpUrl));
    EXPECT_TRUE(isRemoteFile(mtpUrl));
    EXPECT_FALSE(isSMBFile(mtpUrl));
    EXPECT_FALSE(isFTPFile(mtpUrl));
}

/**
 * @brief Test different user IDs in gvfs paths
 * Verify that various user ID patterns work correctly
 */
TEST_F(ProtocolUtilsTest, GvfsPaths_DifferentUserIds)
{
    // Test different user IDs
    QUrl user1000Url = createLocalFileUrl("/run/user/1000/gvfs/smb-share");
    EXPECT_TRUE(isRemoteFile(user1000Url));
    
    QUrl user1001Url = createLocalFileUrl("/run/user/1001/gvfs/smb-share");
    EXPECT_TRUE(isRemoteFile(user1001Url));
    
    QUrl user0Url = createLocalFileUrl("/run/user/0/gvfs/smb-share");
    EXPECT_TRUE(isRemoteFile(user0Url));
    
    QUrl user9999Url = createLocalFileUrl("/run/user/9999/gvfs/smb-share");
    EXPECT_TRUE(isRemoteFile(user9999Url));
}

/**
 * @brief Test regex pattern matching edge cases
 * Verify regex patterns work correctly with various inputs
 */
TEST_F(ProtocolUtilsTest, RegexPatterns_EdgeCases)
{
    // Test paths that should not match
    QUrl fakeGvfsUrl = createLocalFileUrl("/fake/run/user/1000/gvfs/smb-share");
    EXPECT_FALSE(isRemoteFile(fakeGvfsUrl));
    
    QUrl fakeGvfsUrl2 = createLocalFileUrl("/run/user/1000/fake-gvfs/smb-share");
    EXPECT_FALSE(isRemoteFile(fakeGvfsUrl2));
    
    // Test partial matches that should not match
    QUrl partialMatchUrl = createLocalFileUrl("/run/user/1000/gvfs-smb-share");
    EXPECT_FALSE(isRemoteFile(partialMatchUrl));
    
    // Test exact matches with additional path components
    QUrl deepGvfsUrl = createLocalFileUrl("/run/user/1000/gvfs/smb-share:server=server.com,user=user/path/to/file.txt");
    EXPECT_TRUE(isRemoteFile(deepGvfsUrl));
}

/**
 * @brief Test path vs URL differences
 * Verify that functions correctly use toLocalFile() vs path()
 */
TEST_F(ProtocolUtilsTest, PathVsUrl_Differences)
{
    // Test functions that use toLocalFile() vs path()
    // isRemoteFile uses toLocalFile()
    QUrl remoteFileUrl = createLocalFileUrl("/run/user/1000/gvfs/smb-share");
    EXPECT_TRUE(isRemoteFile(remoteFileUrl));
    
    // isSMBFile uses path() and scheme() 
    QUrl smbUrl1 = createTestUrl("file", "/run/user/1000/gvfs/smb-share");
    EXPECT_TRUE(isSMBFile(smbUrl1));
    
    QUrl smbUrl2 = createTestUrl(Global::Scheme::kSmb, "//server/share");
    EXPECT_TRUE(isSMBFile(smbUrl2));
    
    // isFTPFile and others use path()
    QUrl ftpUrl = createTestUrl("ftp", "/run/user/1000/gvfs/ftp:host=server.com");
    EXPECT_TRUE(isFTPFile(ftpUrl));
}

/**
 * @brief Test empty and special characters in URLs
 * Verify handling of edge cases with special characters
 */
TEST_F(ProtocolUtilsTest, SpecialCharacters_Handling)
{
    // Test URL with special characters in path
    QUrl specialCharsUrl = createLocalFileUrl("/run/user/1000/gvfs/smb-share/path with spaces/file name.txt");
    EXPECT_TRUE(isRemoteFile(specialCharsUrl));
    
    // Test URL with Unicode characters
    QUrl unicodeUrl = createLocalFileUrl("/run/user/1000/gvfs/smb-share/路径/文件.txt");
    EXPECT_TRUE(isRemoteFile(unicodeUrl));
    
    // Test URL with encoded characters
    QUrl encodedUrl = createLocalFileUrl("/run/user/1000/gvfs/smb-share/path%20with%20spaces/file.txt");
    EXPECT_TRUE(isRemoteFile(encodedUrl));
}
