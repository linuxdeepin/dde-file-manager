/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#ifndef TRASHPLUGIN_H
#define TRASHPLUGIN_H

#include "dfmplugin_trash_global.h"

#include <dfm-framework/framework.h>

DPTRASH_BEGIN_NAMESPACE
class Trash : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "trash.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

private slots:
    void onTrashDisplayChanged(bool enabled);
    void regTrashCrumbToTitleBar();
    void onAllPluginsInitialized();
    void onWindowOpened(quint64 windId);

private:
    void installToSideBar();
    void addFileOperations();
    void addCustomTopWidget();
};

DPTRASH_END_NAMESPACE
#endif   // TRASHPLUGIN_H
