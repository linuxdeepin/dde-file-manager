/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
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
#ifndef DFMFILEVIEWPRIVATEA_H
#define DFMFILEVIEWPRIVATEA_H

#include "widgets/dfmfileview/dfmfileview.h"

#include <QObject>
#include <QUrl>

class DFMFileViewPrivate
{
    Q_DECLARE_PUBLIC(DFMFileView)
    DFMFileView * q_ptr;

public:

    explicit DFMFileViewPrivate(DFMFileView *qq);
    int iconModeColumnCount(int itemWidth = 0) const;

    QAtomicInteger<bool> m_allowedAdjustColumnSize = true;
    DFMHeaderView* m_headview = nullptr;
    DFMListItemDelegate *m_listDelegate = nullptr;
    DFMIconItemDelegate *m_iconDelegate = nullptr;
    QUrl m_url;
};

#endif // DFMFILEVIEWPRIVATEA_H
