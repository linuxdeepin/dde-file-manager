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

#include "durl.h"
#include "interfaces/dfmstandardpaths.h"

#include <utility>

#include <QFileInfo>
#include <QSet>
#include <QDir>
#include <QDebug>
#include <QUrlQuery>

QRegularExpression DUrl::burn_rxp = QRegularExpression("^(.*?)/(" BURN_SEG_ONDISC "|" BURN_SEG_STAGING ")(.*)$");

static inline QString parseDecodedComponent(const QString &data)
{
    return QString(data).replace(QLatin1Char('%'), QStringLiteral("%25"));
}

QSet<QString> schemeList = QSet<QString>() << QString(TRASH_SCHEME)
                           << QString(RECENT_SCHEME)
                           << QString(BOOKMARK_SCHEME)
                           << QString(FILE_SCHEME)
                           << QString(COMPUTER_SCHEME)
                           << QString(SEARCH_SCHEME)
                           << QString(NETWORK_SCHEME)
                           << QString(SMB_SCHEME)
                           << QString(AFC_SCHEME)
                           << QString(MTP_SCHEME)
                           << QString(USERSHARE_SCHEME)
                           << QString(AVFS_SCHEME)
                           << QString(FTP_SCHEME)
                           << QString(SFTP_SCHEME)
                           << QString(DAV_SCHEME)
                           << QString(TAG_SCHEME)
                           << QString(DFMVAULT_SCHEME) // 文件保险柜
                           << QString(BURN_SCHEME)
                           << QString(PLUGIN_SCHEME); // NOTE [XIAO]

DUrl::DUrl()
    : QUrl()
{

}

DUrl::DUrl(const QUrl &copy)
    : QUrl(copy)
{
    updateVirtualPath();
}


DUrl::DUrl(const DUrl &other)
    : QUrl{other},
      m_virtualPath{other.m_virtualPath}
{
    //###copy constructor
}


DUrl::DUrl(DUrl &&other)
    : QUrl{ std::move(other) },
      m_virtualPath{ std::move(other.m_virtualPath) }
{
    //###move constructor
}


//###copy operator=
DUrl &DUrl::operator=(const DUrl &other)
{
    QUrl::operator=(other);
    m_virtualPath = other.m_virtualPath;

    return *this;
}


//###move operator=
DUrl &DUrl::operator=(DUrl &&other)
{
    QUrl::operator=(std::move(other));
    m_virtualPath = std::move(other.m_virtualPath);
    return *this;
}

QDataStream &DUrl::operator<<(QDataStream &out)
{
    QByteArray u;
    if (this->isValid())
        u = this->toEncoded();
    out << u << m_virtualPath;
    return out;
}

QDataStream &DUrl::operator>>(QDataStream &in)
{
    QByteArray u;
    QString virtualPath;
    in >> u >> virtualPath;
    this->setUrl(QString::fromLatin1(u));
    this->m_virtualPath = virtualPath;
    return in;
}


DUrl::DUrl(const QString &url, QUrl::ParsingMode mode)
    : QUrl(url, mode)
{
    updateVirtualPath();
}

void DUrl::setPath(const QString &path, QUrl::ParsingMode mode, bool makeAbsolutePath)
{
    QUrl::setPath(path, mode);

    if (makeAbsolutePath) {
        this->makeAbsolutePath();
    }

    updateVirtualPath();
}

void DUrl::setScheme(const QString &scheme, bool makeAbsolute)
{
    QUrl::setScheme(scheme);

    if (makeAbsolute) {
        this->makeAbsolutePath();
    }

    updateVirtualPath();
}

