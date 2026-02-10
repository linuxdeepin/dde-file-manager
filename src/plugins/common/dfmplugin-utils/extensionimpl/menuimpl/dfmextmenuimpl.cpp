// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmextmenuimpl.h"
#include "private/dfmextmenuimpl_p.h"
#include "private/dfmextactionimpl_p.h"
#include "dfmextactionimpl.h"
#include "dfmextmendefine.h"
#include "dfmextmenucache.h"

#include <QMetaObject>
#include <QFile>
#include <QMenu>

#include <iostream>

USING_DFMEXT_NAMESPACE
DPUTILS_USE_NAMESPACE

DFMExtMenuImpl::DFMExtMenuImpl(QMenu *menu)
    : DFMExtMenu(new DFMExtMenuImplPrivate(this, menu))
{
}

DFMExtMenuImplPrivate::DFMExtMenuImplPrivate(DFMExtMenuImpl *qImpl, QMenu *m)
    : DFMExtMenuPrivate(), interiorEntity(m != nullptr), menu(m ? m : new QMenu()), q(qImpl)
{
    Q_ASSERT(q);
    // 用于限制操作外部传入的QMenu
    // 设置当前壳入口
    DFMExt_Install_MenuPrivate(menu, this);
    // 信号 转发到bind function
    QObject::connect(menu, &QMenu::hovered,
                     this, &DFMExtMenuImplPrivate::onActionHovered);
    QObject::connect(menu, &QMenu::triggered,
                     this, &DFMExtMenuImplPrivate::onActionTriggered);
    // 应用于actions获取implmenu时自动释放，将匿名函数挂载到menu以屏蔽循环的释放冲突
    QObject::connect(menu, &QObject::destroyed, menu, [=]() {
        q->deleted(q);
        if (q)
            delete q;
    });
}

DFMExtMenuImplPrivate::~DFMExtMenuImplPrivate()
{
    q = nullptr;
}

DFMExtMenuImpl *DFMExtMenuImplPrivate::menuImpl() const
{
    return q;
}

QMenu *DFMExtMenuImplPrivate::qmenu() const
{
    return menu;
}

bool DFMExtMenuImplPrivate::isInterior() const
{
    return interiorEntity;
}

std::string DFMExtMenuImplPrivate::title() const
{
    if (menu)
        return menu->title().toStdString();

    return "";
}

void DFMExtMenuImplPrivate::setTitle(const std::string &title)
{
    if (interiorEntity)
        return;
    if (menu)
        menu->setTitle(QString::fromStdString(title));
}

std::string DFMExtMenuImplPrivate::icon() const
{
    std::string name;
    if (menu)
        name = menu->icon().name().toStdString();

    return name;
}

void DFMExtMenuImplPrivate::setIcon(const std::string &iconName)
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

    if (menu)
        menu->setIcon(icon);
}

bool DFMExtMenuImplPrivate::addAction(DFMExtAction *action)
{
    DFMExtActionImpl *impl = static_cast<DFMExtActionImpl *>(action);
    if (menu && impl) {
        DFMExtActionImplPrivate *actionImpl_d = dynamic_cast<DFMExtActionImplPrivate *>(impl->d);

        // 文管内部创建的不允许添加
        if (actionImpl_d->isInterior())
            return false;

        QAction *ac = actionImpl_d->qaction();
        ac->setParent(menu);
        menu->addAction(ac);
        return true;
    }
    return false;
}

bool DFMExtMenuImplPrivate::insertAction(DFMExtAction *before, DFMExtAction *action)
{
    DFMExtActionImpl *beforeImpl = static_cast<DFMExtActionImpl *>(before);
    DFMExtActionImpl *impl = static_cast<DFMExtActionImpl *>(action);
    DFMExtActionImplPrivate *beforeImpl_d = dynamic_cast<DFMExtActionImplPrivate *>(beforeImpl->d);
    if (impl == nullptr || beforeImpl_d == nullptr)
        return false;

    DFMExtActionImplPrivate *impl_d = dynamic_cast<DFMExtActionImplPrivate *>(impl->d);
    if (menu != nullptr && impl_d != nullptr) {
        QAction *beforeAc = beforeImpl_d->qaction();

        // 文管内部创建的不允许添加
        if (impl_d->isInterior())
            return false;

        QAction *ac = impl_d->qaction();
        ac->setParent(menu);
        menu->insertAction(beforeAc, ac);

        // Record sorting rules
        QPair<QAction *, QAction *> pair(beforeAc, ac);
        QList<QPair<QAction *, QAction *>> &rules = DFMExtMenuCache::instance().extMenuSortRules;
        if (!rules.contains(pair))
            rules.push_back(pair);

        return true;
    }

    return false;
}

DFMExtAction *DFMExtMenuImplPrivate::menuAction() const
{
    if (menu) {
        auto qaction = menu->menuAction();
        if (qaction) {
            DFMExtActionImplPrivate *impl_d = DFMExt_Get_ActionPrivate(qaction);
            //文管自身的创建的菜单，为其创建warpper
            if (impl_d == nullptr) {
                auto bindAciton = new DFMExtActionImpl(qaction);
                impl_d = dynamic_cast<DFMExtActionImplPrivate *>(bindAciton->d);
            }
            return impl_d->actionImpl();
        }
    }
    return nullptr;
}

std::list<DFMEXT::DFMExtAction *> DFMExtMenuImplPrivate::actions() const
{
    std::list<DFMEXT::DFMExtAction *> ret;
    if (menu) {
        auto orgAictons = menu->actions();
        for (QAction *qaction : orgAictons) {
            DFMExtActionImplPrivate *impl_d = DFMExt_Get_ActionPrivate(qaction);
            DFMExtActionImpl *impl = nullptr;
            //文管自身的创建的菜单，为其创建warpper
            if (impl_d == nullptr)
                impl = new DFMExtActionImpl(qaction);
            else
                impl = impl_d->actionImpl();
            ret.push_back(impl);
        }
    }

    return ret;
}

void DFMExtMenuImplPrivate::onActionHovered(QAction *qaction)
{
    DFMExtActionImplPrivate *ext = DFMExt_Get_ActionPrivate(qaction);
    if (ext)
        q->hovered(ext->actionImpl());
}

void DFMExtMenuImplPrivate::onActionTriggered(QAction *qaction)
{
    DFMExtActionImplPrivate *ext = DFMExt_Get_ActionPrivate(qaction);
    if (ext)
        q->triggered(ext->actionImpl());
}
