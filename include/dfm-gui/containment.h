// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTAINMENT_H
#define CONTAINMENT_H

#include <dfm-gui/applet.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>

#include <QObject>

DFMGUI_BEGIN_NAMESPACE

class ContainmentPrivate;
class Containment : public Applet
{
    Q_OBJECT
    Q_PROPERTY(QList<Applet *> applets READ applets NOTIFY appletsChanged FINAL)

public:
    explicit Containment(QObject *parent = nullptr);
    ~Containment() override;

    Applet *createApplet(const dpf::PluginQuickMetaPtr &metaPtr);

    void appendApplet(Applet *applet);
    void removeApplet(Applet *applet);
    QList<Applet *> applets() const;
    Q_SIGNAL void appletAdded(Applet *applet);
    Q_SIGNAL void appletRemoved(Applet *applet);
    Q_SIGNAL void appletsChanged();
    // Applet 关联的 QML 组件变更(初始化结束)时触发
    Q_SIGNAL void appletRootObjectChanged(QObject *rootObject);

    Q_INVOKABLE Applet *appletForProperty(const QString &property, const QVariant &var);
    Q_INVOKABLE QObject *objectForProperty(const QString &property, const QVariant &var);

protected:
    explicit Containment(ContainmentPrivate &dd, QObject *parent = nullptr);

private:
    Q_DECLARE_PRIVATE_D(dptr, Containment)
    Q_DISABLE_COPY(Containment)

    friend class ContainmentItem;
};

DFMGUI_END_NAMESPACE

#endif   // CONTAINMENT_H
