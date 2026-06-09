// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocolutils.cpp
 * @brief Unit tests for ProtocolUtils namespace (protocolutils.cpp)
 *
 * Coverage:
 *   - matchPath() [indirect]: via scheme-based and path-based protocols
 *   - gvfsPathStart() [indirect]: via isGvfsPath / isGvfsProtocolPath
 *   - isGvfsPath() [indirect]: via isRemoteFile
 *   - isGvfsProtocolPath() [indirect]: via isMTPFile, isGphotoFile, etc.
 *   - isMediaSmbMountPath() [indirect]: via isRemoteFile, isSMBFile
 *   - isRemoteFile()   - all branches
 *   - isMTPFile()       - scheme + gvfs path
 *   - isGphotoFile()    - scheme + gvfs path
 *   - isFTPFile()       - scheme + gvfs path (ftp + sftp)
 *   - isSFTPFile()      - scheme + gvfs path
 *   - isSMBFile()       - scheme + gvfs path + media smb
 *   - isLocalFile()     - local + remote + device mount exclusion
 *   - isNFSFile()       - scheme + gvfs path
 *   - isDavFile()       - scheme + gvfs path + ssl=false
 *   - isDavsFile()      - scheme + gvfs path + ssl=true
 *   - Invalid URLs      - all functions return false
 */

#include <gtest/gtest.h>
#include <QUrl>

#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <stub.h>

using namespace dfmbase;

// ---------------------------------------------------------------------------
// Stub DeviceProxyManager to avoid DBus dependency in isLocalFile().
// ---------------------------------------------------------------------------
static DeviceProxyManager *stubDevProxyMng = nullptr;

static DeviceProxyManager *stub_instance()
{
    if (!stubDevProxyMng)
        stubDevProxyMng = new DeviceProxyManager(nullptr);   // -fno-access-control
    return stubDevProxyMng;
}

static bool stub_isFileOfExternalBlockMounts(const QString &)
{
    return false;
}
static bool stub_isFileOfProtocolMounts(const QString &)
{
    return false;
}

__attribute__((constructor)) static void register_deviceproxymanager_stub()
{
    static Stub st;
    st.set(ADDR(DeviceProxyManager, instance), stub_instance);
    st.set(ADDR(DeviceProxyManager, isFileOfExternalBlockMounts), stub_isFileOfExternalBlockMounts);
    st.set(ADDR(DeviceProxyManager, isFileOfProtocolMounts), stub_isFileOfProtocolMounts);
}

// =============================================================================
// Test fixtures
// =============================================================================

class TestProtocolUtils : public testing::Test
{
protected:
    // Helper: build a gvfs-like path with given UID and protocol prefix.
    // e.g. gvfsPath(1000, "mtp:host") → "/run/user/1000/gvfs/mtp:host=..."
    static QString gvfsPath(int uid, const QString &protocol)
    {
        return QStringLiteral("/run/user/%1/gvfs/%2").arg(uid).arg(protocol);
    }
};

// =============================================================================
// Invalid URL — all functions must return false
// =============================================================================

class TestInvalidUrl : public TestProtocolUtils
{
};

TEST_F(TestInvalidUrl, RemoteFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestInvalidUrl, MTPFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isMTPFile(url));
}

TEST_F(TestInvalidUrl, GphotoFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isGphotoFile(url));
}

TEST_F(TestInvalidUrl, FTPFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isFTPFile(url));
}

TEST_F(TestInvalidUrl, SFTPFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isSFTPFile(url));
}

TEST_F(TestInvalidUrl, SMBFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isSMBFile(url));
}

TEST_F(TestInvalidUrl, NFSFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isNFSFile(url));
}

TEST_F(TestInvalidUrl, DavFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isDavFile(url));
}

TEST_F(TestInvalidUrl, DavsFile)
{
    QUrl url;
    EXPECT_FALSE(ProtocolUtils::isDavsFile(url));
}

// =============================================================================
// isRemoteFile
// =============================================================================

class TestIsRemoteFile : public TestProtocolUtils
{
};

