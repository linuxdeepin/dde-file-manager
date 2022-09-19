// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHANGERESULTWIDGET_H
#define CHANGERESULTWIDGET_H

#include <DWidget>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE

class ChangeResultWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit ChangeResultWidget(QWidget *parent = nullptr);

    void setResult(bool success, const QString &msg);

Q_SIGNALS:
    void sigClosed();

private:
    void initUI();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DLabel *m_resultIcon { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *m_msgLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *m_closeBtn { nullptr };
};

#endif   // CHANGERESULTWIDGET_H
