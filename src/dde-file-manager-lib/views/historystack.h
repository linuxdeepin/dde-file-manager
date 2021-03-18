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

#ifndef HISTORYSTACK_H
#define HISTORYSTACK_H

#include <QList>

#include "durl.h"

class HistoryStack
{
public:
    explicit HistoryStack(int threshold);
    void append(DUrl url);
    DUrl back();
    DUrl forward();
    void setThreshold(int threshold);
    bool isFirst();
    bool isLast();
    int size();
    void removeAt(int i);
    int currentIndex();

    /**
     * @brief backIsExist back目录是否存在
     * 该函数只测试计算机中是否包含back目录
     * @return
     */
    bool backIsExist();
    /**
     * @brief forwardIsExist forward目录是否存在
     * 该函数只测试计算机中是否包含forward目录
     * @return
     */
    bool forwardIsExist();


private:
    QList<DUrl> m_list;
    int m_threshold;
    int m_index;

    friend QDebug operator<<(QDebug beg, const HistoryStack &stack);

    bool needCheckExist(const DUrl &url);
    bool checkPathIsExist(const DUrl &url);
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug beg, const HistoryStack &stack);
QT_END_NAMESPACE

#endif // HISTORYSTACK_H
