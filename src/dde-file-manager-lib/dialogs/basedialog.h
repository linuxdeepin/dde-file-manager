// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DTitlebar;
DWIDGET_END_NAMESPACE

class QShowEvent;
class QWidget;

class BaseDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit BaseDialog(QWidget *parent = nullptr);
    ~BaseDialog();

    void setTitle(const QString &title);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    DTitlebar *m_titlebar = nullptr;
};

#endif // BASEDIALOG_H
