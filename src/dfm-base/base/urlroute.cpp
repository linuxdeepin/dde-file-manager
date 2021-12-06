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
#include "urlroute.h"

#include "utils/finallyutil.h"

#include <QDir>
#include <QUrl>
#include <QRegularExpression>

DFMBASE_BEGIN_NAMESPACE

namespace SchemeTypes {
const char *const kFile = "file";
const char *const kDesktop = "desktop";
const char *const kHome = "home";
const char *const kVideos = "videos";
const char *const kMusic = "music";
const char *const kPictures = "pictures";
const char *const kDocuments = "documents";
const char *const kDownloads = "downloads";
const char *const kRoot = "dfmroot";
}   // namespace SchemeTypes

QHash<QString, SchemeNode> UrlRoute::kSchemeInfos {};
QMultiMap<int, QString> UrlRoute::kSchemeRealTree {};

/*!
   \class UrlRoute
 * \brief 统一资源定位符路由，最基础的功能
 * 使用前，你需要调用RegScheme注册scheme与映射路径。
 * 该类支持虚拟路径与真实路径两种状态。
 * 该类是QUrl的延伸，应对Abstract继承树族的任意策略。
 */

/*!
 * \brief UrlRoute::regScheme 注册scheme
 * \param scheme url前缀
 * \param root 映射的跟路径
 * \param icon 前缀所包含的头
 * \param isVirtual 是否为虚拟映射
 * \param[out] errorString 错误信息
 * \return
 */
bool UrlRoute::regScheme(const QString &scheme,
                         const QString &root,
                         const QIcon &icon,
                         const bool isVirtual,
                         QString *errorString)
{
    QString error;
    FinallyUtil finally([&]() {if (errorString) *errorString = error; });
    if (hasScheme(scheme)) {
        error = QObject::tr("Scheme cannot be registered repeatedly.");
        return false;
    }
    // 统一处理路径在最后加上 "/"
    QString formatRoot = root;
    if (!root.endsWith("/"))
        formatRoot = root + "/";

    // 非虚拟路径则进行本地路径判断
    if (!isVirtual) {
        if (!QDir().exists(formatRoot)) {
            error = QObject::tr("Scheme map to root path not exists.");
            return false;
        }

        QString temp = formatRoot;
        temp.replace(QRegularExpression("/{1,}"), "/");
        int treeLevel = temp.count("/") - 1;
        kSchemeRealTree.insert(treeLevel, scheme);   // 缓存层级
    }
    kSchemeInfos.insert(scheme, { formatRoot, icon, isVirtual });
    finally.dismiss();
    return true;
}

/*!
 * \brief UrlRoute::icon 获取注册的Icon图标
 * \param scheme url前缀
 * \return regScheme注册的映射图标
 */
QIcon UrlRoute::icon(const QString &scheme)
{
    if (!hasScheme(scheme))
        return QIcon();
    return kSchemeInfos[scheme].pathIcon();
}

/*!
 * \brief UrlRoute::hasScheme 判断是否存在Scheme
 * \param scheme 要查找的url前缀
 * \return 如果注册则为true，反之false
 */
bool UrlRoute::hasScheme(const QString &scheme)
{
    return kSchemeInfos.keys().contains(scheme);
}

/*!
 * \brief UrlRoute::isRootUrl 判断当前Url是否为映射的跟url
 * \param url 判断的url
 * \return
 */
bool UrlRoute::isRootUrl(const QUrl &url)
{
    if (!hasScheme(url.scheme()))
        return false;

    QUrl urlCmp;
    urlCmp.setScheme(url.scheme());
    urlCmp.setPath(kSchemeInfos[url.scheme()].rootPath());

    if (url == urlCmp)
        return true;

    return false;
}

/*!
 * \brief UrlRoute::isVirtualUrl 判断当前url是否为虚拟的url
 * \param url 参数url
 * \return 超出regScheme边界默认返回false，如果注册则返回注册时isVirtual形参
 */
bool UrlRoute::isVirtual(const QUrl &url)
{
    if (!hasScheme(url.scheme()))
        return false;

    return kSchemeInfos[url.scheme()].virtualFlag;
}

/*!
 * \brief UrlRoute::urlParent 纯分割函数找到前节点
 * \param url 源Url
 * \return 返回处理后的url
 */
QUrl UrlRoute::urlParent(const QUrl &url)
{
    if (isRootUrl(url))
        return url;

    auto &&list = url.path().split("/");
    list.removeAt(list.size() - 1);

    QUrl reUrl;
    url.scheme();
    reUrl.setScheme(url.scheme());
    reUrl.setPath(list.join("/"));

    return reUrl;
}

