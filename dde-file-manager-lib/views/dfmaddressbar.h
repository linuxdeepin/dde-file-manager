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
#ifndef DFMADDRESSBAR_H
#define DFMADDRESSBAR_H

#include "dfmglobal.h"

//#include <QAction>
#include <QLineEdit>

DFM_BEGIN_NAMESPACE

class DFMAddressBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit DFMAddressBar(QWidget *parent = 0);

    void setCurrentUrl(const DUrl &path);

signals:
    void focusOut();

protected:
    void focusOutEvent(QFocusEvent *e) override;

private:
    void initUI();

    bool isSearchStarted = false;
    DUrl currentUrl = DUrl();
    QAction * actionSearch = nullptr;
    QAction * actionJumpTo = nullptr;
};

DFM_END_NAMESPACE

#endif // DFMADDRESSBAR_H
