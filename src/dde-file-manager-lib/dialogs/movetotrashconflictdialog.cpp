/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "dfmglobal.h"

#include <QLabel>
#include <QFont>
#include <QWindow>

MoveToTrashConflictDialog::MoveToTrashConflictDialog(QWidget *parent, const DUrlList &urls) :
    DDialog(parent)
{

    if(DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    QString title;
    QString tip;

    if(urls.size() == 1){
        title = tr("This file is too big for the trash");
        tip = tr("Are you sure you want to permanently delete %1?").arg(urls.first().fileName());
    }
    else{
        title = tr("The files are too big for the trash");
        tip = tr("Are you sure you want to permanently delete %1 files?").arg(QString::number(urls.size()));
    }

    setIcon(QIcon::fromTheme("dialog-warning"));
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

    addButton(tr("Cancel","button"), false);
    addButton(tr("Delete","button"), true, DDialog::ButtonWarning);
}
