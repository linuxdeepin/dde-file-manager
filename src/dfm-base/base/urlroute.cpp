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
#include "urlroute.h"

DFMBASE_BEGIN_NAMESPACE
QList<SchemeNode> UrlRoute::SchemeMapLists{};
/*!
 * \class UrlRoute路由
 *
 * \brief 包含情况时子目录将靠后，例如存在节点
 *
 * {"file://";"/home/uos"},
 *
 * 此时增加一个节点{"desktop://","/home/uos/.desktop"}
 *
 * 那么当前list存在的排布情况为{file,desktop}
 *
 * 解析时逆序查找，insert会插入当前节点后
 */

/*!
 * \brief schemeMapRoot 注册scheme到root的关联数据
 *
 *  {"main","/"} 返回 true
 *
 *  {"desktop","/home/user/Desktop"} 返回 false
 *
 * \param scheme url前缀
 *
 * \param root 根路径
 *
 * \param virtPath 是否是虚拟路径，无本地路径与Url的关联映射，
 *
 * \return 是根节点则返回true，否则返回false
 */
bool UrlRoute::schemeMapRoot(const QString &scheme,
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
        auto itera = SchemeMapLists.end();
        while(itera != SchemeMapLists.begin()) {
            -- itera;
            //如果当前注册的映射根是注册过的任意根的子目录
            //并且非虚拟路径
            if (itera->root().endsWith(formatRoot)) {
                //放到注册节点之后
                SchemeMapLists.insert(itera,{scheme, formatRoot, icon ,isVirtual});
                return true;
            }
        }
    }

    //是虚拟路径则直接添加到最后
    SchemeMapLists.append({scheme, root, icon, isVirtual});
    return true;
}
/* @method schemeIcon
 * @brief 获取scheme注册的图标
 * @param scheme url前缀
 * @return QIcon 无则返回空
 */
QIcon UrlRoute::schemeIcon(const QString &scheme)
{
    for (auto val: SchemeMapLists) {
        if (val.scheme() == scheme) return val.icon();
    }
    return QIcon();
}
//判断是否注册Scheme,该函数虚拟路径可用
bool UrlRoute::hasScheme(const QString &scheme) {
    for (auto val : SchemeMapLists) {
        if (val.scheme() == scheme) return true;
    }
    return false;
}
//path转换成本地的 Url(file://xxx)
//该函数虚拟路径不可用
QUrl UrlRoute::fromLocalFile(const QString &path, QString *errorString)
{
    if (!QFileInfo(path).exists()) {
        if (errorString)
            *errorString = QObject::tr("path not exists");
        return QUrl();
    }
    return QUrl::fromLocalFile(path);
}
//使用scheme找到对应注册时的根路径，
//该函数虚拟路径可用
bool UrlRoute::isSchemeRoot(const QUrl &url)
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
//使用已注册的scheme进行本地路径的转换
bool UrlRoute::isVirtualUrl(const QUrl &url)
{
    auto itera = SchemeMapLists.end();
    while (itera != SchemeMapLists.begin())
    {
        -- itera;

        auto scheme = itera->scheme();

        if(scheme == url.scheme()) {
            return itera->isVirtual();
        }
    }
    return true;
}
//使用以注册的scheme进行虚拟路径到绝对路径的转换，
QUrl UrlRoute::urlParent(const QUrl &url) {
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

//查找当前Url的前节点，如果当前前节点为根路径则直接返回传入Url
//{"home","/home/user"} 返回 {"home","/home/"}
//{"main","/"} 返回 {"main","/"}
//该函数虚拟路径可用
QString UrlRoute::schemeRoot(const QString &scheme, QString *errorString)
{
    //按照子节点逆序规则选中
    auto itera = SchemeMapLists.end();
    while (itera != SchemeMapLists.begin())
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
/* @method isSchemeRoot
 * @brief 判断当前Url是否为顶层
 *  {"main","/"} 返回 true
 *  {"desktop","/home/user/Desktop"} 返回 false
 * @return 是根节点则返回true，否则返回false
 */
bool UrlRoute::schemeIsVirtual(const QString &scheme)
{
    if (scheme.isEmpty())
        return true;

    for (auto node : SchemeMapLists) {
        if (node.scheme() == scheme)
        {
            return node.isVirtual();
        }
    }

    return false;
}
/* @method isVirtualUrl
 * @brief 判断当前Url是否是虚拟路径
 * @return 是虚拟路径则返回true，否则返回false
 */
QUrl UrlRoute::pathToVirtual(const QString &path, QString *errorString)
{
    auto itera = SchemeMapLists.end();
    while (itera != SchemeMapLists.begin())
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
//查找当前Url的前根节点，如果存在如下列表项
//{"home","/home/user"},{"desktop","/home/user/Desktop"}
//如果传入QUrl("desktop:///123"),
//则当前函数返回QUrl{"home:///"}，指向的真实地址为"/home/user"
//    static QUrl urlParentRoot(const QUrl &url);

/* @method pathToVirtual
 * @brief 使用path转换到虚拟路径
 * 存在注册节点 {"virtual":"/"}；
 * 此时传入"/home" 则返回 QUrl("virtual:///home")
 * @param path可以任意,不会进行本地路径检查,但是需要满足根条件
 * @return 返回注册的Scheme虚拟Url
 */
QString UrlRoute::virtualToPath(const QUrl &url, QString *errorString)
{
    QString path = "";
    for (auto val : SchemeMapLists) {

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
/* @method virtualToPath
 * @brief 使用虚拟路径转化成Path，内部不会进行路径检查
 *  url可以是任意注册的url，需要满足已scheme的要求
 *  存在注册节点 {"virtual":"/home"}
 *  传入QUrl("virtual:///home") 则返回 /home/home
 * @return 返回虚拟Url对应的虚拟路径
 */
QUrl UrlRoute::pathToUrl(const QString &path, QString *errorString)
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

    auto itera = SchemeMapLists.end();
    while (itera != SchemeMapLists.begin())
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

QString UrlRoute::urlToPath(const QUrl &url, QString *errorString)
{
    QString path = "";
    for (auto val : SchemeMapLists) {

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
    return myIcon;
}

void SchemeNode::setIcon(const QIcon &icon)
{
    myIcon = icon;
}

SchemeNode::SchemeNode(const QString &scheme, const QString &root,const QIcon &icon ,const bool isVirtual)
    : myScheme(scheme),
      myRoot(root),
      myIcon(icon),
      isMyVirtual(isVirtual)
{

}

SchemeNode &SchemeNode::operator =(const SchemeNode &node)
{
    myScheme = node.scheme();
    myRoot = node.root();
    isMyVirtual = node.isVirtual();
    return *this;
}

QString SchemeNode::scheme() const
{
    return myScheme;
}

void SchemeNode::setScheme(const QString &scheme)
{
    myScheme = scheme;
}

QString SchemeNode::root() const
{
    return myRoot;
}

void SchemeNode::setRoot(const QString &root)
{
    myRoot = root;
}

bool SchemeNode::isVirtual() const
{
    return isMyVirtual;
}

void SchemeNode::setIsVirtual(bool isVirtual)
{
    isMyVirtual = isVirtual;
}

DFMBASE_END_NAMESPACE
