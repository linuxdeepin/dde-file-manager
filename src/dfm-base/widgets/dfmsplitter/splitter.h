/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef Splitter_H
#define Splitter_H

#include "dfm-base/dfm_base_global.h"

#include <QSplitter>
#include <QSplitterHandle>

namespace dfmbase {
class SplitterHandle: public QSplitterHandle
{
    Q_OBJECT
public:
    explicit SplitterHandle(Qt::Orientation orientation, QSplitter* parent);

protected:
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
};

// ---------- Item Get Border Line ----------

class Splitter : public QSplitter
{
    Q_OBJECT
public:
    explicit Splitter(Qt::Orientation orientation, QWidget *parent = nullptr);
    void moveSplitter(int pos, int index);

protected:
    QSplitterHandle* createHandle() override;
};
}

#endif // Splitter_H
