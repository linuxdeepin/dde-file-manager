// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLOSEALLDIALOG_H
#define CLOSEALLDIALOG_H

#include <DAbstractDialog>
#include <QPushButton>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DCommandLinkButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_propertydialog {
class CloseAllDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit CloseAllDialog(QWidget *parent = nullptr);
    ~CloseAllDialog() override;

    void initUI();
    void initConnect();

signals:
    void allClosed();

public slots:
    void setTotalMessage(qint64 size, int count);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

private:
    DTK_WIDGET_NAMESPACE::DLabel *messageLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *closeButton { nullptr };
};
}
#endif   // CLOSEALLDIALOG_H
