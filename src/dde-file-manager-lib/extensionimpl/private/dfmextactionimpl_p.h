/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             huangyu<zhangyub@uniontech.com>
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
#ifndef DFMEXTACTIONIMPL_P_H
#define DFMEXTACTIONIMPL_P_H

#include <menu/private/dfmextactionprivate.h>

#include <QObject>

class QAction;
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
    bool interiorEntity = true; //默认身份文管内部创建
    QAction *action = nullptr;
    DFMExtActionImpl *q = nullptr;
};

#endif // DFMEXTACTIONIMPL_P_H
