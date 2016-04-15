#ifndef ZURL_H
#define ZURL_H

#include <QUrl>

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

    static DUrl fromLocalFile(const QString &filePath);
    static DUrl fromTrashFile(const QString &filePath);
    static DUrl fromRecentFile(const QString &filePath);
    static DUrl fromBookMarkFile(const QString &filePath);
    static DUrl fromSearchFile(const QString &filePath);
    static DUrl fromComputerFile(const QString &filePath);
    static DUrlList fromStringList(const QStringList &urls, ParsingMode mode = TolerantMode);
    static DUrlList fromQUrlList(const QList<QUrl> &urls);
    static DUrl fromUserInput(const QString &userInput);
    static DUrl fromUserInput(const QString &userInput, const QString &workingDirectory,
                              UserInputResolutionOptions options = AssumeLocalFile);
    static QStringList toStringList(const DUrlList &urls,
                                    FormattingOptions options = FormattingOptions( PrettyDecoded ));
    static QList<QUrl> toQUrlList(const DUrlList &urls);

    bool operator ==(const QUrl &url) const;

private:
    void makeAbsolute();
};

typedef QList<DUrl> DUrlList;

#endif // ZURL_H
