// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREMOVEBYRECOVERYKEYVIEW_H
#define VAULTREMOVEBYRECOVERYKEYVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <QWidget>

class QPlainTextEdit;

DWIDGET_BEGIN_NAMESPACE
class DToolTip;
class DFloatingWidget;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class VaultRemoveByRecoverykeyView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultRemoveByRecoverykeyView(QWidget *parent = nullptr);
    ~VaultRemoveByRecoverykeyView() override;

    /*!
    * /brief    获取凭证
    */
    QString getRecoverykey();

    /*!
    * /brief    清空凭证
    */
    void clear();

    void showAlertMessage(const QString &text, int duration = 3000);
public slots:
    void onRecoveryKeyChanged();

private:
    //! 输入凭证后，对凭证添加“-”
    int afterRecoveryKeyChanged(QString &str);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPlainTextEdit *keyEdit { nullptr };

    DTK_WIDGET_NAMESPACE::DToolTip *tooltip { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *floatWidget { nullptr };
};
}
#endif   // VAULTREMOVEBYRECOVERYKEYVIEW_H
