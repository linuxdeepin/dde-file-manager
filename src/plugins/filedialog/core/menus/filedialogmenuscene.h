// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOGMENUSCENE_H
#define FILEDIALOGMENUSCENE_H

#include "filedialogplugin_core_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace filedialog_core {

class FileDialogMenuScenePrivate;
class FileDialogMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "FileDialogMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class FileDialogMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit FileDialogMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    void updateState(QMenu *parent) override;
    bool actionFilter(AbstractMenuScene *caller, QAction *action) override;
    
private:
    QString findSceneName(QAction *act) const;
    void filterAction(QMenu *parent, bool isSubMenu);

private:
    QScopedPointer<FileDialogMenuScenePrivate> d;
    AbstractMenuScene *workspaceScene { nullptr };
};

}

#endif   // FILEDIALOGMENUSCENE_H
