/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include <list>

class QUrl;
class QString;
DFMBASE_BEGIN_NAMESPACE

/*!
 * \brief 路由Url注册节点类
 */
struct SchemeNode
{
    QString myScheme; // url前缀
    QString myRoot;   // 根路径
    QIcon myIcon;     // 标志root的图标
    bool isMyVirtual; // 没有本地路径作为映射的Url

public:
    SchemeNode(const QString &scheme,
               const QString &root,
               const QIcon &icon = QIcon(),
               const bool isVirtual= false);
    SchemeNode& operator = (const SchemeNode& node);
    QString scheme() const;
    void setScheme(const QString &scheme);
    QString root() const;
    void setRoot(const QString &root);
    bool isVirtual() const;
    void setIsVirtual(bool isVirtual);
    QIcon icon() const;
    void setIcon(const QIcon &icon);
};

class UrlRoute
{
    static QList<SchemeNode> SchemeMapLists;
public:
    static bool schemeMapRoot(const QString &scheme,
                              const QString &root,
                              const QIcon &icon = QIcon(),
                              const bool isVirtual = false,
                              QString *errorString = nullptr);
    static QIcon schemeIcon(const QString &scheme);
    static bool hasScheme(const QString &scheme);
    static QUrl fromLocalFile(const QString &path,
                              QString *errorString = nullptr);
    static QString schemeRoot(const QString &scheme,
                              QString *errorString = nullptr);
    static bool schemeIsVirtual(const QString &scheme);
    static QUrl pathToUrl(const QString &path,
                          QString *errorString = nullptr);
    static QString urlToPath(const QUrl &url,
                             QString *errorString = nullptr);
    static QUrl urlParent(const QUrl &url);
    static bool isSchemeRoot(const QUrl &url);
    static bool isVirtualUrl(const QUrl &url);
    static QUrl pathToVirtual(const QString &path,
                              QString *errorString = nullptr);
    static QString virtualToPath(const QUrl &url,
                                 QString *errorString = nullptr);
};
DFMBASE_END_NAMESPACE
Q_DECLARE_METATYPE(QUrl);

#endif // URLROUTE_H
