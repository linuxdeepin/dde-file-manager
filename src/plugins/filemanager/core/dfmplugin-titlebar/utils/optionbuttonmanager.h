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
#ifndef OPTIONBUTTONMANAGER_H
#define OPTIONBUTTONMANAGER_H

#include "dfmplugin_titlebar_global.h"

#include <QObject>
#include <QHash>

namespace dfmplugin_titlebar {

class OptionButtonManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OptionButtonManager)

public:
    enum OptBtnVisibleState {
        kDoNotHide = 0x00,
        kHideListViewBtn = 0x01,
        kHideIconViewBtn = 0x02,
        kHideDetailSpaceBtn = 0x04,
        kHideHideSearchBtn = 0x05
    };

    using Scheme = QString;
    using OptBtnVisibleStateMap = QHash<Scheme, OptBtnVisibleState>;

public:
    static OptionButtonManager *instance();

    // TODO(zhangs): setCustomActionList
    void setOptBtnVisibleState(const Scheme &scheme, OptBtnVisibleState state);
    OptBtnVisibleState optBtnVisibleState(const Scheme &scheme) const;
    bool hasVsibleState(const Scheme &scheme) const;

private:
    explicit OptionButtonManager(QObject *parent = nullptr);

private:
    OptBtnVisibleStateMap stateMap;
};

}

#endif   // OPTIONBUTTONMANAGER_H