TEST_F(TestIsRemoteFile, ViaKnownScheme_Smb)
{
    QUrl url("smb://192.168.1.1/share");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaKnownScheme_Ftp)
{
    QUrl url("ftp://ftp.example.com/pub");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaKnownScheme_Sftp)
{
    QUrl url("sftp://user@host/path");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaKnownScheme_Mtp)
{
    QUrl url("mtp://host");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaKnownScheme_Afc)
{
    QUrl url("afc://host");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaKnownScheme_Dav)
{
    QUrl url("dav://server/path");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaKnownScheme_Davs)
{
    QUrl url("davs://server/path");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaKnownScheme_Nfs)
{
    QUrl url("nfs://server/path");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaGvfsPath_User)
{
    QUrl url = QUrl::fromLocalFile("/run/user/1000/gvfs/smb:host=server");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaGvfsPath_Root)
{
    QUrl url = QUrl::fromLocalFile("/root/.gvfs/smb:host=server");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaMediaSmbMount)
{
    QUrl url = QUrl::fromLocalFile("/media/zhangs/smbmounts/share");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, ViaRunMediaSmbMount)
{
    QUrl url = QUrl::fromLocalFile("/run/media/zhangs/smbmounts/share");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, LocalFileIsNotRemote)
{
    QUrl url = QUrl::fromLocalFile("/home/zhangs/document.txt");
    EXPECT_FALSE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, GvfsPathNoDigitUid_ReturnsFalse)
{
    // "/run/user/abc/gvfs/..." → not a valid UID (no digits)
    QUrl url = QUrl::fromLocalFile("/run/user/abc/gvfs/smb:host=server");
    EXPECT_FALSE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestIsRemoteFile, GvfsPathNoGvfsSuffix_ReturnsFalse)
{
    // "/run/user/1000/other/..." → not a gvfs path
    QUrl url = QUrl::fromLocalFile("/run/user/1000/other/smb:host=server");
    EXPECT_FALSE(ProtocolUtils::isRemoteFile(url));
}

// =============================================================================
// isMTPFile
// =============================================================================

class TestIsMTPFile : public TestProtocolUtils
{
};

TEST_F(TestIsMTPFile, ViaScheme)
{
    QUrl url("mtp://host");
    EXPECT_TRUE(ProtocolUtils::isMTPFile(url));
}

TEST_F(TestIsMTPFile, ViaGvfsPath)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "mtp:host=usb123"));
    EXPECT_TRUE(ProtocolUtils::isMTPFile(url));
}

TEST_F(TestIsMTPFile, ViaRootGvfsPath)
{
    QUrl url = QUrl::fromLocalFile("/root/.gvfs/mtp:host=usb123");
    EXPECT_TRUE(ProtocolUtils::isMTPFile(url));
}

TEST_F(TestIsMTPFile, NotMTP)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "smb:host=server"));
    EXPECT_FALSE(ProtocolUtils::isMTPFile(url));
}

// =============================================================================
// isGphotoFile
// =============================================================================

class TestIsGphotoFile : public TestProtocolUtils
{
};

TEST_F(TestIsGphotoFile, ViaSchemeGphoto)
{
    QUrl url("gphoto://host");
    EXPECT_TRUE(ProtocolUtils::isGphotoFile(url));
}

TEST_F(TestIsGphotoFile, ViaSchemeGphoto2)
{
    QUrl url("gphoto2://host");
    EXPECT_TRUE(ProtocolUtils::isGphotoFile(url));
}

TEST_F(TestIsGphotoFile, ViaGvfsPath)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "gphoto2:host=camera"));
    EXPECT_TRUE(ProtocolUtils::isGphotoFile(url));
}

TEST_F(TestIsGphotoFile, NotGphoto)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "mtp:host=device"));
    EXPECT_FALSE(ProtocolUtils::isGphotoFile(url));
}

// =============================================================================
// isFTPFile
// =============================================================================

class TestIsFTPFile : public TestProtocolUtils
{
};

TEST_F(TestIsFTPFile, ViaScheme)
{
    QUrl url("ftp://ftp.example.com/pub");
    EXPECT_TRUE(ProtocolUtils::isFTPFile(url));
}

TEST_F(TestIsFTPFile, ViaGvfsPathFtp)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "ftp:host=server"));
    EXPECT_TRUE(ProtocolUtils::isFTPFile(url));
}

TEST_F(TestIsFTPFile, ViaGvfsPathSftp)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "sftp:host=server"));
    EXPECT_TRUE(ProtocolUtils::isFTPFile(url));
}

TEST_F(TestIsFTPFile, NotFtp)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "smb:host=server"));
    EXPECT_FALSE(ProtocolUtils::isFTPFile(url));
}

// =============================================================================
// isSFTPFile
// =============================================================================

class TestIsSFTPFile : public TestProtocolUtils
{
};

