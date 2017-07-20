#include "durl.h"
#include "interfaces/dfmstandardpaths.h"

#include <QFileInfo>
#include <QSet>
#include <QDir>
#include <QDebug>
#include <QUrlQuery>

QSet<QString> schemeList = QSet<QString>() << QString(TRASH_SCHEME)
                                           << QString(RECENT_SCHEME)
                                           << QString(BOOKMARK_SCHEME)
                                           << QString(FILE_SCHEME)
                                           << QString(COMPUTER_SCHEME )
                                           << QString(SEARCH_SCHEME)
                                           << QString(NETWORK_SCHEME)
                                           << QString(SMB_SCHEME)
                                           << QString(AFC_SCHEME)
                                           << QString(MTP_SCHEME)
                                           << QString(USERSHARE_SCHEME)
                                           << QString(AVFS_SCHEME)
                                           << QString(FTP_SCHEME)
                                           << QString(SFTP_SCHEME);

DUrl::DUrl()
    : QUrl()
{

}

DUrl::DUrl(const QUrl &copy)
    : QUrl(copy)
{
    updateVirtualPath();
}

DUrl::DUrl(const QString &url, QUrl::ParsingMode mode)
    : QUrl(url, mode)
{
    updateVirtualPath();
}

void DUrl::setPath(const QString &path, QUrl::ParsingMode mode, bool makeAbsolute)
{
    QUrl::setPath(path, mode);

    if(makeAbsolute)
        this->makeAbsolutePath();

    updateVirtualPath();
}

void DUrl::setScheme(const QString &scheme, bool makeAbsolute)
{
    QUrl::setScheme(scheme);

    if(makeAbsolute)
        this->makeAbsolutePath();

    updateVirtualPath();
}

void DUrl::setUrl(const QString &url, QUrl::ParsingMode parsingMode, bool makeAbsolute)
{
    QUrl::setUrl(url, parsingMode);

    if(makeAbsolute)
        this->makeAbsolutePath();

    updateVirtualPath();
}

bool DUrl::isTrashFile() const
{
    return scheme() == TRASH_SCHEME;
}

bool DUrl::isRecentFile() const
{
    return scheme() == RECENT_SCHEME;
}

bool DUrl::isBookMarkFile() const
{
    return scheme() == BOOKMARK_SCHEME;
}

bool DUrl::isSearchFile() const
{
    return scheme() == SEARCH_SCHEME;
}

bool DUrl::isComputerFile() const
{
    return scheme() == COMPUTER_SCHEME;
}

bool DUrl::isNetWorkFile() const
{
    return scheme() == NETWORK_SCHEME;
}

bool DUrl::isSMBFile() const
{
    return scheme() == SMB_SCHEME;
}

bool DUrl::isAFCFile() const
{
    return scheme() == AFC_SCHEME;
}

bool DUrl::isMTPFile() const
{
    return scheme() == MTP_SCHEME;
}

bool DUrl::isUserShareFile() const
{
    return scheme() == USERSHARE_SCHEME;
}

bool DUrl::isAVFSFile() const
{
    return scheme() == AVFS_SCHEME;
}

bool DUrl::isFTPFile() const
{
    return scheme() == FTP_SCHEME;
}

bool DUrl::isSFTPFile() const
{
    return scheme() == SFTP_SCHEME;
}

QString DUrl::toString(QUrl::FormattingOptions options) const
{
    if(isLocalFile() || !schemeList.contains(scheme()))
        return QUrl::toString(options);

    QUrl url(*this);

    url.setScheme(FILE_SCHEME);

    return url.toString(options).replace(0, 4, scheme());
}

QString DUrl::searchKeyword() const
{
    if(!isSearchFile())
        return QString();

    QUrlQuery query(this->query());

    return query.queryItemValue("keyword", FullyDecoded);
}

DUrl DUrl::searchTargetUrl() const
{
    if (!isSearchFile())
        return DUrl();

    QUrlQuery query(this->query());

    return DUrl(query.queryItemValue("url", FullyDecoded));
}

DUrl DUrl::searchedFileUrl() const
{
    if (!isSearchFile())
        return DUrl();

    return DUrl(fragment(FullyDecoded));
}

DUrl DUrl::parentUrl() const
{
    return parentUrl(*this);
}

void DUrl::setSearchKeyword(const QString &keyword)
{
    if(!isSearchFile())
        return;

    QUrlQuery query(this->query());

    query.removeQueryItem("keyword");
    query.addQueryItem("keyword", QUrl::toPercentEncoding(keyword, QByteArray(), "%"));

    setQuery(query);
}

void DUrl::setSearchTargetUrl(const DUrl &url)
{
    if(!isSearchFile())
        return;

    QUrlQuery query(this->query());

    query.removeQueryItem("url");
    query.addQueryItem("url", url.toString());

    setQuery(query);
}

void DUrl::setSearchedFileUrl(const DUrl &url)
{
    if (!isSearchFile())
        return;

    setFragment(url.toString(QUrl::FullyEncoded), QUrl::DecodedMode);
}

DUrl DUrl::fromLocalFile(const QString &filePath)
{
    return QUrl::fromLocalFile(filePath);
}

