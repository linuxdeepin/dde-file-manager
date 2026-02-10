// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTMENUIMPL_P_H
#define DFMEXTMENUIMPL_P_H

#include "dfmplugin_utils_global.h"

#include <dfm-extension/menu/private/dfmextmenuprivate.h>

#include <QObject>

class QAction;
class QMenu;

DPUTILS_BEGIN_NAMESPACE

class DFMExtMenuImpl;
class DFMExtMenuImplPrivate : public QObject, public DFMEXT::DFMExtMenuPrivate
{
    Q_OBJECT
public:
    DFMExtMenuImplPrivate(DFMExtMenuImpl *qImpl, QMenu *m = nullptr);
    virtual ~DFMExtMenuImplPrivate() override;

    DFMExtMenuImpl *menuImpl() const;
    QMenu *qmenu() const;
    bool isInterior() const;

    std::string title() const Q_DECL_OVERRIDE;
    void setTitle(const std::string &title) Q_DECL_OVERRIDE;

    std::string icon() const Q_DECL_OVERRIDE;
    void setIcon(const std::string &iconName) Q_DECL_OVERRIDE;

    bool addAction(DFMEXT::DFMExtAction *action) Q_DECL_OVERRIDE;
    bool insertAction(DFMEXT::DFMExtAction *before, DFMEXT::DFMExtAction *action) Q_DECL_OVERRIDE;

    DFMEXT::DFMExtAction *menuAction() const Q_DECL_OVERRIDE;
    std::list<DFMEXT::DFMExtAction *> actions() const Q_DECL_OVERRIDE;

    Q_SLOT void onActionHovered(QAction *qaction);
    Q_SLOT void onActionTriggered(QAction *qaction);

protected:
    bool interiorEntity = true;   // 默认身份文管内部创建
    QMenu *menu = nullptr;
    DFMExtMenuImpl *q = nullptr;
};

DPUTILS_END_NAMESPACE

#endif   // DFMEXTMENUIMPL_P_H
