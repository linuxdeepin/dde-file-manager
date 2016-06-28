#ifndef ZURL_H
#define ZURL_H

#include <QUrl>

class DUrl;

QT_BEGIN_NAMESPACE
Q_CORE_EXPORT uint qHash(const DUrl &url, uint seed = 0) Q_DECL_NOTHROW;
QT_END_NAMESPACE

class DUrl : public QUrl
{
    typedef QList<DUrl> DUrlList;

public:
    enum SearchAction {
        StartSearch,
        StopSearch
    };

    DUrl();
    DUrl(const QUrl &copy);
#ifdef QT_NO_URL_CAST_FROM_STRING
    explicit ZUrl(const QString &url, ParsingMode mode = TolerantMode);
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

    QString toString(FormattingOptions options = FormattingOptions( PrettyDecoded )) const;

    QString searchKeyword() const;
    SearchAction searchAction() const;
    DUrl searchTargetUrl() const;

    void setSearchKeyword(const QString &keyword);
    void setSearchAction(SearchAction action);
    void setSearchTargetUrl(const DUrl &url);

    inline bool isStopSearch() const
    { return searchAction() == StopSearch;}

    static DUrl fromLocalFile(const QString &filePath);
    static DUrl fromTrashFile(const QString &filePath);
    static DUrl fromRecentFile(const QString &filePath);
    static DUrl fromBookMarkFile(const QString &filePath);
    static DUrl fromSearchFile(const QString &filePath);
    static DUrl fromSearchFile(const DUrl &targetUrl, const QString &keyword, SearchAction action = StartSearch);
    static DUrl fromComputerFile(const QString &filePath);
    static DUrl fromNetworkFile(const QString &filePath);
    static DUrl fromAFCFile(const QString &filePath);
    static DUrl fromMTPFile(const QString &filePath);
    static DUrlList fromStringList(const QStringList &urls, ParsingMode mode = TolerantMode);
    static DUrlList fromQUrlList(const QList<QUrl> &urls);
    static DUrl fromUserInput(const QString &userInput);
    static DUrl fromUserInput(const QString &userInput, const QString &workingDirectory,
                              UserInputResolutionOptions options = AssumeLocalFile);
    static QStringList toStringList(const DUrlList &urls,
                                    FormattingOptions options = FormattingOptions( PrettyDecoded ));
    static QList<QUrl> toQUrlList(const DUrlList &urls);

    static DUrlList childrenList(const DUrl &url);
    static DUrl parentUrl(const DUrl &url);

    bool operator ==(const DUrl &url) const;
    inline bool operator !=(const DUrl &url) const
    { return !operator ==(url);}
    friend Q_CORE_EXPORT uint qHash(const DUrl &url, uint seed) Q_DECL_NOTHROW;

    void makeAbsolutePath();
    DUrl toAbsolutePathUrl() const;

private:
    void updateVirtualPath();

    QString m_virtualPath;
};

typedef QList<DUrl> DUrlList;

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DUrl &url);
QT_END_NAMESPACE

#endif // ZURL_H
