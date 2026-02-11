// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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
    : DDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    installEventFilter(this);
}

void AlertHideAllDialog::initialize()
{
    // setStyleSheet("border: 1px solid red");
    setFixedWidth(420);
    setSpacing(0);
    setContentLayoutContentsMargins(QMargins(0, 0, 0, 0));
    setWordWrapMessage(true);
    setWordWrapTitle(true);

    const auto &keySeq { CfgPresenter->hideAllKeySequence().toString() };
    const QString &content { tr("The hortcut key \"%1\" to show collection").arg(keySeq) };

    setTitle(content);
    setMessage("To disable the One-Click Hide feature, "
               "turn off the One-Click Hide Collection by "
               "invoking the View Options window in the desktop context menu.");
    setIcon(QIcon::fromTheme("deepin-toggle-desktop"));

    DCheckBox *checkBox { new DCheckBox(tr("No prompt")) };
    connect(checkBox, &DCheckBox::stateChanged, this, [this](int state) {
        if (state == Qt::CheckState::Checked)
            repeatNoMore = true;
        else
            repeatNoMore = false;
    });

    addSpacing(20);
    addContent(checkBox, Qt::AlignHCenter);
    addButton(QObject::tr("Confirm", "button"), true);
    connect(this, &AlertHideAllDialog::buttonClicked, this, [this](int index, const QString &) {
        btnIndex = index;
        accept();
    });

    adjustSize();
}

bool AlertHideAllDialog::isRepeatNoMore() const
{
    return repeatNoMore;
}

int AlertHideAllDialog::confirmBtnIndex() const
{
    return btnIndex;
}

bool AlertHideAllDialog::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FontChange || e->type() == QEvent::Show) {
        QLabel *label = qobject_cast<QLabel *>(o);

        if (label && !label->text().isEmpty() && label->wordWrap()) {
            QSize sz = style()->itemTextRect(label->fontMetrics(), label->rect(), Qt::TextWordWrap, false, label->text()).size();

            label->setMinimumHeight(qMax(sz.height(), label->sizeHint().height()));
        }
        adjustSize();
        return true;
    }
    return DDialog::eventFilter(o, e);
}

}   // namespace ddplugin_organizer
