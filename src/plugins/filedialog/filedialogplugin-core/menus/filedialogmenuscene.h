/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FILEDIALOGMENUSCENE_H
#define FILEDIALOGMENUSCENE_H

#include "filedialogplugin_core_global.h"

#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

namespace filedialog_core {

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

private:
    QString findSceneName(QAction *act) const;
    void filterAction(QMenu *parent, bool isSubMenu);

private:
    AbstractMenuScene *workspaceScene { nullptr };
};

}

#endif   // FILEDIALOGMENUSCENE_H
