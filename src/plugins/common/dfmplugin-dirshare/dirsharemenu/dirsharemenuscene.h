// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIRSHAREMENUSCENE_H
#define DIRSHAREMENUSCENE_H

#include "dfmplugin_dirshare_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_dirshare {

class DirShareMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "DirShareMenu";
    }

    virtual dfmbase::AbstractMenuScene *create() override;
};

class DirShareMenuScenePrivate;
class DirShareMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit DirShareMenuScene(QObject *parent = nullptr);
    virtual ~DirShareMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<DirShareMenuScenePrivate> d;
};

}
#endif   // DIRSHAREMENUSCENE_H
