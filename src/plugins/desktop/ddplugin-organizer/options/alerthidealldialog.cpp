// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "alerthidealldialog.h"
#include "config/configpresenter.h"

#include <DLabel>
#include <DCheckBox>
#include <DPushButton>
#include <DTitlebar>

#include <QVBoxLayout>
#include <QSizePolicy>

namespace ddplugin_organizer {

DWIDGET_USE_NAMESPACE

AlertHideAllDialog::AlertHideAllDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
}

void AlertHideAllDialog::initialize()
{
    // setStyleSheet("border: 1px solid red");
    // TODO: update UI
    setFixedSize(500, 250);
    const auto &keySeq { CfgPresenter->hideAllKeySequence().toString() };
    const QString &content { tr("You can use the shortcut key \"%1\" to display all collections.").arg(keySeq) };

    auto titleBar = new DTitlebar(this);
    titleBar->setMenuVisible(false);
    titleBar->setBackgroundTransparent(true);

    QVBoxLayout *mainLayout { new QVBoxLayout() };
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QVBoxLayout *contentLayout { new QVBoxLayout() };
    contentLayout->setContentsMargins(10, 5, 10, 5);
    contentLayout->setSpacing(0);

    DLabel *titleLabel { new DLabel(tr("How do I retrieve the collection?")) };
    DLabel *contentLabel { new DLabel(content) };
    DLabel *tipLabel { new DLabel(tr("If you want to disable the One-Click Hide feature, you can turn off the One-Click"
                                     " Hide Collection by invoking the \"View Options\" window in the desktop context menu.")) };

    tipLabel->setWordWrap(true);

    DCheckBox *checkBox { new DCheckBox(tr("Do not repeat this message")) };
    connect(checkBox, &DCheckBox::stateChanged, this, [this](int state) {
        if (state == Qt::CheckState::Checked)
            repeatNoMore = true;
        else
            repeatNoMore = false;
    });

    DPushButton *button { new DPushButton(tr("Confirm", "button")) };
    button->setFixedWidth(80);
    connect(button, &DPushButton::clicked, this, [this]() {
        accept();
    });

    contentLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    contentLayout->addWidget(contentLabel);
    contentLayout->addWidget(tipLabel);
    contentLayout->addWidget(checkBox);
    contentLayout->addWidget(button, 0, Qt::AlignHCenter);

    mainLayout->addWidget(titleBar, 0, Qt::AlignTop);
    mainLayout->addLayout(contentLayout, 1);

    setLayout(mainLayout);
    adjustSize();
}

bool AlertHideAllDialog::isRepeatNoMore() const
{
    return repeatNoMore;
}

}   // namespace ddplugin_organizer
