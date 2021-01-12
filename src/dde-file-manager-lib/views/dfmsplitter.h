/*
 * Copyright (C) 2017 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     liuyang <liuyang_cm@deepin.com>
 *
 * Maintainer: liuyang <liuyang_cm@deepin.com>
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

#ifndef DFMSPLITTER_H
#define DFMSPLITTER_H


#include <QSplitter>
#include <QSplitterHandle>

class DFMSplitterHandle: public QSplitterHandle
{
    Q_OBJECT

public:
    explicit DFMSplitterHandle(Qt::Orientation orientation, QSplitter* parent);

protected:
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
};

// ---------- Item Get Border Line ----------

class DFMSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit DFMSplitter(Qt::Orientation orientation, QWidget *parent = nullptr);
    void moveSplitter(int pos, int index);

protected:
    QSplitterHandle* createHandle() override;
};

#endif // DFMSPLITTER_H
