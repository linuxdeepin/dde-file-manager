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

class QUrl;
class QString;
DFMBASE_BEGIN_NAMESPACE

struct SchemeNode
{
    QString path;   // 根路径
    QIcon icon;   // 标志root的图标
    bool virtualFlag;   // 没有本地路径作为映射的Url
    friend class QHash<QString, SchemeNode>;
    SchemeNode() {}

public:
    SchemeNode(const QString &root,
               const QIcon &icon = QIcon(),
               const bool isVirtual = false);
    SchemeNode &operator=(const SchemeNode &node);
    bool isEmpty();
    QString rootPath() const;
    QIcon pathIcon() const;
    bool isVirtual() const;
};

namespace SchemeTypes {
extern const char *const kFile;
extern const char *const kDesktop;
extern const char *const kHome;
extern const char *const kVideos;
extern const char *const kMusic;
extern const char *const kPictures;
extern const char *const kDocuments;
extern const char *const kDownloads;
extern const char *const kRoot;
}   // namespace SchemeTypes

class UrlRoute
{
    static QHash<QString, SchemeNode> schemeInfos;   //info cache
    static QMultiMap<int, QString> schemeRealTree;   //index cache
public:
    static bool regScheme(const QString &scheme,
                          const QString &root,
                          const QIcon &icon = QIcon(),
                          const bool isVirtual = false,
                          QString *errorString = nullptr);
    static bool hasScheme(const QString &scheme);
    static QIcon icon(const QString &scheme);
    static QString rootPath(const QString &scheme);
    static QString urlToPath(const QUrl &url);
    static QUrl fromLocalFile(const QString &path);
    static QUrl pathToReal(const QString &path);
    static QUrl pathToUrl(const QString &path, const QString &scheme);
    static bool isVirtual(const QString &scheme);
    static bool isVirtual(const QUrl &url);
    static bool isRootUrl(const QUrl &url);
    static QUrl urlParent(const QUrl &url);
};
DFMBASE_END_NAMESPACE
Q_DECLARE_METATYPE(QUrl);

#endif   // URLROUTE_H
