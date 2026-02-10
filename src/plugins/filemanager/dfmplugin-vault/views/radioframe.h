// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RADIOFRAME_H
#define RADIOFRAME_H

#include <dtkwidget_global.h>

#include <QFrame>

namespace dfmplugin_vault {
class RadioFrame : public QFrame
{
    Q_OBJECT
public:
    explicit RadioFrame(QFrame *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};
}

#endif
