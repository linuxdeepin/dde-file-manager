// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchhelper.h"
#include "topwidget/advancesearchbar.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <QUrlQuery>

Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(QVariant *)

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
namespace dfmplugin_search {

static inline QString parseDecodedComponent(const QString &data)
{
    return QString(data).replace(QLatin1Char('%'), QStringLiteral("%25"));
}

SearchHelper *SearchHelper::instance()
{
    static SearchHelper ins;
    return &ins;
}

QUrl SearchHelper::rootUrl()
{
    return fromSearchFile("/");
}

bool SearchHelper::isRootUrl(const QUrl &url)
{
    return url.path() == rootUrl().path();
}

bool SearchHelper::isSearchFile(const QUrl &url)
{
    return url.scheme() == scheme();
}

QUrl SearchHelper::searchTargetUrl(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return QUrl(query.queryItemValue("url", QUrl::FullyDecoded));
}

QString SearchHelper::searchKeyword(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return query.queryItemValue("keyword", QUrl::FullyDecoded);
}

QString SearchHelper::searchWinId(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return query.queryItemValue("winId", QUrl::FullyDecoded);
}

QUrl SearchHelper::setSearchKeyword(const QUrl &searchUrl, const QString &keyword)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("keyword");
    query.addQueryItem("keyword", parseDecodedComponent(keyword));
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::setSearchTargetUrl(const QUrl &searchUrl, const QUrl &targetUrl)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("url");
    query.addQueryItem("url", parseDecodedComponent(targetUrl.toString()));
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::setSearchWinId(const QUrl &searchUrl, const QString &winId)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("winId");
    query.addQueryItem("winId", winId);
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::fromSearchFile(const QString &filePath)
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath(filePath);

    return url;
}

QUrl SearchHelper::fromSearchFile(const QUrl &targetUrl, const QString &keyword, const QString &winId)
{
    QUrl url = rootUrl();
    QUrlQuery query;

    query.addQueryItem("url", parseDecodedComponent(targetUrl.toString()));
    query.addQueryItem("keyword", parseDecodedComponent(keyword));
    query.addQueryItem("winId", winId);
    url.setQuery(query);

    return url;
}

bool SearchHelper::showTopWidget(QWidget *w, const QUrl &url)
{
    auto topWidget = qobject_cast<AdvanceSearchBar *>(w);
    if (!topWidget)
        return false;

    bool visible = w->isVisible();
    if (visible)
        topWidget->refreshOptions(url);

    return visible;
}

bool SearchHelper::customColumnRole(const QUrl &rootUrl, QList<ItemRoles> *roleList)
{
    if (rootUrl.scheme() != scheme())
        return false;

    const QUrl &targetUrl = searchTargetUrl(rootUrl);
    if (!dpfHookSequence->run("dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles", targetUrl, roleList)) {
        roleList->append(kItemFileDisplayNameRole);
        roleList->append(kItemFilePathRole);
        roleList->append(kItemFileLastModifiedRole);
        roleList->append(kItemFileSizeRole);
        roleList->append(kItemFileMimeTypeRole);
    }

    return true;
}

bool SearchHelper::customRoleDisplayName(const QUrl &rootUrl, const ItemRoles role, QString *displayName)
{
    if (rootUrl.scheme() != scheme())
        return false;

    const QUrl &targetUrl = searchTargetUrl(rootUrl);
    if (dpfHookSequence->run("dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName", targetUrl, role, displayName))
        return true;

    if (role == kItemFilePathRole) {
        displayName->append(tr("Path"));
        return true;
    }

    return false;
}

bool SearchHelper::blockPaste(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to)
{
    Q_UNUSED(winId)
    Q_UNUSED(fromUrls)

    if (to.scheme() == SearchHelper::scheme()) {
        fmInfo() << "The search root directory does not support paste!";
        return true;
    }
    return false;
}

bool SearchHelper::searchIconName(const QUrl &url, QString *iconName)
{
    if (url.scheme() == SearchHelper::scheme()) {
        *iconName = "search";
        return true;
    }

    return false;
}

QString SearchHelper::checkWildcardAndToRegularExpression(const QString &pattern)
{
    if (!pattern.contains('*') && !pattern.contains('?'))
        return wildcardToRegularExpression('*' + pattern + '*');

    return wildcardToRegularExpression(pattern);
}

