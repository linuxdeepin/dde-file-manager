// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizationgroup.h"
#include "config/configpresenter.h"

#include <QDebug>

static constexpr int kContentWidght = 400;
static constexpr int kContentHeight = 48;
static constexpr int kCheckEntryHeight = 36;
static constexpr int kCheckEntryWidget = 400;

using namespace ddplugin_organizer;
OrganizationGroup::OrganizationGroup(QWidget *parent)
    : QWidget(parent)
{
    contentLayout = new QVBoxLayout(this);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(1);
    contentLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(contentLayout);
}

OrganizationGroup::~OrganizationGroup()
{
    delete currentClass;
    currentClass = nullptr;
}

void OrganizationGroup::reset()
{
    bool first = false;
    // add switch
    if (!organizationSwitch) {
        organizationSwitch = new SwitchWidget(tr("Organize desktop"), this);
        organizationSwitch->hide();   // 1071: 移除此选项
        organizationSwitch->setFixedSize(kContentWidght, kContentHeight);
        contentLayout->insertWidget(0, organizationSwitch, 0, Qt::AlignTop);
        connect(organizationSwitch, &SwitchWidget::checkedChanged, this, &OrganizationGroup::checkedChanged);
        first = true;
    }

    bool on = CfgPresenter->isEnable();
    organizationSwitch->setChecked(on);

    if (on) {
        organizationSwitch->setRoundEdge(ContentBackgroundWidget::kTop);

        // add method combox
        if (!methodCombox) {
            methodCombox = new MethodComBox(tr("Organize by"), this);
            methodCombox->initCheckBox();
            methodCombox->setFixedSize(kContentWidght, kCheckEntryHeight);
            contentLayout->insertWidget(1, methodCombox, 0, Qt::AlignTop);
            // requiring widget to be visible when layout calculates sizehint.
            methodCombox->setVisible(true);
            connect(methodCombox, &MethodComBox::methodChanged, this, &OrganizationGroup::reset);
        }

        if (CfgPresenter->mode() == kNormalized) {
            auto method = CfgPresenter->classification();
            methodCombox->setCurrentMethod(method);

            if (!currentClass || method != currentClass->id()) {
                delete currentClass;
                currentClass = MethodGroupHelper::create(method);
                Q_ASSERT(currentClass);
                currentClass->build();
            }

            int pos = 2;

            QWidget *last = nullptr;
            for (QWidget *wid : currentClass->subWidgets()) {
                wid->setFixedSize(kCheckEntryWidget, kCheckEntryHeight);
                contentLayout->insertWidget(pos++, wid, 0, Qt::AlignTop);
                // requiring widget to be visible when layout calculates sizehint.
                wid->setVisible(true);
                last = wid;
            }

            if (ContentBackgroundWidget *bk = qobject_cast<ContentBackgroundWidget *>(last)) {
                methodCombox->setRoundEdge(MethodComBox::kNone);
                bk->setRoundEdge(MethodComBox::kBottom);
            } else {
                methodCombox->setRoundEdge(MethodComBox::kBottom);
            }

        } else {
            methodCombox->setCurrentMethod(-1);
            methodCombox->setRoundEdge(MethodComBox::kBottom);
        }

        if (!spacer)
            spacer = new QSpacerItem(1, 10, QSizePolicy::Fixed);
        contentLayout->addItem(spacer);
    } else {
        organizationSwitch->setRoundEdge(ContentBackgroundWidget::kBoth);

        if (methodCombox) {
            delete methodCombox;
            methodCombox = nullptr;
        }

        if (currentClass) {
            currentClass->release();
            delete currentClass;
            currentClass = nullptr;
        }
        contentLayout->removeItem(spacer);
        if (spacer) {
            delete spacer;
            spacer = nullptr;
        }
    }

    if (first)
        contentLayout->addStretch(1);

    // adjust size when widgets were added or removed.
    adjustSize();
}

void OrganizationGroup::checkedChanged(bool enable)
{
    emit CfgPresenter->changeEnableState(enable);
}
