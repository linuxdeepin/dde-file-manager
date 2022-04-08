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
#ifndef DETAILSPACE_DEFINES_H
#define DETAILSPACE_DEFINES_H

#include "dfm_filemanager_service_global.h"

DSB_FM_BEGIN_NAMESPACE

namespace DetailEventType {
extern const int kShowDetailView;
extern const int kSetDetailViewSelectFileUrl;
}

DSB_FM_END_NAMESPACE
#define DTSP_NAMESPACE detailsapce_service
#define DTSP_BEGIN_NAMESPACE namespace DTSP_NAMESPACE {
#define DTSP_END_NAMESPACE }
#define DTSP_USE_NAMESPACE using namespace DTSP_NAMESPACE;

#endif   // DETAILSPACE_DEFINES_H
