#ifndef ZURL_H
#define ZURL_H

#include <QUrl>
#include <QMetaType>

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
#define GPHOTO2_SCHEME "gphoto2"
#define USERSHARE_SCHEME "usershare"
#define AVFS_SCHEME "avfs"
#define FTP_SCHEME "ftp"
#define SFTP_SCHEME "sftp"

#define TRASH_ROOT "trash:///"
#define RECENT_ROOT "recent:///"
#define BOOKMARK_ROOT "bookmark:///"
#define FILE_ROOT  "file:///"
#define COMPUTER_ROOT "computer:///"
#define NETWORK_ROOT "network:///"
#define SMB_ROOT "smb:///"
#define USERSHARE_ROOT "usershare:///"
#define AVFS_ROOT "avfs:///"

class DUrl;

QT_BEGIN_NAMESPACE
Q_CORE_EXPORT uint qHash(const DUrl &url, uint seed = 0) Q_DECL_NOTHROW;
QT_END_NAMESPACE

class DUrl : public QUrl
{
    typedef QList<DUrl> DUrlList;

public:
    DUrl();
    DUrl(const QUrl &copy);
#ifdef QT_NO_URL_CAST_FROM_STRING
    explicit DUrl(const QString &url, ParsingMode mode = TolerantMode);
#else
    explicit DUrl(const QString &url, ParsingMode mode = TolerantMode);
#endif

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

    QString toString(FormattingOptions options = FormattingOptions( PrettyDecoded )) const;

    QString searchKeyword() const;
    DUrl searchTargetUrl() const;
    DUrl searchedFileUrl() const;

    DUrl parentUrl() const;

    void setSearchKeyword(const QString &keyword);
    void setSearchTargetUrl(const DUrl &url);
    void setSearchedFileUrl(const DUrl &url);

    static DUrl fromLocalFile(const QString &filePath);
    static DUrl fromTrashFile(const QString &filePath);
    static DUrl fromRecentFile(const QString &filePath);
    static DUrl fromBookMarkFile(const QString &filePath);
    static DUrl fromSearchFile(const QString &filePath);
    static DUrl fromSearchFile(const DUrl &targetUrl, const QString &keyword, const DUrl &searchedFileUrl = DUrl());
    static DUrl fromComputerFile(const QString &filePath);
    static DUrl fromNetworkFile(const QString &filePath);
    static DUrl fromSMBFile(const QString &filePath);
    static DUrl fromAFCFile(const QString &filePath);
    static DUrl fromMTPFile(const QString &filePath);
    static DUrl fromUserShareFile(const QString &filePath);
    static DUrl fromAVFSFile(const QString& filePath);
    static DUrlList fromStringList(const QStringList &urls, ParsingMode mode = TolerantMode);
    static DUrlList fromQUrlList(const QList<QUrl> &urls);
    static DUrl fromUserInput(const QString &userInput, bool preferredLocalPath = true);
    // Return "file://$CURRENT_DIR/userInput" if preferredLocalPath is true and directory exists
    static DUrl fromUserInput(const QString &userInput, QString workingDirectory,
                              bool preferredLocalPath = true, UserInputResolutionOptions options = AssumeLocalFile);
    static QStringList toStringList(const DUrlList &urls,
                                    FormattingOptions options = FormattingOptions( PrettyDecoded ));
    static QList<QUrl> toQUrlList(const DUrlList &urls);

    static DUrlList childrenList(const DUrl &url);
    static DUrl parentUrl(const DUrl &url);
    static bool hasScheme(const QString &scheme);

    bool operator ==(const DUrl &url) const;
    inline bool operator !=(const DUrl &url) const
    { return !operator ==(url);}
    friend Q_CORE_EXPORT uint qHash(const DUrl &url, uint seed) Q_DECL_NOTHROW;

    void makeAbsolutePath();
    DUrl toAbsolutePathUrl() const;
    QString toLocalFile() const;

private:
    void updateVirtualPath();

    QString m_virtualPath;
};

typedef QList<DUrl> DUrlList;

Q_DECLARE_METATYPE(DUrl)
Q_DECLARE_METATYPE(DUrlList)

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DUrl &url);
QT_END_NAMESPACE

#endif // ZURL_H
