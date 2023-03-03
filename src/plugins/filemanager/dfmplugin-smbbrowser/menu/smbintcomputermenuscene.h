// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SmbIntComputerMenuScene_H
#define SmbIntComputerMenuScene_H

#include "dfmplugin_smbbrowser_global.h"

#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

namespace dfmplugin_smbbrowser {

class SmbIntComputerMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    inline static QString name()
    {
        return "SmbIntComputerSubMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class SmbIntComputerMenuScenePrivate;
class SmbIntComputerMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit SmbIntComputerMenuScene(QObject *parent = nullptr);
    virtual ~SmbIntComputerMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<SmbIntComputerMenuScenePrivate> d;
};

}

#endif   // SmbIntComputerMenuScene_H
