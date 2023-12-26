// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/fileutils.h>

#include <QDir>
#include <QRegularExpression>

namespace dfmbase {

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
                         const QString &displayName,
                         QString *errorString)
{
    QString error;
    FinallyUtil finally([&]() {if (errorString) *errorString = error; });
    if (hasScheme(scheme)) {
        error = "Scheme cannot be registered repeatedly.";
        return false;
    }
    // 统一处理路径在最后加上 "/"
    QString formatRoot = root;
    if (!root.endsWith("/"))
        formatRoot = root + "/";

    // 非虚拟路径则进行本地路径判断
    if (!isVirtual) {
        if (!QDir().exists(formatRoot)) {
            error = "Scheme map to root path not exists.";
            return false;
        }

        QString temp = formatRoot;
        temp.replace(QRegularExpression("/{1,}"), "/");
        int treeLevel = temp.count("/") - 1;
        kSchemeRealTree.insert(treeLevel, scheme);   // 缓存层级
    }
    kSchemeInfos.insert(scheme, { formatRoot, icon, isVirtual, displayName });
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

QString UrlRoute::toString(const QUrl &url, QUrl::FormattingOptions options)
{
    if (!url.isValid())
        return "";

    if (dfmbase::FileUtils::isLocalFile(url) || !hasScheme(url.scheme()))
        return url.toString(options);

    QUrl tmpUrl { url };
    tmpUrl.setScheme(Global::Scheme::kFile);

    return tmpUrl.toString(options).replace(0, strlen(Global::Scheme::kFile), url.scheme());
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
        return true;

    QString path = urlToPath(url);

    QUrl urlCmp;
    urlCmp.setScheme(url.scheme());
    urlCmp.setPath(kSchemeInfos[url.scheme()].rootPath());

    if (url.scheme() == urlCmp.scheme()
        && path == urlCmp.path())
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
    // todo liyigang need a rule to get parenturl
    const QString &path = url.path();

    if (path == "/")
        return QUrl();
    QUrl reUrl = url;
    if (url.scheme() == Global::Scheme::kRecent) {
        reUrl.setPath("/");
        return reUrl;
    }
    QStringList paths = path.split("/");
    paths.removeAt(0);
    if (!paths.isEmpty() && paths.last().isEmpty())
        paths.removeLast();

    if (!paths.isEmpty())
        paths.removeLast();

    QString curPath;
    for (QString p : paths)
        curPath += "/" + p;

    if (curPath.isEmpty() && url.host().isEmpty())
        curPath += "/";

    reUrl.setPath(curPath);

    return reUrl;
}

bool UrlRoute::isAncestorsUrl(QUrl url, const QUrl &ancestorsUrl, QList<QUrl> *list)
{
    Q_ASSERT(list);

    while (!isRootUrl(url)) {
        QUrl parent { urlParent(url) };
        list->append(parent);
        url = parent;
        if (parent == ancestorsUrl)
            return true;
    }

    return false;
}

void UrlRoute::urlParentList(QUrl url, QList<QUrl> *list)
{
    Q_ASSERT(list);

    while (!isRootUrl(url)) {
        QUrl parent { urlParent(url) };
        list->append(parent);
        url = parent;
    }
}

/*!
 * \brief UrlRoute::rootDisplayName get the display name of a root url
 * \param scheme which has been registered
 * \return if an unregistered scheme is passed then returns an empty string, otherwise returns the registered display name of scheme
 */
QString UrlRoute::rootDisplayName(const QString &scheme)
{
    if (!hasScheme(scheme))
        return "";
    return kSchemeInfos[scheme].displayName();
}

QUrl UrlRoute::fromUserInput(const QString &userInput, bool preferredLocalPath)
{
    return fromUserInput(userInput, QString(), preferredLocalPath);
}

QUrl UrlRoute::fromUserInput(const QString &userInput, QString workingDirectory, bool preferredLocalPath, QUrl::UserInputResolutionOptions options)
{
    if (options != QUrl::AssumeLocalFile) {
        return QUrl(QUrl::fromUserInput(userInput, workingDirectory, options));
    }

    if ((userInput.startsWith("~") && preferredLocalPath) || userInput.startsWith("~/")) {
        return QUrl::fromLocalFile(QDir::homePath() + userInput.mid(1));
    } else if ((preferredLocalPath && QDir().exists(userInput)) || userInput.startsWith("./")
               || userInput.startsWith("../") || userInput.startsWith("/")) {
        QDir dir(userInput);

        return QUrl::fromLocalFile(dir.absolutePath());
    } else {
        QUrl url(userInput);
        return url;
    }
}

QList<QUrl> UrlRoute::fromStringList(const QStringList &strList)
{
    QList<QUrl> urls;
    urls.reserve(strList.size());
    for (const auto &str : strList)
        urls << fromUserInput(str);

    return urls;
}

QByteArray UrlRoute::urlsToByteArray(const QList<QUrl> &list)
{
    QByteArray urlByteArray;
    QDataStream stream(&urlByteArray, QIODevice::WriteOnly);
    stream << list;
    return urlByteArray;
}

QList<QUrl> UrlRoute::byteArrayToUrls(const QByteArray &arr)
{
    QList<QUrl> urlList;
    QByteArray urlByteArray { arr };
    QDataStream stream(&urlByteArray, QIODevice::ReadOnly);
    stream >> urlList;
    return urlList;
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

QUrl UrlRoute::rootUrl(const QString &scheme)
{
    if (!hasScheme(scheme))
        return QUrl();

    QUrl root;
    root.setScheme(scheme);
    root.setPath(rootPath(scheme));
    return root;
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
            if (val == dfmbase::Global::Scheme::kAsyncFile)
                continue;
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
    result.setHost("");
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

QString UrlRoute::urlToLocalPath(const QUrl &url)
{
    if (!url.isValid())
        return {};
    QUrl localUrl { url };
    localUrl.setScheme(Global::Scheme::kFile);
    return localUrl.toString().replace(0, 4, url.scheme());
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
SchemeNode::SchemeNode(const QString &root, const QIcon &icon, const bool isVirtual, const QString &displayName)
    : path(root),
      icon(icon),
      virtualFlag(isVirtual),
      name(displayName)
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

QString SchemeNode::displayName() const
{
    return name;
}

}
