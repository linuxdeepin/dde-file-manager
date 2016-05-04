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
    DUrl();
    DUrl(const QUrl &copy);
#ifdef QT_NO_URL_CAST_FROM_STRING
    explicit ZUrl(const QString &url, ParsingMode mode = TolerantMode);
#else
    explicit DUrl(const QString &url, ParsingMode mode = TolerantMode);
#endif

    void setPath(const QString &path, ParsingMode mode = DecodedMode, bool makeAbsolute = true);
    void setScheme(const QString &scheme, bool makeAbsolute = true);
    void setUrl(const QString &url, ParsingMode parsingMode = TolerantMode, bool makeAbsolute = true);

    bool isTrashFile() const;
    bool isRecentFile() const;
    bool isBookMarkFile() const;
    bool isSearchFile() const;
    bool isComputerFile() const;

    QString toString(FormattingOptions options = FormattingOptions( PrettyDecoded )) const;

    static DUrl fromLocalFile(const QString &filePath);
    static DUrl fromTrashFile(const QString &filePath);
    static DUrl fromRecentFile(const QString &filePath);
    static DUrl fromBookMarkFile(const QString &filePath);
    static DUrl fromSearchFile(const QString &filePath, const QString &keyword = QString());
    static DUrl fromComputerFile(const QString &filePath);
    static DUrlList fromStringList(const QStringList &urls, ParsingMode mode = TolerantMode);
    static DUrlList fromQUrlList(const QList<QUrl> &urls);
    static DUrl fromUserInput(const QString &userInput);
    static DUrl fromUserInput(const QString &userInput, const QString &workingDirectory,
                              UserInputResolutionOptions options = AssumeLocalFile);
    static QStringList toStringList(const DUrlList &urls,
                                    FormattingOptions options = FormattingOptions( PrettyDecoded ));
    static QList<QUrl> toQUrlList(const DUrlList &urls);

    static DUrlList childrenList(const DUrl &url);

    bool operator ==(const DUrl &url) const;
    friend Q_CORE_EXPORT uint qHash(const DUrl &url, uint seed) Q_DECL_NOTHROW;

private:
    void makeAbsolute();
    void updateVirtualPath();

    QString m_virtualPath;
};

typedef QList<DUrl> DUrlList;

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DUrl &url);
QT_END_NAMESPACE

#endif // ZURL_H