/*!
 * \brief UrlRoute::rootPath 获取根路径
 * \param scheme 已注册的scheme
 * \return 如果传入未注册scheme，则总是返回空，否则将返回注册时指定的path形参
 */
QString UrlRoute::rootPath(const QString &scheme)
{
    if (!hasScheme(scheme))
        return "";
    return kSchemeInfos[scheme].path;
}

/*!
 * \brief UrlRoute::pathToReal 转换成本地映射的路径
 *  如果注册了其他scheme，例如{downloads,/home/user/Downloads}
 *  此时传入 /home/user/Downloads/aaa.txt，则转换为
 *  downloads:///aaa.txt。此转换只本地存在路径映射
 *  如果没有注册 downloads，则向上继续检索，如果注册了{file，/}
 *  则转换为file:///home/user/Downloads,
 *  总体来说这是QUrl的扩展，新增了自定义本地映射结构。
 *  使用该机制可以构造scheme的组件空间，以应对低速设备等特殊要求，
 *  避免文管走file通用流程导致的卡顿
 * \param path 本地路径
 * \return 转换后的路径
 */
QUrl UrlRoute::pathToReal(const QString &path)
{
    QString temp = path;
    temp.replace(QRegularExpression("/{1,}"), "/");
    int treeLevel = temp.count("/");
    while (treeLevel >= 0) {
        //同层级所有的scheme
        auto &&schemeList = kSchemeRealTree.values(treeLevel);
        for (auto val : schemeList) {
            // 包含映射的根路径，判断是否转换为当前scheme
            QString rootPath = kSchemeInfos[val].rootPath();
            if (path.contains(rootPath)
                || QString(path + "/").contains(rootPath)) {
                QUrl result = pathToUrl(path, val);
                return result;
            }
        }
        treeLevel -= 1;
    }
    return QUrl();
}

/*!
 * \brief UrlRoute::fromLocalFile 同QUrl::fromLocalFile转换
 * \param path 本地路径
 * \return 转换后的Url
 */
QUrl UrlRoute::fromLocalFile(const QString &path)
{
    return QUrl::fromLocalFile(path);
}

/*!
 * \brief UrlRoute::isVirtualScheme 判断是否虚拟的Url前缀
 * \param scheme url前缀
 * \return 默认未注册范围false，注册则返回regScheme isVirtual形参结果
 */
bool UrlRoute::isVirtual(const QString &scheme)
{
    if (!hasScheme(scheme))
        return false;
    return kSchemeInfos[scheme].isVirtual();
}

/*!
 * \brief UrlRoute::pathToUrl 使用scheme进行路径转换，非本地文件映射则必须使用该函数
 * \param path 路径，可以是本地或者虚拟
 * \param scheme url前缀
 * \return 转换后的路径
 */
QUrl UrlRoute::pathToUrl(const QString &path, const QString &scheme)
{
    if (!hasScheme(scheme))
        return QUrl();

    QString rootPath = kSchemeInfos[scheme].rootPath();
    if (rootPath.isEmpty())
        return QUrl();

    QString tempPath = path;
    tempPath = tempPath.replace(0, rootPath.size(), "/");   //制空映射前缀
    QUrl result;
    result.setScheme(scheme);
    result.setPath(tempPath);
    return result;
}

/*!
 * \brief UrlRoute::urlToPath url转换为路径，虚拟路径与真实本地映射均可使用
 * \param url 带有regScheme注册前缀的url
 * \return
 */
QString UrlRoute::urlToPath(const QUrl &url)
{
    if (!hasScheme(url.scheme()))
        return "";

    QString result = kSchemeInfos[url.scheme()].rootPath() + url.path();
    result.replace(QRegularExpression("/{1,}"), "/");
    return result;
}

/*!
 * \brief SchemeNode::icon 获取文件icon
 * \return QIcon 文件的ICON实例
 */
QIcon SchemeNode::pathIcon() const
{
    return icon;
}

/*!
 * \brief 路由Url注册节点类
 */
SchemeNode::SchemeNode(const QString &root, const QIcon &icon, const bool isVirtual)
    : path(root),
      icon(icon),
      virtualFlag(isVirtual)
{
}

SchemeNode &SchemeNode::operator=(const SchemeNode &node)
{
    path = node.rootPath();
    virtualFlag = node.isVirtual();
    return *this;
}

bool SchemeNode::isEmpty()
{
    return path.isEmpty();
}

QString SchemeNode::rootPath() const
{
    return path;
}

bool SchemeNode::isVirtual() const
{
    return virtualFlag;
}

DFMBASE_END_NAMESPACE
