/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef TRASHCOREPLUGIN_H
#define TRASHCOREPLUGIN_H

#include "dfmplugin_trashcore_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_trashcore {
class TrashCore : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "trashcore.json")

    DPF_EVENT_NAMESPACE(DPTRASHCORE_NAMESPACE)
    DPF_EVENT_REG_SIGNAL(signal_TrashCore_TrashStateChanged)

    DPF_EVENT_REG_SLOT(slot_TrashCore_EmptyTrash)

public:
    virtual void initialize() override;
    virtual bool start() override;
};
}   // namespace dfmplugin_trashcore

#endif   // TRASHCOREPLUGIN_H
