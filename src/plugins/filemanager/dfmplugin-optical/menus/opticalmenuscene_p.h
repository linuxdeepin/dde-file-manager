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
#ifndef OPTICALMENUSCENE_P_H
#define OPTICALMENUSCENE_P_H

#include "dfmplugin_optical_global.h"

#include "dfm-base/interfaces/private/abstractmenuscene_p.h"

DPOPTICAL_BEGIN_NAMESPACE

class OpticalMenuScene;
class OpticalMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class OpticalMenuScene;

public:
    explicit OpticalMenuScenePrivate(OpticalMenuScene *qq);
    QString findSceneName(QAction *act) const;

private:
    OpticalMenuScene *q;
    bool isBlankDisc { false };
};

DPOPTICAL_END_NAMESPACE

#endif   // OPTICALMENUSCENE_P_H
