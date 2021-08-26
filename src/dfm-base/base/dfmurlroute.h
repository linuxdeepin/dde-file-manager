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

#include <QUrl>
#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QMap>
#include <QDebug>
#include <QIcon>

#include <list>

/* @class 路由Url注册节点类
 */
struct SchemeNode
{
    QString m_scheme; //url前缀
    QString m_root;   //根路径
    QIcon m_icon;     //标志root的图标
    bool m_isVirtual; //没有本地路径作为映射的Url

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


/* @class url路由
 */
class DFMUrlRoute
{
    //包含情况时子目录将靠后，例如存在节点
    //{"file://";"/home/uos"},
    //此时增加一个节点{"desktop://","/home/uos/.desktop"}
    //那么当前list存在的排布情况为{file,desktop}
    //解析时逆序查找，insert会插入当前节点后
    static QList<SchemeNode> m_schemeMapLists;

public:

    //


    /* @method schemeMapRoot
     * @brief 注册scheme到root的关联数据
     *  {"main","/"} 返回 true
     *  {"desktop","/home/user/Desktop"} 返回 false
     * @param scheme url前缀
     * @param root 根路径
     * @param virtPath 是否是虚拟路径，无本地路径与Url的关联映射，
     * @return 是根节点则返回true，否则返回false
     */
    static bool schemeMapRoot(const QString &scheme,
                              const QString &root,
                              const QIcon &icon = QIcon(),
                              const bool isVirtual = false,
                              QString *errorString = nullptr);

    /* @method schemeIcon
     * @brief 获取scheme注册的图标
     * @param scheme url前缀
     * @return QIcon 无则返回空
     */
    static QIcon schemeIcon(const QString &scheme);

    //判断是否注册Scheme,该函数虚拟路径可用
    static bool hasScheme(const QString &scheme);

    //path转换成本地的 Url(file://xxx)
    //该函数虚拟路径不可用
    static QUrl fromLocalFile(const QString &path,
                              QString *errorString = nullptr);

    //使用scheme找到对应注册时的根路径，
    //该函数虚拟路径可用
    static QString schemeRoot(const QString &scheme,
                              QString *errorString = nullptr);

    static bool schemeIsVirtual(const QString &scheme);

    //使用已注册的scheme进行本地路径的转换
    static QUrl pathToUrl(const QString &path,
                          QString *errorString = nullptr);

    //使用以注册的scheme进行虚拟路径到绝对路径的转换，
    static QString urlToPath(const QUrl &url,
                             QString *errorString = nullptr);

    //查找当前Url的前节点，如果当前前节点为根路径则直接返回传入Url
    //{"home","/home/user"} 返回 {"home","/home/"}
    //{"main","/"} 返回 {"main","/"}
    //该函数虚拟路径可用
    static QUrl urlParent(const QUrl &url);

    /* @method isSchemeRoot
     * @brief 判断当前Url是否为顶层
     *  {"main","/"} 返回 true
     *  {"desktop","/home/user/Desktop"} 返回 false
     * @return 是根节点则返回true，否则返回false
     */
    static bool isSchemeRoot(const QUrl &url);

    /* @method isVirtualUrl
     * @brief 判断当前Url是否是虚拟路径
     * @return 是虚拟路径则返回true，否则返回false
     */
    static bool isVirtualUrl(const QUrl &url);

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
    static QUrl pathToVirtual(const QString &path,
                              QString *errorString = nullptr);

    /* @method virtualToPath
     * @brief 使用虚拟路径转化成Path，内部不会进行路径检查
     *  url可以是任意注册的url，需要满足已scheme的要求
     *  存在注册节点 {"virtual":"/home"}
     *  传入QUrl("virtual:///home") 则返回 /home/home
     * @return 返回虚拟Url对应的虚拟路径
     */
    static QString virtualToPath(const QUrl &url,
                                 QString *errorString = nullptr);

};

Q_DECLARE_METATYPE(QUrl);

#endif // URLROUTE_H