void DUrl::setUrl(const QString &url, QUrl::ParsingMode parsingMode, bool makeAbsolute)
{
    QUrl::setUrl(url, parsingMode);

    if (makeAbsolute) {
        this->makeAbsolutePath();
    }

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


///###: Judge whether current the scheme of current url is equal to TAG_SCHEME.
bool DUrl::isTaggedFile() const
{
    return (this->scheme() == QString{TAG_SCHEME});
}

bool DUrl::isVaultFile() const
{
    return (this->scheme() == DFMVAULT_SCHEME);
}

bool DUrl::isPluginFile() const
{
    return this->scheme() == PLUGIN_SCHEME;
}

QString DUrl::toString(QUrl::FormattingOptions options) const
{
    if (!isValid()) {
        return m_virtualPath;
    }

    if (isLocalFile() || !schemeList.contains(scheme())) {
        return QUrl::toString(options);
    }

    QUrl url(*this);

    url.setScheme(FILE_SCHEME);

    return url.toString(options).replace(0, 4, scheme());
}

QString DUrl::searchKeyword() const
{
    if (!isSearchFile()) {
        return QString();
    }

    QUrlQuery query(this->query());

    return query.queryItemValue("keyword", FullyDecoded);
}

DUrl DUrl::searchTargetUrl() const
{
    if (!isSearchFile()) {
        return DUrl();
    }

    QUrlQuery query(this->query());

    return DUrl(query.queryItemValue("url", FullyDecoded));
}

DUrl DUrl::searchedFileUrl() const
{
    if (!isSearchFile()) {
        return DUrl();
    }

    return DUrl(fragment(FullyDecoded));
}

///###: tag:///tagA#real file path(fregment).
///###: so I get the true path from fregment of Uri.
QString DUrl::taggedLocalFilePath() const noexcept
{
    if (this->isTaggedFile()) {
        QString localFilePath{ this->QUrl::fragment(FullyDecoded) };

        return localFilePath;
    }

    return QString{};
}

QString DUrl::tagName() const noexcept
{
    if (this->isTaggedFile()) {
        QUrlQuery qq(query());
        return qq.hasQueryItem("tagname") ? qq.queryItemValue("tagname") : QUrl::fileName();
        //return this->fileName();
    }

    return QString{};
}

QString DUrl::fileName(QUrl::ComponentFormattingOptions options) const
{
    return isTaggedFile() ? tagName() : QUrl::fileName(options);
}

QString DUrl::deviceId() const
{
    if (scheme() != DEVICE_SCHEME)
        return QString();

    return path();
}

DUrl DUrl::bookmarkTargetUrl() const
{
    if (scheme() != BOOKMARK_SCHEME)
        return DUrl();

    return DUrl(path());
}

QString DUrl::bookmarkName() const
{
    if (scheme() != BOOKMARK_SCHEME)
        return QString();

    return fragment(FullyDecoded);
}

QString DUrl::burnDestDevice() const
{
    QRegularExpressionMatch m;
    if (scheme() != BURN_SCHEME || !path().contains(burn_rxp, &m)) {
        return "";
    }
    return m.captured(1);
}

QString DUrl::burnFilePath() const
{
    QRegularExpressionMatch m;
    if (scheme() != BURN_SCHEME || !path().contains(burn_rxp, &m)) {
        return "";
    }
    return m.captured(3);
}

bool DUrl::burnIsOnDisc() const
{
    QRegularExpressionMatch m;
    if (scheme() != BURN_SCHEME || !path().contains(burn_rxp, &m)) {
        return false;
    }
    return m.captured(2) == BURN_SEG_ONDISC;
}

DUrl DUrl::parentUrl() const
{
    return parentUrl(*this);
}

void DUrl::setSearchKeyword(const QString &keyword)
{
    if (!isSearchFile()) {
        return;
    }

    QUrlQuery query(this->query());

    query.removeQueryItem("keyword");
    query.addQueryItem("keyword", parseDecodedComponent(keyword));

    setQuery(query);
}

void DUrl::setSearchTargetUrl(const DUrl &url)
{
    if (!isSearchFile()) {
        return;
    }

    QUrlQuery query(this->query());

    query.removeQueryItem("url");
    query.addQueryItem("url", parseDecodedComponent(url.toString()));

    setQuery(query);
}

void DUrl::setSearchedFileUrl(const DUrl &url)
{
    if (!isSearchFile()) {
        return;
    }

    setFragment(url.toString(), QUrl::DecodedMode);
}


///###: the real path of file was puted in fragment field of Uri.
void DUrl::setTaggedFileUrl(const QString &localFilePath) noexcept
{
    if (this->isTaggedFile()) {
        this->QUrl::setFragment(localFilePath, QUrl::DecodedMode);
    }
}

void DUrl::setBookmarkName(const QString &name)
{
    if (this->scheme() != BOOKMARK_SCHEME)
        return;

    setFragment(name, DecodedMode);
}

DUrl DUrl::fromLocalFile(const QString &filePath)
{
    return DUrl(QUrl::fromLocalFile(filePath));
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

DUrl DUrl::fromBookMarkFile(const DUrl &targetUrl, const QString &name)
{
    DUrl url;

    url.setScheme(BOOKMARK_SCHEME, false);
    url.setPath(targetUrl.toString());
    url.setBookmarkName(name);

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

    query.addQueryItem("url", parseDecodedComponent(targetUrl.toString()));
    query.addQueryItem("keyword", parseDecodedComponent(keyword));

    url.setQuery(query);

    if (searchedFileUrl.isValid()) {
        url.setFragment(searchedFileUrl.toString(), DecodedMode);
    }

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


DUrl DUrl::fromUserTaggedFile(const QString &tag_name, const QString &localFilePath)noexcept
{
    DUrl uri{};
    uri.setScheme(TAG_SCHEME);
    uri.setPath(QString{"/"} + tag_name);
    uri.setQuery("tagname=" + tag_name);

    if (!localFilePath.isEmpty()) {
        uri.setFragment(localFilePath, QUrl::DecodedMode);
    }

    return uri;
}

DUrl DUrl::fromDeviceId(const QString &deviceId)
{
    DUrl url;
    url.setScheme(DEVICE_SCHEME);
    url.setPath(deviceId);

    return url;
}

DUrl DUrl::fromBurnFile(const QString &filePath)
{
    DUrl ret;
    ret.setScheme(BURN_SCHEME);
    ret.setPath(filePath);
    return ret;
}

DUrl DUrl::fromVaultFile(const QString &filePath)
{
    DUrl url;
    url.setScheme(DFMVAULT_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrlList DUrl::fromStringList(const QStringList &urls, QUrl::ParsingMode mode)
{
    QList<DUrl> urlList;

    for (const QString &string : urls) {
        urlList << DUrl(string, mode);
    }

    return urlList;
}

DUrlList DUrl::fromQUrlList(const QList<QUrl> &urls)
{
    QList<DUrl> urlList;

    for (const QUrl &url : urls) {
        urlList << DUrl(url);
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
    if (options != AssumeLocalFile) {
        return DUrl(QUrl::fromUserInput(userInput, workingDirectory, options));
    }

    if ((userInput.startsWith("~") && preferredLocalPath) || userInput.startsWith("~/")) {
        return DUrl::fromLocalFile(QDir::homePath() + userInput.mid(1));
    } else if ((preferredLocalPath && QDir().exists(userInput)) || userInput.startsWith("./")
               || userInput.startsWith("../") || userInput.startsWith("/")) {
        QDir dir(userInput);

        return DUrl::fromLocalFile(dir.absolutePath());
    } else {
        DUrl url(userInput);

        // NOTE(zccrs): 保证数据传入QUrl还能使用toString返回数据
        if (!url.isValid() || (url.scheme().isEmpty() && url.toString() != userInput)) {
            DUrl durl;

            durl.m_virtualPath = userInput;

            return durl;
        }

        return url;
    }
}

QStringList DUrl::toStringList(const DUrlList &urls, QUrl::FormattingOptions options)
{
    QStringList urlList;

    for (const DUrl &url : urls) {
        urlList << url.toString(options);
    }

    return urlList;
}

QList<QUrl> DUrl::toQUrlList(const DUrlList &urls)
{
    QList<QUrl> urlList;

    for (const DUrl &url : urls) {
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

    if (path == "/") {
        return DUrl();
    }

    _url.setScheme(url.scheme());
    QStringList paths = path.split("/");
    paths.removeAt(0);
    if (!paths.isEmpty() && paths.last().isEmpty()) {
        paths.removeLast();
    }
    if (!paths.isEmpty()) {
        paths.removeLast();
    }
    QString _path;
    foreach (QString p, paths) {
        _path += "/" + p;
    }
    if (_path.isEmpty()) {
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
    if (!hasScheme(url.scheme())) {
        return QUrl::operator ==(url);
    }

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
    if (!hasScheme(this->scheme())) {
        return;
    }

    if (isLocalFile()) {
        const QString &path = toLocalFile();

        if (path.startsWith("~")) {
            QUrl::setPath(QDir::homePath() + path.mid(1));
        } else {
            QUrl::setPath(QFileInfo(path).absoluteFilePath());
        }
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
        return DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + path();
    } else if (isSearchFile()) {
        return searchedFileUrl().toLocalFile();
    } else if (isAVFSFile()) {
        return path();
    } else if (isTaggedFile()) {
        return taggedLocalFilePath();
    } else if (isUserShareFile()) {
        return QString(path()).remove(USERSHARE_ROOT);
    } else if (isVaultFile()) {
        return path();
    } else {
        return QUrl::toLocalFile();
    }
}

void DUrl::updateVirtualPath()
{
    m_virtualPath = toAbsolutePathUrl().path();

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

uint qHash(const DUrl &url, uint seed) Q_DECL_NOTHROW {
    return qHash(url.scheme()) ^
    qHash(url.userName()) ^
    qHash(url.password()) ^
    qHash(url.host()) ^
    qHash(url.port(), seed) ^
    qHash(url.m_virtualPath) ^
    qHash(url.query()) ^
    qHash(url.fragment());
}

QDataStream &operator<<(QDataStream &out, const DUrl &url)
{
    QByteArray u;
    if (url.isValid())
        u = url.toEncoded();
    out << u << url.m_virtualPath;
    return out;
}

QDataStream &operator>>(QDataStream &in, DUrl &url)
{
    QByteArray u;
    QString virtualPath;
    in >> u >> virtualPath;
    url.setUrl(QString::fromLatin1(u));
    url.m_virtualPath = virtualPath;
    return in;
}
QT_END_NAMESPACE