TEST_F(TestIsSFTPFile, ViaScheme)
{
    QUrl url("sftp://user@host/path");
    EXPECT_TRUE(ProtocolUtils::isSFTPFile(url));
}

TEST_F(TestIsSFTPFile, ViaGvfsPath)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "sftp:host=server"));
    EXPECT_TRUE(ProtocolUtils::isSFTPFile(url));
}

TEST_F(TestIsSFTPFile, FtpNotSftp)
{
    // isSFTPFile should NOT match plain "ftp" gvfs prefix
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "ftp:host=server"));
    EXPECT_FALSE(ProtocolUtils::isSFTPFile(url));
}

// =============================================================================
// isSMBFile
// =============================================================================

class TestIsSMBFile : public TestProtocolUtils
{
};

TEST_F(TestIsSMBFile, ViaScheme)
{
    QUrl url("smb://192.168.1.1/share");
    EXPECT_TRUE(ProtocolUtils::isSMBFile(url));
}

TEST_F(TestIsSMBFile, ViaGvfsPath)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "smb:host=server,share=data"));
    EXPECT_TRUE(ProtocolUtils::isSMBFile(url));
}

TEST_F(TestIsSMBFile, ViaMediaSmbMount)
{
    QUrl url = QUrl::fromLocalFile("/media/zhangs/smbmounts/share");
    EXPECT_TRUE(ProtocolUtils::isSMBFile(url));
}

TEST_F(TestIsSMBFile, ViaRunMediaSmbMount)
{
    QUrl url = QUrl::fromLocalFile("/run/media/zhangs/smbmounts/share");
    EXPECT_TRUE(ProtocolUtils::isSMBFile(url));
}

TEST_F(TestIsSMBFile, NotSmb)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "ftp:host=server"));
    EXPECT_FALSE(ProtocolUtils::isSMBFile(url));
}

TEST_F(TestIsSMBFile, MediaPathWithoutSmbmounts_NotSmb)
{
    // "/media/zhangs/usb" contains "/media/" but no "/smbmounts"
    QUrl url = QUrl::fromLocalFile("/media/zhangs/usb/data");
    EXPECT_FALSE(ProtocolUtils::isSMBFile(url));
}

// =============================================================================
// isNFSFile
// =============================================================================

class TestIsNFSFile : public TestProtocolUtils
{
};

TEST_F(TestIsNFSFile, ViaScheme)
{
    QUrl url("nfs://server/path");
    EXPECT_TRUE(ProtocolUtils::isNFSFile(url));
}

TEST_F(TestIsNFSFile, ViaGvfsPath)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "nfs:host=server,path=/export/data"));
    EXPECT_TRUE(ProtocolUtils::isNFSFile(url));
}

TEST_F(TestIsNFSFile, NotNfs)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "smb:host=server"));
    EXPECT_FALSE(ProtocolUtils::isNFSFile(url));
}

// =============================================================================
// isDavFile / isDavsFile — SSL flag discrimination
// =============================================================================

class TestIsDavFile : public TestProtocolUtils
{
};

TEST_F(TestIsDavFile, ViaScheme)
{
    QUrl url("dav://server/path");
    EXPECT_TRUE(ProtocolUtils::isDavFile(url));
}

TEST_F(TestIsDavFile, ViaGvfsPath_WithSslFalse)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "dav:host=server,ssl=false"));
    EXPECT_TRUE(ProtocolUtils::isDavFile(url));
}

TEST_F(TestIsDavFile, ViaGvfsPath_WithSslTrue_IsDavNotDavs)
{
    // ssl=true → isDavsFile, not isDavFile
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "dav:host=server,ssl=true"));
    EXPECT_FALSE(ProtocolUtils::isDavFile(url));
}

TEST_F(TestIsDavFile, NotDav)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "smb:host=server"));
    EXPECT_FALSE(ProtocolUtils::isDavFile(url));
}

class TestIsDavsFile : public TestProtocolUtils
{
};

TEST_F(TestIsDavsFile, ViaScheme)
{
    QUrl url("davs://server/path");
    EXPECT_TRUE(ProtocolUtils::isDavsFile(url));
}

TEST_F(TestIsDavsFile, ViaGvfsPath_WithSslTrue)
{
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "dav:host=server,ssl=true"));
    EXPECT_TRUE(ProtocolUtils::isDavsFile(url));
}

TEST_F(TestIsDavsFile, ViaGvfsPath_WithSslFalse_IsDavsNotDav)
{
    // ssl=false → isDavFile, not isDavsFile
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "dav:host=server,ssl=false"));
    EXPECT_FALSE(ProtocolUtils::isDavsFile(url));
}

