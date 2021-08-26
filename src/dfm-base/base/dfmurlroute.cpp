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

#include "dfmurlroute.h"

QList<SchemeNode> DFMUrlRoute::m_schemeMapLists{};

bool DFMUrlRoute::schemeMapRoot(const QString &scheme,
                                const QString &root,
                                const QIcon &icon,
                                const bool isVirtual,
                                QString *errorString)
{
    if (hasScheme(scheme)) {
        *errorString = QObject::tr("Scheme cannot be registered repeatedly.");
        return false;
    }

    //非虚拟路径则进行本地路径判断
    if (!isVirtual) {

        if (!QDir(root).exists(root)) {
            if (errorString)
                *errorString = QObject::tr("Scheme map to root path not exists.");
            return false;
        }

        QString formatRoot = root;
        if (!root.endsWith("/")) {
            formatRoot = root + "/";
        }

        //逆序遍历
        auto itera = m_schemeMapLists.end();
        while(itera != m_schemeMapLists.begin()) {
            -- itera;
            //如果当前注册的映射根是注册过的任意根的子目录
            //并且非虚拟路径
            if (itera->root().endsWith(formatRoot)) {
                //放到注册节点之后
                m_schemeMapLists.insert(itera,{scheme, formatRoot, icon ,isVirtual});
                return true;
            }
        }
    }

    //是虚拟路径则直接添加到最后
    m_schemeMapLists.append({scheme, root, icon, isVirtual});
    return true;
}

QIcon DFMUrlRoute::schemeIcon(const QString &scheme)
{
    for (auto val: m_schemeMapLists) {
        if (val.scheme() == scheme) return val.icon();
    }
    return QIcon();
}

bool DFMUrlRoute::hasScheme(const QString &scheme) {
    for (auto val : m_schemeMapLists) {
        if (val.scheme() == scheme) return true;
    }
    return false;
}

QUrl DFMUrlRoute::fromLocalFile(const QString &path, QString *errorString)
{
    if (!QFileInfo(path).exists()) {
        if (errorString)
            *errorString = QObject::tr("path not exists");
        return QUrl();
    }
    return QUrl::fromLocalFile(path);
}

bool DFMUrlRoute::isSchemeRoot(const QUrl &url)
{
    QUrl urlCmp;
    urlCmp.setScheme(url.scheme());
    urlCmp.setPath("/");
    urlCmp.setHost(" ");
    urlCmp.setPort(-1);

    if (url == urlCmp)
        return true;

    return false;
}

bool DFMUrlRoute::isVirtualUrl(const QUrl &url)
{
    auto itera = m_schemeMapLists.end();
    while (itera != m_schemeMapLists.begin())
    {
        -- itera;

        auto scheme = itera->scheme();

        if(scheme == url.scheme()) {
            return itera->isVirtual();
        }
    }
    return true;
}

QUrl DFMUrlRoute::urlParent(const QUrl &url) {
    if(isSchemeRoot(url))
        return url;

    auto list = url.path().split("/");
    list.removeAt(list.size() -1);

    QUrl reUrl;
    url.scheme();
    reUrl.setScheme(url.scheme());
    reUrl.setPath(list.join("/"));

    return reUrl;
}

//QUrl DFMUrlRoute::urlParentRoot(const QUrl &url)
//{
//    auto itera = m_schemeMapLists.end();
//    QString currPathRoot;
//    while (itera != m_schemeMapLists.begin())
//    {
//        -- itera;
//        if (itera->first == url.scheme()) {
//            currPathRoot = itera->second;
//            continue;
//        }

//        if (!currPathRoot.isEmpty() &&
//                currPathRoot.contains(itera->second)) {
//            QUrl url;
//            url.setScheme(itera->first);
//            url.setHost(" ");
//            url.setPath(itera->second);
//            return url;
//        }
//    }
//    return QUrl();
//}

QString DFMUrlRoute::schemeRoot(const QString &scheme, QString *errorString)
{
    //按照子节点逆序规则选中
    auto itera = m_schemeMapLists.end();
    while (itera != m_schemeMapLists.begin())
    {
        -- itera;
        if (itera->scheme() == scheme) {
            return itera->root();
        }
    }

    if (errorString)
        *errorString = QObject::tr("Not found root path from %0").arg(scheme);
    return "";
}

