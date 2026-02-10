// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALENTRYMENUSCENE_H
#define VIRTUALENTRYMENUSCENE_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

#include <QObject>

DPSMBBROWSER_BEGIN_NAMESPACE

class VirtualEntryMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    inline static QString name()
    {
        return "VirtualEntry";
    }
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class VirtualEntryMenuScenePrivate;
class VirtualEntryMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit VirtualEntryMenuScene(QObject *parent = nullptr);
    virtual ~VirtualEntryMenuScene() override;

    // AbstractMenuScene interface
    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual dfmbase::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<VirtualEntryMenuScenePrivate> d;
};

DPSMBBROWSER_END_NAMESPACE

#endif   // VIRTUALENTRYMENUSCENE_H
