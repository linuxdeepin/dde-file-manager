// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLETMANAGER_H
#define APPLETMANAGER_H

#include <dfm-gui/dfm_gui_global.h>
#include <dfm-framework/lifecycle/pluginquickmetadata.h>

#include <QObject>

DFMGUI_BEGIN_NAMESPACE

class Applet;
class Containment;
class Panel;

class AppletManagerPrivate;
class AppletManager : public QObject
{
    Q_OBJECT

    explicit AppletManager(QObject *parent = nullptr);
    ~AppletManager();

public:
    static AppletManager *instance();

    void initRootTemplates(const QString &configFile = {});
    bool registeApplet(const dpf::PluginQuickMetaPtr &infoPtr);
    bool unRegisteApplet(const QString &pluginName, const QString &quickId);

    QList<QString> panelIdList() const;
    Panel *createPanel(const QString &templateId);
    Panel *createPanel(const QString &pluginName, const QString &quickId);
    void fillChildren(Containment *containment);

    QList<QString> allAppletTemplateIdList() const;
    Applet *createAppletRecursive(const QString &templateId, Containment *parent = nullptr);
    Applet *createAppletRecursive(const QString &pluginName, const QString &quickId, Containment *parent = nullptr);

    QString lastError() const;

    static Applet *createAppletFromInfo(
            const dpf::PluginQuickMetaPtr &metaPtr, Containment *parent = nullptr, QString *errorString = nullptr);

private:
    QScopedPointer<AppletManagerPrivate> dptr;
    Q_DECLARE_PRIVATE_D(dptr, AppletManager)
    Q_DISABLE_COPY_MOVE(AppletManager)
};

DFMGUI_END_NAMESPACE

#endif   // APPLETMANAGER_H
