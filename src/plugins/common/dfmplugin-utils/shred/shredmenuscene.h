// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHREDMENUSCENE_H
#define SHREDMENUSCENE_H

#include "dfmplugin_utils_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_utils {

class ShredMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "ShredMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class ShredMenuScenePrivate;
class ShredMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit ShredMenuScene(QObject *parent = nullptr);
    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    ShredMenuScenePrivate *const d;
};

}   // namespace dfmplugin_utils

#endif   // SHREDMENUSCENE_H