bool DFMUrlRoute::schemeIsVirtual(const QString &scheme)
{
    if (scheme.isEmpty())
        return true;

    for (auto node : m_schemeMapLists) {
        if (node.scheme() == scheme)
        {
            return node.isVirtual();
        }
    }

    return false;
}

QUrl DFMUrlRoute::pathToVirtual(const QString &path, QString *errorString)
{
    auto itera = m_schemeMapLists.end();
    while (itera != m_schemeMapLists.begin())
    {
        -- itera;

        if (itera->isVirtual() == false)
            continue;

        auto shcheme = itera->scheme();
        auto root = itera->root();
        auto pathTmp = path;

        if(pathTmp.startsWith(root)) {
            QUrl url;
            QString repPath = pathTmp.replace(root,"/").replace("//","/");
            url.setHost(" ");
            url.setPort(-1);
            url.setPath(repPath);
            url.setScheme(shcheme);
            return url;
        }
    }
    if (errorString)
        *errorString = QObject::tr("Not found scheme convert path to virtual url");
    return QUrl();
}

QString DFMUrlRoute::virtualToPath(const QUrl &url, QString *errorString)
{
    QString path = "";
    for (auto val : m_schemeMapLists) {

        if (val.isVirtual() == false)
            continue;

        auto root = val.root();
        auto scheme = val.scheme();
        if (scheme == url.scheme()) {

            if (url.path().startsWith("/")) {
                path = root.remove(root.size()-1,1) + url.path();
            }
            return path;
        }
    }

    if (errorString)
        *errorString = "Maybe in called after execute schemeMapRoot function";
    return "";
}

QUrl DFMUrlRoute::pathToUrl(const QString &path, QString *errorString)
{
    QString formatPath = path;
    if (QFileInfo(path).isDir()
            && !path.endsWith("/")) {
        formatPath = path + "/";
    }

    if (!QFileInfo(path).exists()) {
        if (errorString)
            *errorString = QObject::tr("Can't convert path to url , '%0' it not exists").arg(path);
        return QUrl();
    }

    auto itera = m_schemeMapLists.end();
    while (itera != m_schemeMapLists.begin())
    {
        -- itera;

        if (itera->isVirtual()) continue;

        auto shcheme = itera->scheme();
        auto root = itera->root();

        if(formatPath.startsWith(root)) {
            QUrl url;
            QString repPath = formatPath.replace(root,"/").replace("//","/");
            url.setHost(" ");
            url.setPort(-1);
            url.setPath(repPath);
            url.setScheme(shcheme);
            return url;
        }
    }

    QUrl url;
    url.setScheme("file");
    url.setPath(formatPath);
    return url;
}

QString DFMUrlRoute::urlToPath(const QUrl &url, QString *errorString)
{
    QString path = "";
    for (auto val : m_schemeMapLists) {

        if (val.isVirtual() == true)
            continue;

        auto root = val.root();
        auto scheme = val.scheme();
        if (scheme == url.scheme()) {

            if (url.path().startsWith("/")) {
                path = root.remove(root.size(),1) + url.path();
            }
            return path;
        }
    }

    if (errorString) *errorString = "Maybe in called after execute schemeMapRoot function";
    return "";
}

QIcon SchemeNode::icon() const
{
    return m_icon;
}

void SchemeNode::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

SchemeNode::SchemeNode(const QString &scheme, const QString &root,const QIcon &icon ,const bool isVirtual)
    : m_scheme(scheme),
      m_root(root),
      m_icon(icon),
      m_isVirtual(isVirtual)
{

}

SchemeNode &SchemeNode::operator =(const SchemeNode &node)
{
    m_scheme = node.scheme();
    m_root = node.root();
    m_isVirtual = node.isVirtual();
    return *this;
}

QString SchemeNode::scheme() const
{
    return m_scheme;
}

void SchemeNode::setScheme(const QString &scheme)
{
    m_scheme = scheme;
}

QString SchemeNode::root() const
{
    return m_root;
}

void SchemeNode::setRoot(const QString &root)
{
    m_root = root;
}

bool SchemeNode::isVirtual() const
{
    return m_isVirtual;
}

void SchemeNode::setIsVirtual(bool isVirtual)
{
    m_isVirtual = isVirtual;
}