// =============================================================================
// isLocalFile — depends on stubbed DevProxyMng (returns false for mounts)
// =============================================================================

class TestIsLocalFile : public TestProtocolUtils
{
};

TEST_F(TestIsLocalFile, OrdinaryLocalFile)
{
    QUrl url = QUrl::fromLocalFile("/home/zhangs/documents/readme.md");
    EXPECT_TRUE(ProtocolUtils::isLocalFile(url));
}

TEST_F(TestIsLocalFile, RootPath)
{
    QUrl url = QUrl::fromLocalFile("/");
    EXPECT_TRUE(ProtocolUtils::isLocalFile(url));
}

TEST_F(TestIsLocalFile, RemoteSchemeIsNotLocal)
{
    QUrl url("smb://server/share");
    EXPECT_FALSE(ProtocolUtils::isLocalFile(url));
}

TEST_F(TestIsLocalFile, GvfsPathIsNotLocal)
{
    // DevProxyMng stub returns false → isRemoteFile takes precedence
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "smb:host=server"));
    EXPECT_FALSE(ProtocolUtils::isLocalFile(url));
}

// =============================================================================
// Edge cases — boundary conditions for gvfsPathStart
// =============================================================================

class TestGvfsPathEdgeCases : public TestProtocolUtils
{
};

TEST_F(TestGvfsPathEdgeCases, MultiDigitUid)
{
    // UID like 1000, 10000, etc. should all parse correctly
    QUrl url = QUrl::fromLocalFile("/run/user/10000/gvfs/smb:host=server");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestGvfsPathEdgeCases, ZeroUid)
{
    // UID 0 → root user, but through /run/user path
    QUrl url = QUrl::fromLocalFile("/run/user/0/gvfs/smb:host=server");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestGvfsPathEdgeCases, RootPrefix_WithoutGvfsSubdir)
{
    // "/root/.gvfs" alone without protocol prefix → still a gvfs path
    QUrl url = QUrl::fromLocalFile("/root/.gvfs/");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestGvfsPathEdgeCases, EmptyScheme_GvfsPath_IsRemote)
{
    // Empty scheme but path is a valid gvfs mount path — still detected as remote.
    // QUrl::isValid() returns true for empty scheme URLs.
    QUrl url;
    url.setPath("/run/user/1000/gvfs/smb:host=server");
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestGvfsPathEdgeCases, FileUrl_WithNonLocalPath)
{
    // QUrl with file:// scheme but the path contains gvfs
    QUrl url("file:///run/user/1000/gvfs/smb:host=server");
    // url.isLocalFile() returns true → toLocalFile() is used
    EXPECT_TRUE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestGvfsPathEdgeCases, UidOnly_NoGvfsSuffix)
{
    // "/run/user/1000/other" → not a gvfs path
    QUrl url = QUrl::fromLocalFile("/run/user/1000/other/path");
    EXPECT_FALSE(ProtocolUtils::isRemoteFile(url));
}

TEST_F(TestGvfsPathEdgeCases, ProtocolPrefixCaseSensitive)
{
    // gvfs protocol prefixes are case-sensitive (all lowercase)
    QUrl url = QUrl::fromLocalFile(gvfsPath(1000, "MTP:host=device"));
    EXPECT_FALSE(ProtocolUtils::isMTPFile(url));
}

// =============================================================================
// Non-file URLs — path() vs toLocalFile() behavior
// =============================================================================

class TestNonLocalUrl : public TestProtocolUtils
{
};

TEST_F(TestNonLocalUrl, SftpUrl_UsesUrlPath)
{
    // For non-local URLs, matchPath returns url.path() not url.toLocalFile()
    // sftp://host/path → url.path() = "/path"
    QUrl url("sftp://host/gvfs/sftp:host=server");
    // Not a gvfs-style path (no /run/user/ prefix)
    EXPECT_TRUE(ProtocolUtils::isSFTPFile(url));   // true via scheme
}

TEST_F(TestNonLocalUrl, DavUrlWithSslInPath)
{
    // dav://host/dav:host=server,ssl=false → url.path() = "/dav:host=server,ssl=false"
    // Not a gvfs-style path, so only scheme match
    QUrl url("dav://host/dav:host=server,ssl=false");
    EXPECT_TRUE(ProtocolUtils::isDavFile(url));   // true via scheme
}
