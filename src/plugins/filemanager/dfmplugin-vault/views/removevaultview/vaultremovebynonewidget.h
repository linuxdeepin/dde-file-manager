// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREMOVEBYNONEWIDGET_H
#define VAULTREMOVEBYNONEWIDGET_H

#include "dfmplugin_vault_global.h"

#include <QWidget>

namespace dfmplugin_vault {

class VaultRemoveByNoneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VaultRemoveByNoneWidget(QWidget *parent = nullptr);

    QStringList btnText() const;
    QString titleText() const;
    void buttonClicked(int index, const QString &text);

Q_SIGNALS:
    void jumpPage(const RemoveWidgetType &type);
    void closeDialog();

public Q_SLOTS:
    void slotCheckAuthorizationFinished(bool result);

private:
    void initUI();
};

}

#endif // VAULTREMOVEBYNONEWIDGET_H
