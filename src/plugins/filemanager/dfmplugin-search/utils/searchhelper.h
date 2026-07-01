// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHHELPER_H
#define SEARCHHELPER_H

#include "dfmplugin_search_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QWidget>

namespace dfmplugin_search {

class SearchHelper final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchHelper)

public:
    static SearchHelper *instance();

    inline static QString scheme()
    {
        return "search";
    }

    static QUrl rootUrl();
    static bool isRootUrl(const QUrl &url);
    static bool isSearchFile(const QUrl &url);
    static QUrl searchTargetUrl(const QUrl &searchUrl);
    static QString searchKeyword(const QUrl &searchUrl);
    static QString searchWinId(const QUrl &searchUrl);

    static QUrl setSearchKeyword(const QUrl &searchUrl, const QString &keyword);
    static QUrl setSearchTargetUrl(const QUrl &searchUrl, const QUrl &targetUrl);
    static QUrl setSearchWinId(const QUrl &searchUrl, const QString &winId);
    static QUrl viewModelUrl(const QUrl &url);

    static QUrl fromSearchFile(const QString &filePath);
    static QUrl fromSearchFile(const QUrl &targetUrl, const QString &keyword, const QString &winId);
    static bool showTopWidget(QWidget *w, const QUrl &url);

    bool customColumnRole(const QUrl &rootUrl, QList<DFMGLOBAL_NAMESPACE::ItemRoles> *roleList);
    bool customRoleDisplayName(const QUrl &rootUrl, const DFMGLOBAL_NAMESPACE::ItemRoles role, QString *displayName);

    // disable paste
    bool blockPaste(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to);

    bool searchIconName(const QUrl &url, QString *iconName);

    QString checkWildcardAndToRegularExpression(const QString &pattern);
    QString wildcardToRegularExpression(const QString &pattern);
    inline QString anchoredPattern(const QString &expression)
    {
        return QLatin1String("\\A(?:")
                + expression
                + QLatin1String(")\\z");
    }
    bool isHiddenFile(const QString &fileName, QHash<QString, QSet<QString>> &filters, const QString &searchPath);
    bool allowRepeatUrl(const QUrl &cur, const QUrl &pre);

    bool crumbRedirectUrl(QUrl *redirectUrl);

    [[nodiscard]] static QWidget *createCheckBoxWithFileIndex(QObject *opt);
    [[nodiscard]] static QWidget *createCheckBoxWithTextIndex(QObject *opt);
    [[nodiscard]] static QWidget *createCheckBoxWithOcrIndex(QObject *opt);
    [[nodiscard]] static QWidget *createCheckBoxWithSemanticIndex(QObject *opt);

    // View-hint "authorization experience": the workspace shows a floating hint on
    // the search scheme; these statics supply its visibility/text predicate and react
    // to the user's action (the built-in close button arrives with id "close").
    // shouldShowAuthHint reads DConfig and fills *text when the hint should appear.
    static bool shouldShowAuthHint(const QUrl &url, QString *text);
    static void onAuthHintAction(const QString &id);
    // Enable the advertised search modes / persist the opt-out. Moved here from
    // SearchManager so all hint business logic lives in one helper.
    static void authorizeSearchExperience();
    static void dismissAuthHint();

private:
    explicit SearchHelper(QObject *parent = nullptr);
    ~SearchHelper() override;

    // 返回当前处于关闭状态的搜索模式名称列表（翻译后的名称）。
    static QStringList disabledSearchModes();
};

}

#endif   // SEARCHHELPER_H