QString SearchHelper::wildcardToRegularExpression(const QString &pattern)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    return QRegularExpression::wildcardToRegularExpression(pattern);
#endif   // (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))

    const int wclen = pattern.length();
    QString rx;
    rx.reserve(wclen + wclen / 16);
    int i = 0;
    const QChar *wc = pattern.unicode();
#ifdef Q_OS_WIN
    const QLatin1Char nativePathSeparator('\\');
    const QLatin1String starEscape("[^/\\\\]*");
    const QLatin1String questionMarkEscape("[^/\\\\]");
#else
    const QLatin1Char nativePathSeparator('/');
    const QLatin1String starEscape("[^/]*");
    const QLatin1String questionMarkEscape("[^/]");
#endif
    while (i < wclen) {
        const QChar c = wc[i++];
        switch (c.unicode()) {
        case '*':
            rx += starEscape;
            break;
        case '?':
            rx += questionMarkEscape;
            break;
        case '\\':
#ifdef Q_OS_WIN
        case '/':
            rx += QLatin1String("[/\\\\]");
            break;
#endif
        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
            rx += QLatin1Char('\\');
            rx += c;
            break;
        case '[':
            rx += c;
            // Support for the [!abc] or [!a-c] syntax
            if (i < wclen) {
                if (wc[i] == QLatin1Char('!')) {
                    rx += QLatin1Char('^');
                    ++i;
                }
                if (i < wclen && wc[i] == QLatin1Char(']'))
                    rx += wc[i++];
                while (i < wclen && wc[i] != QLatin1Char(']')) {
                    // The '/' appearing in a character class invalidates the
                    // regular expression parsing. It also concerns '\\' on
                    // Windows OS types.
                    if (wc[i] == QLatin1Char('/') || wc[i] == nativePathSeparator)
                        return rx;
                    if (wc[i] == QLatin1Char('\\'))
                        rx += QLatin1Char('\\');
                    rx += wc[i++];
                }
            }
            break;
        default:
            rx += c;
            break;
        }
    }
    return anchoredPattern(rx);
}

bool SearchHelper::isHiddenFile(const QString &fileName, QHash<QString, QSet<QString>> &filters, const QString &searchPath)
{
    if (!fileName.startsWith(searchPath) || fileName == searchPath)
        return false;

    QFileInfo fileInfo(fileName);
    if (fileInfo.isHidden())
        return true;

    const auto &fileParentPath = fileInfo.absolutePath();
    const auto &hiddenFileConfig = fileParentPath + "/.hidden";

    // 判断.hidden文件是否存在，不存在说明该路径下没有隐藏文件
    if (!QFile::exists(hiddenFileConfig))
        return isHiddenFile(fileParentPath, filters, searchPath);

    if (filters[fileParentPath].isEmpty()) {
        QFile file(hiddenFileConfig);
        // 判断.hidden文件中的内容是否为空，空则表示该路径下没有隐藏文件
        if (file.isReadable() && file.size() > 0) {
            if (!file.open(QFile::ReadOnly))
                return false;

            QByteArray data = file.readAll();
            file.close();
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
            const auto &hiddenFiles = QSet<QString>::fromList(QString(data).split('\n', QString::SkipEmptyParts));
#else
            auto &&list { QString(data).split('\n', Qt::SkipEmptyParts) };
            const auto &hiddenFiles = QSet<QString>(list.begin(), list.end());
#endif
            filters[fileParentPath] = hiddenFiles;
        } else {
            return isHiddenFile(fileParentPath, filters, searchPath);
        }
    }

    return filters[fileParentPath].contains(fileInfo.fileName())
            ? true
            : isHiddenFile(fileParentPath, filters, searchPath);
}

bool SearchHelper::allowRepeatUrl(const QUrl &cur, const QUrl &pre)
{
    if (cur.scheme() == scheme() && pre.scheme() == scheme())
        return true;
    return false;
}

bool SearchHelper::crumbRedirectUrl(QUrl *redirectUrl)
{
    if (redirectUrl->scheme() == scheme()) {
        *redirectUrl = searchTargetUrl(*redirectUrl);
        return true;
    }
    return false;
}

QDBusInterface &SearchHelper::anythingInterface()
{
    static QDBusInterface interface("com.deepin.anything",
                                    "/com/deepin/anything",
                                    "com.deepin.anything",
                                    QDBusConnection::systemBus());

    return interface;
}

SearchHelper::SearchHelper(QObject *parent)
    : QObject(parent)
{
}

SearchHelper::~SearchHelper()
{
}

}
