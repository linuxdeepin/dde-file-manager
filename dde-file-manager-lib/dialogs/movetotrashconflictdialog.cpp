/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "movetotrashconflictdialog.h"
#include <QLabel>
#include <QFont>

MoveToTrashConflictDialog::MoveToTrashConflictDialog(QWidget *parent, const DUrlList &urls) :
    DDialog(parent)
{

    QString title;
    QString tip;

    if(urls.size() == 1){
        title = tr("This file is too large to put into trash");
        tip = tr("Are you sure you want to permanently delete %1?").arg(urls.first().fileName());
    }
    else{
        title = tr("Files are too large to put into trash");
        tip = tr("Are you sure you want to permanently delete %1 files?").arg(QString::number(urls.size()));
    }

    QIcon dialogWarningIcon;
    dialogWarningIcon.addFile(":/images/dialogs/images/dialog_warning.png");
    dialogWarningIcon.addFile(":/images/dialogs/images/dialog_warning@2x.png");

    setIcon(dialogWarningIcon);
    setTitle(title);

    QLabel* label = new QLabel(this);
//    label->setWordWrap(true);

    QFont font;
    font.setPixelSize(12);
    label->setFont(font);

    QFontMetrics fm(font);
    QString elidedTipStr = fm.elidedText(tip,Qt::ElideRight,240);
    elidedTipStr += "?";
    label->setText(elidedTipStr);

    addContent(label);

    addButton(tr("Cancel"), false);
    addButton(tr("Delete"), true, DDialog::ButtonWarning);
}
