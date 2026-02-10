// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEMENUSCENE_H
#define TEMPLATEMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

#include <mutex>

namespace dfmplugin_menu {

class TemplateMenu;
class TemplateMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "TemplateMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
protected:
    TemplateMenu *templateMenu = nullptr;
    std::once_flag loadFlag;
};

class TemplateMenuScenePrivate;
class TemplateMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit TemplateMenuScene(TemplateMenu *menu, QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    TemplateMenuScenePrivate *const d = nullptr;
};

}

#endif   // TEMPLATEMENUSCENE_H
