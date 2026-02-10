// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URLROUTE_H
#define URLROUTE_H

#include <dfm-base/dfm_base_global.h>

#include <QString>
#include <QIcon>
#include <QMap>
#include <QUrl>

namespace dfmbase {

// TODO(zhangs): refactor, use UrlHelper repalce it

struct SchemeNode
{
    QString path;   // 根路径
    QIcon icon;   // 标志root的图标
    bool virtualFlag;   // 没有本地路径作为映射的Url
    QString name;   // 根目录对应的路径名称，例如 computer:/// 「计算机」，recent:/// 「最近使用」
    friend class QHash<QString, SchemeNode>;
    SchemeNode() {}

public:
    SchemeNode(const QString &root,
               const QIcon &icon = QIcon(),
               const bool isVirtual = false,
               const QString &name = QString());
    SchemeNode &operator=(const SchemeNode &node);
    bool isEmpty();
    QString rootPath() const;
    QIcon pathIcon() const;
    bool isVirtual() const;
    QString displayName() const;
};

class UrlRoute
{
private:
    static QHash<QString, SchemeNode> &schemeInfos();   // info cache
    static QMultiMap<int, QString> &schemeRealTree();   // index cache
public:
    static bool regScheme(const QString &scheme,
                          const QString &root,
                          const QIcon &icon = QIcon(),
                          const bool isVirtual = false,
                          const QString &displayName = QString(),
                          QString *errorString = nullptr);
    static bool hasScheme(const QString &scheme);
    static QIcon icon(const QString &scheme);
    static QString toString(const QUrl &url, QUrl::FormattingOptions options = QUrl::PrettyDecoded);
    static QString rootPath(const QString &scheme);
    static QUrl rootUrl(const QString &scheme);
    static QString urlToPath(const QUrl &url);
    static QString urlToLocalPath(const QUrl &url);
    static QUrl fromLocalFile(const QString &path);
    static QUrl pathToReal(const QString &path);
    static QUrl pathToUrl(const QString &path, const QString &scheme);
    static bool isVirtual(const QString &scheme);
    static bool isVirtual(const QUrl &url);
    static bool isRootUrl(const QUrl &url);
    [[deprecated("Use QUrl::RemoveFilename | QUrl::StripTrailingSlash instead.")]] static QUrl urlParent(const QUrl &url);
    static bool isAncestorsUrl(QUrl url, const QUrl &ancestorsUrl, QList<QUrl> *list);
    static void urlParentList(QUrl url, QList<QUrl> *list);
    static QString rootDisplayName(const QString &scheme);
    static QUrl fromUserInput(const QString &userInput, bool preferredLocalPath = true);
    static QUrl fromUserInput(const QString &userInput, QString workingDirectory,
                              bool preferredLocalPath = true, QUrl::UserInputResolutionOptions options = QUrl::AssumeLocalFile);
    static QList<QUrl> fromStringList(const QStringList &strList);
    static QByteArray urlsToByteArray(const QList<QUrl> &list);
    static QList<QUrl> byteArrayToUrls(const QByteArray &arr);
};

}

#endif   // URLROUTE_H
