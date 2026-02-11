// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBBROWSERMENUSCENE_H
#define SMBBROWSERMENUSCENE_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_smbbrowser {

class SmbBrowserMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "SmbBrowserMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class SmbBrowserMenuScenePrivate;
class SmbBrowserMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit SmbBrowserMenuScene(QObject *parent = nullptr);
    virtual ~SmbBrowserMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<SmbBrowserMenuScenePrivate> d;
};

}
#endif   // SMBBROWSERMENUSCENE_H
