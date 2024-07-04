// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLETMANAGER_P_H
#define APPLETMANAGER_P_H

#include <dfm-gui/appletmanager.h>
#include <dfm-gui/applet.h>

DFMGUI_BEGIN_NAMESPACE

struct AppletTemplateNode
{
    typedef QSharedPointer<AppletTemplateNode> Ptr;

    Applet::Flags flag = Applet::kUnknown;   // 组件类型
    QString cachedId;   // 缓存ID [插件名.组件名]
    dpf::PluginQuickMetaPtr quickInfoPtr;   // 记录的 Applet 加载信息
    QWeakPointer<AppletTemplateNode> parent;
    QList<AppletTemplateNode::Ptr> childNode;
};

class AppletManagerPrivate
{
    Q_DECLARE_PUBLIC(AppletManager)

public:
    explicit AppletManagerPrivate(AppletManager *q);

    void parseDefaultRootTemplates();
    Applet *createAppletFromNode(const AppletTemplateNode::Ptr &node, Containment *parent = nullptr, bool recursive = true);
    void clear();

    static Applet::Flags flagFromString(const QString &typeString);
    inline QString generateId(const QString &pluginName, const QString &quickId)
    {
        return QString("%1.%2").arg(pluginName).arg(quickId);
    }

    AppletManager *q_ptr;
    AppletTemplateNode::Ptr root;   // 顶层模板节点
    QHash<QString, AppletTemplateNode::Ptr> cacheIDToNode;   // 缓存信息，<缓存ID，元信息节点>
    QString errorString;
};

DFMGUI_END_NAMESPACE

#endif   // APPLETMANAGER_P_H