DUrl DUrl::fromTrashFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(TRASH_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromRecentFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(RECENT_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromBookMarkFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(BOOKMARK_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromSearchFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(SEARCH_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromSearchFile(const DUrl &targetUrl, const QString &keyword, const DUrl &searchedFileUrl)
{
    DUrl url = fromSearchFile(QString());

    QUrlQuery query;

    query.addQueryItem("url", targetUrl.toString());
    query.addQueryItem("keyword", QUrl::toPercentEncoding(keyword, QByteArray(), "%"));

    url.setQuery(query);

    if (searchedFileUrl.isValid())
        url.setFragment(searchedFileUrl.toString());

    return url;
}

DUrl DUrl::fromComputerFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(COMPUTER_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromNetworkFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(NETWORK_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromSMBFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(SMB_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromAFCFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(AFC_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromMTPFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(MTP_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromUserShareFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(USERSHARE_SCHEME);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromAVFSFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(AVFS_SCHEME);
    url.setPath(filePath);

    return url;
}

DUrlList DUrl::fromStringList(const QStringList &urls, QUrl::ParsingMode mode)
{
    QList<DUrl> urlList;

    for(const QString &string : urls) {
        urlList << DUrl(string, mode);
    }

    return urlList;
}

DUrlList DUrl::fromQUrlList(const QList<QUrl> &urls)
{
    QList<DUrl> urlList;

    for(const QUrl &url : urls) {
        urlList << url;
    }

    return urlList;
}

DUrl DUrl::fromUserInput(const QString &userInput, bool preferredLocalPath)
{
    return fromUserInput(userInput, QString(), preferredLocalPath);
}

DUrl DUrl::fromUserInput(const QString &userInput, QString workingDirectory,
                         bool preferredLocalPath, QUrl::UserInputResolutionOptions options)
{
    if (options != AssumeLocalFile)
        return QUrl::fromUserInput(userInput, workingDirectory, options);

    if ((userInput.startsWith("~") && preferredLocalPath) || userInput.startsWith("~/")) {
        return DUrl::fromLocalFile(QDir::homePath() + userInput.mid(1));
    } else if ((preferredLocalPath && QDir().exists(userInput)) || userInput.startsWith("./")
               || userInput.startsWith("../") || userInput.startsWith("/")) {
        QDir dir(userInput);

        return DUrl::fromLocalFile(dir.absolutePath());
    } else {
        return DUrl(userInput);
    }
}

QStringList DUrl::toStringList(const DUrlList &urls, QUrl::FormattingOptions options)
{
    QStringList urlList;

    for(const DUrl &url : urls) {
        urlList << url.toString(options);
    }

    return urlList;
}

QList<QUrl> DUrl::toQUrlList(const DUrlList &urls)
{
    QList<QUrl> urlList;

    for(const DUrl &url : urls) {
        urlList << url;
    }

    return urlList;
}

DUrl::DUrlList DUrl::childrenList(const DUrl &url)
{
    DUrlList children;
    QStringList paths = url.path().split("/");
    paths.removeAt(0);
    QString _path;
    foreach (QString p, paths) {
        DUrl _url;
        _url.setScheme(url.scheme());
        _path += "/" + p;
        _url.setPath(_path);
        children.append(_url);
    }

    return children;
}

DUrl DUrl::parentUrl(const DUrl &url)
{
    DUrl _url;
    const QString &path = url.path();

    if (path == "/")
        return DUrl();

    _url.setScheme(url.scheme());
    QStringList paths = path.split("/");
    paths.removeAt(0);
    if (!paths.isEmpty() && paths.last().isEmpty())
        paths.removeLast();
    if (!paths.isEmpty())
        paths.removeLast();
    QString _path;
    foreach (QString p, paths) {
        _path += "/" + p;
    }
    if (_path.isEmpty()){
        _path += "/";
    }
    _url.setPath(_path);

    return _url;
}

bool DUrl::hasScheme(const QString &scheme)
{
    return schemeList.contains(scheme);
}

bool DUrl::operator ==(const DUrl &url) const
{
    if (!hasScheme(url.scheme()))
        return QUrl::operator ==(url);

    QString path1 = m_virtualPath;
    QString path2 = url.m_virtualPath;

    return  path1 == path2 &&
            scheme() == url.scheme() &&
            fragment() == url.fragment() &&
            query() == url.query() &&
            userName() == url.userName() &&
            password() == url.password() &&
            host() == url.host() &&
            port() == url.port();
}

void DUrl::makeAbsolutePath()
{
    if (!hasScheme(this->scheme()))
        return;

    if (isLocalFile()) {
        const QString &path = toLocalFile();

        if (path.startsWith("~"))
            QUrl::setPath(QDir::homePath() + path.mid(1));
        else
            QUrl::setPath(QFileInfo(path).absoluteFilePath());
    } else {
        const QString &path = this->path();

        if (path.startsWith('/')) {
            QUrl::setPath(QFileInfo(this->path()).absoluteFilePath());
        }
    }
}

DUrl DUrl::toAbsolutePathUrl() const
{
    DUrl url = *this;

    url.makeAbsolutePath();

    return url;
}

QString DUrl::toLocalFile() const
{
    if (isTrashFile()) {
        return DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + path();
    } else if (isSearchFile()) {
        return DUrl(fragment()).toLocalFile();
    } else if(isAVFSFile()) {
        return path();
    } else {
        return QUrl::toLocalFile();
    }
}

void DUrl::updateVirtualPath()
{
    m_virtualPath = this->toAbsolutePathUrl().path();

    if (m_virtualPath.endsWith('/') && m_virtualPath.count() != 1) {
        m_virtualPath.remove(m_virtualPath.count() - 1, 1);
    }
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DUrl &url)
{
    QDebugStateSaver saver(deg);

    Q_UNUSED(saver)

    deg.nospace() << "DUrl(" << url.toString() << ")";

    return deg;
}

uint qHash(const DUrl &url, uint seed) Q_DECL_NOTHROW
{
    return qHash(url.scheme()) ^
            qHash(url.userName()) ^
            qHash(url.password()) ^
            qHash(url.host()) ^
            qHash(url.port(), seed) ^
            qHash(url.m_virtualPath) ^
            qHash(url.query()) ^
            qHash(url.fragment());
}
QT_END_NAMESPACE

