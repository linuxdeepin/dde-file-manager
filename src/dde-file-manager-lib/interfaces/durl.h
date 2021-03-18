/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef ZURL_H
#define ZURL_H

#include <QUrl>
#include <QMetaType>
#include <QRegularExpression>


#define TRASH_SCHEME "trash"
#define RECENT_SCHEME "recent"
#define BOOKMARK_SCHEME "bookmark"
#define FILE_SCHEME "file"
#define COMPUTER_SCHEME "computer"
#define SEARCH_SCHEME "search"
#define NETWORK_SCHEME "network"
#define SMB_SCHEME "smb"
#define AFC_SCHEME "afc"
#define MTP_SCHEME "mtp"
#define DAV_SCHEME "dav"
#define GPHOTO2_SCHEME "gphoto2"
#define USERSHARE_SCHEME "usershare"
#define AVFS_SCHEME "avfs"
#define FTP_SCHEME "ftp"
#define SFTP_SCHEME "sftp"
#define TAG_SCHEME "tag"
#define DEVICE_SCHEME "device"
#define MOUNT_SCHEME "mount"
#define BURN_SCHEME "burn"

#define TRASH_ROOT "trash:///"
#define RECENT_ROOT "recent:///"
#define BOOKMARK_ROOT "bookmark:///"
#define FILE_ROOT  "file:///"
#define COMPUTER_ROOT "computer:///"
#define NETWORK_ROOT "network:///"
#define SMB_ROOT "smb:///"
#define USERSHARE_ROOT "usershare:///"
#define AVFS_ROOT "avfs:///"
#define TAG_ROOT "tag:///"
#define DEVICE_ROOT "device:///"
#define MOUNT_ROOT "mount:///"
#define BURN_ROOT "burn:///"

// internal scheme, don't rely on this scheme outside the file manager app.
#define DFMMD_SCHEME "dfmmd"
#define DFMMD_ROOT "dfmmd:///"
#define DFMROOT_SCHEME "dfmroot"
#define DFMROOT_ROOT "dfmroot:///"
#define DFMVAULT_SCHEME "dfmvault"
#define DFMVAULT_ROOT "dfmvault:///"
// internal scheme end.

// Auxiliary path segment strings for the burn scheme
#define BURN_SEG_ONDISC "disc_files"
#define BURN_SEG_STAGING "staging_files"

#define GVFS_ROOT_MATCH "(^//run/user/\\d+/gvfs/|^//root/.gvfs/)"
#define GVFS_MATCH "(^/run/user/\\d+/gvfs/|^/root/.gvfs/)"
#define FTP_HOST "ftp:host="
#define GVFS_MATCH_EX "(^/run/user/\\d+/gvfs/.+$|^/root/.gvfs/.+$)"

class DUrl;

QT_BEGIN_NAMESPACE
Q_CORE_EXPORT uint qHash(const DUrl &url, uint seed = 0) Q_DECL_NOTHROW;
QT_END_NAMESPACE

class DUrl : public QUrl
{
    typedef QList<DUrl> DUrlList;

public:
    DUrl();
    explicit DUrl(const QUrl &copy);

    virtual ~DUrl() = default;
    DUrl(const DUrl &other);
    DUrl(DUrl &&other);
    DUrl &operator=(const DUrl &other);
    DUrl &operator=(DUrl &&other);

    QDataStream &operator<<(QDataStream &out);
    QDataStream &operator>>(QDataStream &in);

    DUrl(const QString &url, ParsingMode mode = TolerantMode);

    void setPath(const QString &path, ParsingMode mode = DecodedMode, bool makeAbsolutePath = true);
    void setScheme(const QString &scheme, bool makeAbsolutePath = true);
    void setUrl(const QString &url, ParsingMode parsingMode = TolerantMode, bool makeAbsolutePath = true);

    bool isTrashFile() const;
    bool isRecentFile() const;
    bool isBookMarkFile() const;
    bool isSearchFile() const;
    bool isComputerFile() const;
    bool isNetWorkFile() const;
    bool isSMBFile() const;
    bool isAFCFile() const;
    bool isMTPFile() const;
    bool isUserShareFile() const;
    bool isAVFSFile() const;
    bool isFTPFile() const;
    bool isSFTPFile() const;
    bool isTaggedFile() const;
    bool isVaultFile() const;

