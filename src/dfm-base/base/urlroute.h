/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef URLROUTE_H
#define URLROUTE_H

#include "dfm-base/dfm_base_global.h"

#include <QString>
#include <QIcon>
#include <QMap>
#include <QUrl>

DFMBASE_BEGIN_NAMESPACE

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

namespace SchemeTypes {
extern const char *const kFile;
extern const char *const kDesktop;
extern const char *const kEntry;
extern const char *const kSmb;
extern const char *const kFtp;
extern const char *const kSFtp;
extern const char *const kBurn;
extern const char *const kComputer;
}   // namespace SchemeTypes

class UrlRoute
{
    static QHash<QString, SchemeNode> kSchemeInfos;   // info cache
    static QMultiMap<int, QString> kSchemeRealTree;   // index cache
public:
    static bool regScheme(const QString &scheme,
                          const QString &root,
                          const QIcon &icon = QIcon(),
                          const bool isVirtual = false,
                          const QString &displayName = QString(),
                          QString *errorString = nullptr);
    static bool hasScheme(const QString &scheme);
    static QIcon icon(const QString &scheme);
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
    static QUrl urlParent(const QUrl &url);
    static void urlParentList(QUrl url, QList<QUrl> *list);
    static QString rootDisplayName(const QString &scheme);
    static QUrl fromUserInput(const QString &userInput, bool preferredLocalPath = true);
    static QUrl fromUserInput(const QString &userInput, QString workingDirectory,
                              bool preferredLocalPath = true, QUrl::UserInputResolutionOptions options = QUrl::AssumeLocalFile);
};

DFMBASE_END_NAMESPACE

#endif   // URLROUTE_H
