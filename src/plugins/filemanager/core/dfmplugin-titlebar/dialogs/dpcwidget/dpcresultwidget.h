// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPCRESULTWIDGET_H
#define DPCRESULTWIDGET_H

#include "dfmplugin_titlebar_global.h"

#include <DWidget>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE

namespace dfmplugin_titlebar {

class DPCResultWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit DPCResultWidget(QWidget *parent = nullptr);
    void setResult(bool success, const QString &msg);

Q_SIGNALS:
    void sigCloseDialog();

private:
    void initUI();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DLabel *resultIcon { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *msgLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *closeBtn { nullptr };
};

}

#endif   // DPCRESULTWIDGET_H
