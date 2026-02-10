// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONLIBMENUSCENE_H
#define EXTENSIONLIBMENUSCENE_H

#include "dfmplugin_utils_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_utils {

class ExtensionLibMenuSceneCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "ExtensionLibMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class ExtensionLibMenuScenePrivate;
class ExtensionLibMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT

public:
    explicit ExtensionLibMenuScene(QObject *parent = nullptr);
    ~ExtensionLibMenuScene() override;

    QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<ExtensionLibMenuScenePrivate> d;
};

}   // namespace dfmplugin_utils

#endif   // EXTENSIONLIBMENUSCENE_H
