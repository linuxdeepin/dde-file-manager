// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTACTIONIMPL_P_H
#define DFMEXTACTIONIMPL_P_H

#include "dfmplugin_utils_global.h"

#include <dfm-extension/menu/private/dfmextactionprivate.h>

#include <QObject>

class QAction;

DPUTILS_BEGIN_NAMESPACE

class DFMExtActionImpl;
class DFMExtActionImplPrivate : public QObject, public DFMEXT::DFMExtActionPrivate
{
    Q_OBJECT
public:
    DFMExtActionImplPrivate(DFMExtActionImpl *qImpl, QAction *ac = nullptr);
    virtual ~DFMExtActionImplPrivate() override;

    DFMExtActionImpl *actionImpl() const;
    QAction *qaction() const;
    bool isInterior() const;

    void setIcon(const std::string &iconName) Q_DECL_OVERRIDE;
    std::string icon() const Q_DECL_OVERRIDE;

    void setText(const std::string &text) Q_DECL_OVERRIDE;
    std::string text() const Q_DECL_OVERRIDE;

    void setToolTip(const std::string &tip) Q_DECL_OVERRIDE;
    std::string toolTip() const Q_DECL_OVERRIDE;

    void setMenu(DFMEXT::DFMExtMenu *menu) Q_DECL_OVERRIDE;
    DFMEXT::DFMExtMenu *menu() const Q_DECL_OVERRIDE;

    void setSeparator(bool b) Q_DECL_OVERRIDE;
    bool isSeparator() const Q_DECL_OVERRIDE;

    void setCheckable(bool b) Q_DECL_OVERRIDE;
    bool isCheckable() const Q_DECL_OVERRIDE;

    void setChecked(bool b) Q_DECL_OVERRIDE;
    bool isChecked() const Q_DECL_OVERRIDE;

    void setEnabled(bool b) Q_DECL_OVERRIDE;
    bool isEnabled() const Q_DECL_OVERRIDE;

private:
    void omitText(const std::string &text);
private slots:
    void onActionHovered();
    void onActionTriggered(bool checked);
    void deleteParent();

private:
    bool interiorEntity = true;   //默认身份文管内部创建
    QAction *action = nullptr;
    DFMExtActionImpl *q = nullptr;
};

DPUTILS_END_NAMESPACE

#endif   // DFMEXTACTIONIMPL_P_H
