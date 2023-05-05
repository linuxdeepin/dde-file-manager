// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HISTORYSTACK_H
#define HISTORYSTACK_H

#include "dfmplugin_titlebar_global.h"

#include <QList>
#include <QUrl>

namespace dfmplugin_titlebar {

class HistoryStack
{
public:
    explicit HistoryStack(int threshold);
    void append(const QUrl &url);
    QUrl back();
    QUrl forward();
    void setThreshold(int threshold);
    bool isFirst();
    bool isLast();
    int size();
    void removeAt(int i);
    void removeUrl(const QUrl &url);
    int currentIndex();

    bool backIsExist();
    bool forwardIsExist();

    // TODO(zhangs): check network exists

private:
    bool needCheckExist(const QUrl &url);
    bool checkPathIsExist(const QUrl &url);

private:
    QList<QUrl> list;
    int curThreshold;
    int index;
};

}

#endif   // HISTORYSTACK_H
