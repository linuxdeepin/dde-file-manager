// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        kHideTreeViewBtn = 0x08,

        kHideAllBtn = kHideListViewBtn | kHideIconViewBtn | kHideDetailSpaceBtn | kHideTreeViewBtn,
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
