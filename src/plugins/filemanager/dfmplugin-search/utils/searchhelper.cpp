// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchhelper.h"

#include "checkboxwithfileindex.h"
#include "checkboxwithtextindex.h"
#include "checkboxwithocrindex.h"
#include "checkboxwithsemanticindex.h"
#include "topwidget/advancesearchbar.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/dpf.h>

#include <DSettingsOption>

#include <QLocale>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

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

QUrl SearchHelper::viewModelUrl(const QUrl &url)
{
    if (url.scheme() != scheme() || !url.hasQuery())
        return url;

    // 解析原始查询参数
    QUrlQuery urlQuery(url.query());
    QString urlValue = urlQuery.queryItemValue("url");

    // 创建新的URL对象
    QUrl newUrl(url);

    if (!urlValue.isEmpty()) {
        // 只保留url参数
        QUrlQuery newQuery;
        newQuery.addQueryItem("url", urlValue);
        newUrl.setQuery(newQuery);
    }

    return newUrl;
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
        *iconName = "dfm_search";
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
    if (UniversalUtils::urlEqualsWithQuery(cur, pre))
        return false;
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

QWidget *SearchHelper::createCheckBoxWithTextIndex(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    const QString &text = option->data("text").toString();

    CheckBoxWithTextIndex *cb = new CheckBoxWithTextIndex;
    cb->connectToBackend();
    cb->setDisplayText(qApp->translate("QObject", text.toStdString().c_str()));
    cb->setChecked(option->value().toBool());
    cb->initStatusBar();

    QObject::connect(cb, &CheckBoxWithTextIndex::checkStateChanged, option, [=](Qt::CheckState state) {
        if (state == Qt::CheckState::Unchecked)
            option->setValue(false);
        else if (state == Qt::CheckState::Checked)
            option->setValue(true);
    });

    return cb;
}

QWidget *SearchHelper::createCheckBoxWithFileIndex(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    const QString &text = option->data("text").toString();

    CheckBoxWithFileIndex *cb = new CheckBoxWithFileIndex;
    cb->setDisplayText(qApp->translate("QObject", text.toStdString().c_str()));
    cb->initStatusBar();

    QObject::connect(cb, &IndexStatusCheckBox::checkStateChanged, option, [=](Qt::CheckState state) {
        option->setValue(state == Qt::CheckState::Checked);
    });
    QObject::connect(option, &Dtk::Core::DSettingsOption::valueChanged, cb, [cb](const QVariant &value) {
        if (value.toBool() && !cb->isChecked())
            cb->setChecked(true);
    });

    return cb;
}

QWidget *SearchHelper::createCheckBoxWithOcrIndex(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    const QString &text = option->data("text").toString();

    CheckBoxWithOcrIndex *cb = new CheckBoxWithOcrIndex;
    cb->connectToBackend();
    cb->setDisplayText(qApp->translate("QObject", text.toStdString().c_str()));
    cb->setChecked(option->value().toBool());
    cb->initStatusBar();

    QObject::connect(cb, &CheckBoxWithOcrIndex::checkStateChanged, option, [=](Qt::CheckState state) {
        if (state == Qt::CheckState::Unchecked)
            option->setValue(false);
        else if (state == Qt::CheckState::Checked)
            option->setValue(true);
    });

    return cb;
}

QWidget *SearchHelper::createCheckBoxWithSemanticIndex(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
    const QString &text = option->data("text").toString();

    CheckBoxWithSemanticIndex *cb = new CheckBoxWithSemanticIndex;
    cb->setDisplayText(qApp->translate("QObject", text.toStdString().c_str()));
    cb->setChecked(option->value().toBool());
    cb->initStatusBar();

    QObject::connect(cb, &CheckBoxWithSemanticIndex::checkStateChanged, option, [=](Qt::CheckState state) {
        if (state == Qt::CheckState::Unchecked)
            option->setValue(false);
        else if (state == Qt::CheckState::Checked)
            option->setValue(true);
    });

    return cb;
}

SearchHelper::SearchHelper(QObject *parent)
    : QObject(parent)
{
}

SearchHelper::~SearchHelper()
{
}

bool SearchHelper::shouldShowAuthHint(const QUrl &url, QString *text)
{
    Q_UNUSED(url)
    const QString &cfg = DConfig::kSearchCfgPath;
    if (DConfigManager::instance()->value(cfg, DConfig::kSearchAuthHintDone, false).toBool())
        return false;

    // 收集尚未开启的搜索模式，仅在提示中展示用户实际需要开启的功能项。
    QStringList modes = disabledSearchModes();
    if (modes.isEmpty())
        return false;

    if (text) {
        // 使用 createSeparatedList 生成符合语言习惯的分隔列表（中文使用顿号 + "和"）。
        *text = SearchHelper::tr("Authorize to enable %1")
                        .arg(QLocale().createSeparatedList(modes));
    }
    return true;
}

void SearchHelper::onAuthHintAction(const QString &id)
{
    if (id == "authorize")
        authorizeSearchExperience();
    else if (id == "close")
        dismissAuthHint();
}

void SearchHelper::authorizeSearchExperience()
{
    const QString &cfg = DConfig::kSearchCfgPath;
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableFileIndexSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableFullTextSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableOcrTextSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableSemanticSearch, true);

    // 发送带操作按钮的通知，提示用户索引正在构建。
    // 点击"查看索引状态"按钮时，通过 file-manager.sh --event 打开设置窗口
    // 并自动跳转到"高级设置—搜索"分组。
    QStringList actions = { "view-index-status", tr("View index status") };
    QJsonObject paramObj;
    paramObj.insert("group", SEARCH_SETTING_GROUP);
    QJsonObject argsObj;
    argsObj.insert("action", "settings");
    argsObj.insert("params", paramObj);
    const QStringList cmdShowSettings { "file-manager.sh",
                                        "--event",
                                        QJsonDocument(argsObj).toJson(QJsonDocument::Compact) };
    QVariantMap hints = { { "x-deepin-action-view-index-status", cmdShowSettings } };
    UniversalUtils::notifyMessage(
            tr("dde-file-manager"),
            tr("Index is being built. You can check the index status in Settings."),
            actions,
            hints);
}

void SearchHelper::dismissAuthHint()
{
    // 用户拒绝授权，持久化标记以避免再次弹出提示。
    DConfigManager::instance()->setValue(DConfig::kSearchCfgPath, DConfig::kSearchAuthHintDone, true);

    // 通知用户当前仍处于关闭状态的搜索模式，
    // 提示可在"设置—高级设置—搜索"中手动开启。
    QStringList disabledModes = disabledSearchModes();
    if (!disabledModes.isEmpty()) {
        const QString &modeList = QLocale().createSeparatedList(disabledModes);
        UniversalUtils::notifyMessage(
                tr("dde-file-manager"),
                tr("You can manually enable %1 in Settings — Advanced — Search.").arg(modeList));
    }

    fmInfo() << "Search authorization hint dismissed by user";
}

QStringList SearchHelper::disabledSearchModes()
{
    const QString &cfg = DConfig::kSearchCfgPath;
    QStringList modes;
    if (!DConfigManager::instance()->value(cfg, DConfig::kEnableFullTextSearch, true).toBool())
        modes << tr("\"Full-Text search\"");
    if (!DConfigManager::instance()->value(cfg, DConfig::kEnableOcrTextSearch, false).toBool())
        modes << tr("\"Image-Content search\"");
    if (!DConfigManager::instance()->value(cfg, DConfig::kEnableSemanticSearch, false).toBool())
        modes << tr("\"Smart search\"");
    return modes;
}

}
