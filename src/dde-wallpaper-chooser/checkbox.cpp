// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkbox.h"

#include <QKeyEvent>

CheckBox::CheckBox(QWidget *parent)
    : QCheckBox (parent)
{

}

CheckBox::CheckBox(const QString &text, QWidget *parent)
    : QCheckBox (parent)
{
    this->setText(text);
}

void CheckBox::keyPressEvent(QKeyEvent *event)
{
    // 不要加event->key() == Qt::Key_Space，QCheckBox默认就是空格勾选
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return /*|| event->key() == Qt::Key_Space*/){
        if (this->isChecked()){
            this->setChecked(false);
            emit clicked(false);
        }else {
            this->setChecked(true);
            emit clicked(true);
        }
    }

    QCheckBox::keyPressEvent(event);
}
