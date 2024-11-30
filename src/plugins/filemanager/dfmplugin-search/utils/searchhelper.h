// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHHELPER_H
#define SEARCHHELPER_H

#include "dfmplugin_search_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QWidget>
#include <QDBusInterface>

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

    static QDBusInterface &anythingInterface();
    [[nodiscard]] static QWidget *createCheckBoxWidthTextIndex(QObject *opt);

private:
    explicit SearchHelper(QObject *parent = nullptr);
    ~SearchHelper() override;
};

}

#endif   // SEARCHHELPER_H
