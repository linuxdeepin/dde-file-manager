// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmextactionimpl.h"
#include "private/dfmextactionimpl_p.h"
#include "private/dfmextmenuimpl_p.h"
#include "dfmextmendefine.h"
#include "dfmextmenuimpl.h"

#include <QAction>
#include <QMenu>
#include <QFile>
#include <QIcon>

#include <iostream>

USING_DFMEXT_NAMESPACE
DPUTILS_USE_NAMESPACE

DFMExtActionImpl::DFMExtActionImpl(QAction *action)
    : DFMExtAction(new DFMExtActionImplPrivate(this, action))
{
}

DFMExtActionImplPrivate::DFMExtActionImplPrivate(DFMExtActionImpl *qImpl, QAction *ac)
    : DFMExtActionPrivate(), interiorEntity(ac != nullptr)   // action存在实体则表明文管内部导出impl对象
      ,
      action(ac ? ac : new QAction()),
      q(qImpl)
{
    Q_ASSERT(q);
    // 设置当前壳入口
    DFMExt_Install_ActionPrivate(action, this);
    // 信号
    connect(action, &QAction::hovered, this, &DFMExtActionImplPrivate::onActionHovered);
    connect(action, &QAction::triggered, this, &DFMExtActionImplPrivate::onActionTriggered);
    // 应用于接口actions批量获取时，随qAction释放，将匿名函数挂载到action对象上，避免循环的释放冲突
    connect(action, &QAction::destroyed, this, [=]() {
        if (action->menu())
            action->menu()->deleteLater();
        q->deleted(q);
        if (q)
            delete q;
    });
}

DFMExtActionImplPrivate::~DFMExtActionImplPrivate()
{
    q = nullptr;
    fmDebug() << "release extend action" << action->text();
}

DFMExtActionImpl *DFMExtActionImplPrivate::actionImpl() const
{
    return q;
}

QAction *DFMExtActionImplPrivate::qaction() const
{
    return action;
}

bool DFMExtActionImplPrivate::isInterior() const
{
    return interiorEntity;
}

void DFMExtActionImplPrivate::setIcon(const std::string &iconName)
{
    if (interiorEntity)
        return;

    auto strIcon = QString::fromStdString(iconName);
    QIcon icon;
    if (!strIcon.isEmpty()) {
        if (QFile::exists(strIcon)) {
            icon = QIcon(strIcon);
        } else {
            icon = QIcon::fromTheme(strIcon);
        }
    }

    if (action)
        action->setIcon(icon);
}

std::string DFMExtActionImplPrivate::icon() const
{
    std::string name;
    if (action)
        name = action->icon().name().toStdString();
    return name;
}

void DFMExtActionImplPrivate::setText(const std::string &text)
{
    if (interiorEntity)
        return;

    if (action)
        omitText(text);
}

std::string DFMExtActionImplPrivate::text() const
{
    if (action)
        return action->text().toStdString();

    return "";
}

void DFMExtActionImplPrivate::setToolTip(const std::string &tip)
{
    if (interiorEntity)
        return;

    if (action)
        action->setToolTip(QString::fromStdString(tip));
}

std::string DFMExtActionImplPrivate::toolTip() const
{
    if (action)
        return action->toolTip().toStdString();

    return "";
}

void DFMExtActionImplPrivate::setMenu(DFMExtMenu *menu)
{
    if (interiorEntity)
        return;

    DFMExtMenuImpl *menuImpl = static_cast<DFMExtMenuImpl *>(menu);
    if (action && menuImpl) {
        auto menuImpl_d = dynamic_cast<DFMExtMenuImplPrivate *>(menuImpl->d);

        // 文管内部的创建的不能设置
        if (menuImpl_d->isInterior())
            return;
        QMenu *menu = menuImpl_d->qmenu();
        action->setMenu(menu);
    }
}

DFMExtMenu *DFMExtActionImplPrivate::menu() const
{
    DFMExtMenu *ret = nullptr;
    auto menu = action->menu();
    if (menu) {
        DFMExtMenuImplPrivate *impl = DFMExt_Get_MenuPrivate(menu);
        if (impl == nullptr) {
            // 文管自身的创建的菜单，为其创建warpper
            DFMExtMenuImpl *bindMenu = new DFMExtMenuImpl(menu);
            impl = dynamic_cast<DFMExtMenuImplPrivate *>(bindMenu->d);
        }
        ret = impl->menuImpl();
    }

    return ret;
}

void DFMExtActionImplPrivate::setSeparator(bool b)
{
    if (interiorEntity)
        return;

    if (action)
        action->setSeparator(b);
}

bool DFMExtActionImplPrivate::isSeparator() const
{
    if (action)
        return action->isSeparator();

    return false;
}

void DFMExtActionImplPrivate::setCheckable(bool b)
{
    if (interiorEntity)
        return;

    if (action)
        action->setCheckable(b);
}

bool DFMExtActionImplPrivate::isCheckable() const
{
    if (action)
        return action->isCheckable();

    return false;
}

void DFMExtActionImplPrivate::setChecked(bool b)
{
    if (interiorEntity)
        return;

    if (action)
        action->setChecked(b);
}

bool DFMExtActionImplPrivate::isChecked() const
{
    if (action)
        action->isChecked();

    return false;
}

void DFMExtActionImplPrivate::setEnabled(bool b)
{
    if (interiorEntity)
        return;

    if (action)
        action->setEnabled(b);
}

bool DFMExtActionImplPrivate::isEnabled() const
{
    if (action)
        return action->isEnabled();

    return false;
}

void DFMExtActionImplPrivate::omitText(const std::string &text)
{
    auto tempText = QString::fromStdString(text).toLocal8Bit();
    QFontMetrics fm(action->font());
    // 需求固定宽度与现有宽度限制保持一致
    const QString &&elidedName = fm.elidedText(tempText, Qt::ElideMiddle, 150);
    action->setText(elidedName);
    if (elidedName != tempText)
        action->setToolTip(tempText);
}

void DFMExtActionImplPrivate::onActionHovered()
{
    q->hovered(q);
}

void DFMExtActionImplPrivate::onActionTriggered(bool checked)
{
    q->triggered(q, checked);
}

void DFMExtActionImplPrivate::deleteParent()
{
    delete q;
    q = nullptr;
}