    QString toString(FormattingOptions options = FormattingOptions(PrettyDecoded)) const;

    QString searchKeyword() const;
    DUrl searchTargetUrl() const;
    DUrl searchedFileUrl() const;
    QString taggedLocalFilePath() const noexcept;
    QString tagName() const noexcept;
    QString fileName(ComponentFormattingOptions options = FullyDecoded) const; // not virtual
    QString deviceId() const;
    DUrl bookmarkTargetUrl() const;
    QString bookmarkName() const;
    QString burnDestDevice() const;
    QString burnFilePath() const;
    bool burnIsOnDisc() const;

    DUrl parentUrl() const;

    void setSearchKeyword(const QString &keyword);
    void setSearchTargetUrl(const DUrl &url);
    void setSearchedFileUrl(const DUrl &url);
    void setTaggedFileUrl(const QString &localFilePath) noexcept;
    void setBookmarkName(const QString &name);

    static DUrl fromLocalFile(const QString &filePath);
    static DUrl fromTrashFile(const QString &filePath);
    static DUrl fromRecentFile(const QString &filePath);
    static DUrl fromBookMarkFile(const DUrl &targetUrl, const QString &name);
    static DUrl fromSearchFile(const QString &filePath);
    static DUrl fromSearchFile(const DUrl &targetUrl, const QString &keyword, const DUrl &searchedFileUrl = DUrl());
    static DUrl fromComputerFile(const QString &filePath);
    static DUrl fromNetworkFile(const QString &filePath);
    static DUrl fromSMBFile(const QString &filePath);
    static DUrl fromAFCFile(const QString &filePath);
    static DUrl fromMTPFile(const QString &filePath);
    static DUrl fromUserShareFile(const QString &filePath);
    static DUrl fromAVFSFile(const QString &filePath);
    static DUrl fromUserTaggedFile(const QString &tag_name, const QString &localFilePath) noexcept;
    static DUrl fromDeviceId(const QString &deviceId);
    static DUrl fromBurnFile(const QString &filePath);
    static DUrl fromVaultFile(const QString &filePath);

    static DUrlList fromStringList(const QStringList &urls, ParsingMode mode = TolerantMode);
    static DUrlList fromQUrlList(const QList<QUrl> &urls);
    static DUrl fromUserInput(const QString &userInput, bool preferredLocalPath = true);

    // Return "file://$CURRENT_DIR/userInput" if preferredLocalPath is true and directory exists
    static DUrl fromUserInput(const QString &userInput, QString workingDirectory,
                              bool preferredLocalPath = true, UserInputResolutionOptions options = AssumeLocalFile);
    static QStringList toStringList(const DUrlList &urls,
                                    FormattingOptions options = FormattingOptions(PrettyDecoded));
    static QList<QUrl> toQUrlList(const DUrlList &urls);

    static DUrlList childrenList(const DUrl &url);
    static DUrl parentUrl(const DUrl &url);
    static bool hasScheme(const QString &scheme);

    bool operator ==(const DUrl &url) const;
    inline bool operator !=(const DUrl &url) const
    {
        return !operator ==(url);
    }
    friend Q_CORE_EXPORT uint qHash(const DUrl &url, uint seed) Q_DECL_NOTHROW;

    void makeAbsolutePath();
    DUrl toAbsolutePathUrl() const;
    QString toLocalFile() const;

    friend QDataStream &operator<<(QDataStream &out, const DUrl &url);
    friend QDataStream &operator>>(QDataStream &in, DUrl &url);

private:
    void updateVirtualPath();

    QString m_virtualPath;

    static QRegularExpression burn_rxp;
};

typedef QList<DUrl> DUrlList;

Q_DECLARE_METATYPE(DUrl)
Q_DECLARE_METATYPE(DUrlList)

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DUrl &url);
QDataStream &operator<<(QDataStream &out, const DUrl &url);
QDataStream &operator>>(QDataStream &in, DUrl &url);
QT_END_NAMESPACE

#endif // ZURL_H
