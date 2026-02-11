// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDMENUSCENE_H
#define EXTENDMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

#include <mutex>

namespace dfmplugin_menu {
class DCustomActionParser;
class ExtendMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "ExtendMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
protected:
    DCustomActionParser *customParser = nullptr;
    std::once_flag loadFlag;
};

class ExtendMenuScenePrivate;
class ExtendMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit ExtendMenuScene(DCustomActionParser *parser, QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    ExtendMenuScenePrivate *const d = nullptr;
};

}
#endif   // EXTENDMENUSCENE_H
