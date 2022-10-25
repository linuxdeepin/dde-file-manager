// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLOSEALLDIALOGINDICATOR_H
#define CLOSEALLDIALOGINDICATOR_H

#include <QLabel>
#include <QPushButton>

#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class CloseAllDialogIndicator : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit CloseAllDialogIndicator(QWidget *parent = nullptr);
    ~CloseAllDialogIndicator() override;

    void initUI();
    void initConnect();

signals:
    void allClosed();

public slots:
    void setTotalMessage(qint64 size, int count);

protected:
//    void showEvent(QShowEvent* event);
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QLabel *m_messageLabel;
    QPushButton *m_closeButton;
};

#endif // CLOSEALLDIALOGINDICATOR_H
