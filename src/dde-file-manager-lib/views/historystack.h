// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
