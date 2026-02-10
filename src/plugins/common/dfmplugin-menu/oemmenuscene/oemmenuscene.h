// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OEMMENUSCENE_H
#define OEMMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>
#include <mutex>

namespace dfmplugin_menu {

class OemMenu;
class OemMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "OemMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
protected:
    OemMenu *oemMenu = nullptr;
    std::once_flag loadFlag;
};

class OemMenuScenePrivate;
class OemMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit OemMenuScene(OemMenu *oem, QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    OemMenuScenePrivate *const d = nullptr;
};

}
#endif   // OEMMENUSCENE_H
