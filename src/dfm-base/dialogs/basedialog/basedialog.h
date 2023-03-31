// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <dfm-base/dfm_base_global.h>

#include <ddialog.h>

DWIDGET_BEGIN_NAMESPACE
class DTitlebar;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QShowEvent;
class QWidget;
QT_END_NAMESPACE

namespace dfmbase {
class BaseDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit BaseDialog(QWidget *parent = nullptr);
    ~BaseDialog();

    void setTitle(const QString &title);

    void setTitleFont(const QFont &font);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    DTK_WIDGET_NAMESPACE::DTitlebar *titlebar { nullptr };
};

}

#endif   // BASEDIALOG_H
