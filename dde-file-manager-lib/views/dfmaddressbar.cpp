/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmaddressbar.h"

#include <QAction>

DFM_BEGIN_NAMESPACE


DFMAddressBar::DFMAddressBar(QWidget *parent)
    : QLineEdit(parent)
{
    initUI();
}

void DFMAddressBar::setCurrentUrl(const DUrl &path)
{
    QString text = path.isLocalFile() ? path.toLocalFile() : path.toString();

    currentUrl = path;
    //m_disableCompletion = true;
    this->setText(text);
    this->setSelection(0, text.length());
}

void DFMAddressBar::focusOutEvent(QFocusEvent *e)
{
    emit focusOut();

    return QLineEdit::focusOutEvent(e);
}

void DFMAddressBar::initUI()
{
    // Left indicator (clickable! did u know that?)
    actionJumpTo = new QAction(this);
    actionSearch = new QAction(this);

    // Clear text button
    setClearButtonEnabled(true);

    // Other misc..
    setFixedHeight(24);
    setObjectName("DSearchBar");
    setMinimumWidth(1);

    setFocusPolicy(Qt::ClickFocus);
}


DFM_END_NAMESPACE
