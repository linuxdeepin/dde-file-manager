// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizationgroup.h"
#include "config/configpresenter.h"

#include <QDebug>
#include <QShortcut>
#include <QTimer>

static constexpr int kContentWith = 400;
static constexpr int kContentHeight = 48;
static constexpr int kCheckEntryHeight = 36;
static constexpr int kCheckEntryWidth = 400;

using namespace ddplugin_organizer;

OrganizationGroup::OrganizationGroup(QWidget *parent)
    : QWidget(parent)
{
    contentLayout = new QVBoxLayout(this);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(1);
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
        organizationSwitch->setFixedHeight(kContentHeight);
        contentLayout->insertWidget(0, organizationSwitch, 0, Qt::AlignTop);
        connect(organizationSwitch, &SwitchWidget::checkedChanged, this, &OrganizationGroup::enableOrganizeChanged);
        first = true;
    }

    bool on = CfgPresenter->isEnable();
    organizationSwitch->setChecked(on);

    if (on) {
        organizationSwitch->setRoundEdge(ContentBackgroundWidget::kTop);
        initAll();
    } else {
        organizationSwitch->setRoundEdge(ContentBackgroundWidget::kBoth);
        clearlAll();
    }

    if (first)
        contentLayout->addStretch(1);

    // adjust size when widgets were added or removed.
    adjustSize();
}

void OrganizationGroup::enableOrganizeChanged(bool enable)
{
    emit CfgPresenter->changeEnableState(enable);
}

void OrganizationGroup::enableHideAllChanged(bool enable)
{
    if (enable) {
        initShortcutWidget();
        hideAllSwitch->setRoundEdge(SwitchWidget::kTop);
        int index { contentLayout->indexOf(hideAllSwitch) };
        index = index < 0 ? 0 : index + 1;
        contentLayout->insertWidget(index, shortcutForHide);
    } else {
        clearShortcutWidget();
        hideAllSwitch->setRoundEdge(SwitchWidget::kBoth);
    }
    emit CfgPresenter->changeEnableVisibilityState(enable);
}

QLayout *OrganizationGroup::buildTypeLayout()
{
    int count { currentClass->subWidgets().size() };
    QGridLayout *gridLayout { new QGridLayout };
    gridLayout->setSpacing(1);
    if (count <= 1)
        return gridLayout;
    // 最后一个元素单独为底
    auto list { currentClass->subWidgets().mid(0, count - 1) };

    // 每 3 列为一行
    int row { 0 };
    int col { 0 };
    int index { 0 };
    for (QWidget *wid : list) {
        wid->setFixedHeight(kCheckEntryHeight);
        gridLayout->addWidget(wid, row, col, Qt::AlignTop);
        wid->setVisible(true);   // requiring widget to be visible when layout calculates sizehint.
        ++index;
        row = index / 3;
        col = index % 3;
    }

    return gridLayout;
}

void OrganizationGroup::initAll()
{
    // add method combox
    if (!methodCombox) {
        methodCombox = new MethodComBox(tr("Organize by"), this);
        methodCombox->initCheckBox();
        methodCombox->setFixedHeight(kCheckEntryHeight);
        methodCombox->setMinimumWidth(kCheckEntryWidth);
        ;
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

        contentLayout->insertLayout(pos++, buildTypeLayout());
        QWidget *last = currentClass->subWidgets().last();
        last->setFixedHeight(kCheckEntryHeight);
        last->setMinimumWidth(kCheckEntryWidth);
        contentLayout->insertWidget(pos++, last, 0, Qt::AlignTop);
        last->setVisible(true);
        if (ContentBackgroundWidget *bk = qobject_cast<ContentBackgroundWidget *>(last)) {
            methodCombox->setRoundEdge(organizationSwitch->isVisible()
                                               ? MethodComBox::kNone
                                               : MethodComBox::kTop);
            bk->setRoundEdge(MethodComBox::kBottom);
        } else {
            methodCombox->setRoundEdge(MethodComBox::kBottom);
        }

        if (!spacer1)
            spacer1 = new QSpacerItem(1, 10, QSizePolicy::Fixed);
        contentLayout->insertItem(pos++, spacer1);
        if (!hideAllSwitch) {
            hideAllSwitch = new SwitchWidget(tr("Hide all collections with one click"), this);
            hideAllSwitch->setFixedHeight(kContentHeight);
            hideAllSwitch->setMinimumWidth(kContentWith);
            hideAllSwitch->setChecked(CfgPresenter->isEnableVisibility());
            hideAllSwitch->setRoundEdge(CfgPresenter->isEnableVisibility()
                                                ? SwitchWidget::kTop
                                                : SwitchWidget::kBoth);
            contentLayout->insertWidget(pos++, hideAllSwitch, 0, Qt::AlignTop);
            connect(hideAllSwitch, &SwitchWidget::checkedChanged, this, &OrganizationGroup::enableHideAllChanged);
        }

        if (hideAllSwitch && CfgPresenter->isEnableVisibility()) {
            initShortcutWidget();
            contentLayout->insertWidget(pos++, shortcutForHide, 0, Qt::AlignTop);
        }

    } else {
        methodCombox->setCurrentMethod(-1);
        methodCombox->setRoundEdge(MethodComBox::kBottom);
    }

    if (!spacer2)
        spacer2 = new QSpacerItem(1, 10, QSizePolicy::Fixed);
    contentLayout->addItem(spacer2);
}

void OrganizationGroup::clearlAll()
{
    if (methodCombox) {
        delete methodCombox;
        methodCombox = nullptr;
    }

    if (hideAllSwitch) {
        delete hideAllSwitch;
        hideAllSwitch = nullptr;
    }

    clearShortcutWidget();

    if (currentClass) {
        currentClass->release();
        delete currentClass;
        currentClass = nullptr;
    }
    contentLayout->removeItem(spacer1);
    if (spacer1) {
        delete spacer1;
        spacer1 = nullptr;
    }
    contentLayout->removeItem(spacer2);
    if (spacer2) {
        delete spacer2;
        spacer2 = nullptr;
    }
}

void OrganizationGroup::initShortcutWidget()
{
    if (!shortcutForHide) {
        shortcutForHide = new ShortcutWidget(tr("Hide/Show Collection Shortcuts"), this);
        const auto &oldSeq { CfgPresenter->hideAllKeySequence() };
        shortcutForHide->setKeySequence(oldSeq);
        shortcutForHide->setRoundEdge(SwitchWidget::kBottom);

        connect(shortcutForHide, &ShortcutWidget::keySequenceChanged, this, [](const QKeySequence &seq) {
            fmInfo() << "collections key sequence changed for hide all:" << seq.toString();
            emit CfgPresenter->changeHideAllKeySequence(seq);
        });
        connect(shortcutForHide, &ShortcutWidget::keySequenceUpdateFailed, this, [oldSeq, this](const QKeySequence &seq) {
            fmWarning() << "custom hide all collections shortcut failed:" << seq.toString();
            // 延迟的作用是为了让用户看到一个错误的输入被修改的过程
            QTimer::singleShot(200, this, [this, oldSeq]() {
                shortcutForHide->setKeySequence(oldSeq);
            });
        });
    }
}

void OrganizationGroup::clearShortcutWidget()
{
    if (shortcutForHide) {
        shortcutForHide->hide();
        delete shortcutForHide;
        shortcutForHide = nullptr;
    }
}
