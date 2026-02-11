// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SENDTODISCMENUSCENE_H
#define SENDTODISCMENUSCENE_H

#include "dfmplugin_burn_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_burn {

class SendToDiscMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "SendToDiscMenu";
    }

    virtual dfmbase::AbstractMenuScene *create() override;
};

class SendToDiscMenuScenePrivate;
class SendToDiscMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit SendToDiscMenuScene(QObject *parent = nullptr);
    virtual ~SendToDiscMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    void updateStageAction(QMenu *parent);

private:
    QScopedPointer<SendToDiscMenuScenePrivate> d;
};

}

#endif   // SENDTODISCMENUSCENE_H
