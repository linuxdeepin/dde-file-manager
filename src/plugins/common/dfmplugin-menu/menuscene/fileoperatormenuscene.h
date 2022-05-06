/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef FILEOPERATORMENUSCENE_H
#define FILEOPERATORMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

DPMENU_BEGIN_NAMESPACE

class FileOperatorMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "FileOperatorMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class FileOperatorMenuScenePrivate;
class FileOperatorMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit FileOperatorMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    FileOperatorMenuScenePrivate *const d = nullptr;
};

DPMENU_END_NAMESPACE
#endif   // FILEOPERATORMENUSCENE_H
