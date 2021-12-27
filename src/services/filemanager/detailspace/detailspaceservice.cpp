/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "detailspaceservice.h"
#include "private/detailspaceservice_p.h"

DSB_FM_USE_NAMESPACE

/*!
 * \class DetailSpaceServicePrivate
 * \brief
 */

DetailSpaceServicePrivate::DetailSpaceServicePrivate(DetailSpaceService *serv)
    : QObject(nullptr), service(serv)
{
}

/*!
 * \class DetailSpaceService
 * \brief
 */

DetailSpaceService::DetailSpaceService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<DetailSpaceService>(),
      d(new DetailSpaceServicePrivate(this))
{
}

DetailSpaceService::~DetailSpaceService()
{
}
