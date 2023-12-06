// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREMOVEBYTPMPINWIDGET_H
#define VAULTREMOVEBYTPMPINWIDGET_H

#include "dfmplugin_vault_global.h"

#include <polkit-qt5-1/PolkitQt1/Authority>

#include <DPasswordEdit>

#include <QWidget>
#include <DPushButton>

namespace dfmplugin_vault {

class VaultRemoveByTpmPinWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VaultRemoveByTpmPinWidget(QWidget *parent = nullptr);

    QStringList btnText() const;
    QString titleText() const;
    void buttonClicked(int index, const QString &text);

public Q_SLOTS:
    void showHintInfo();
    void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);

Q_SIGNALS:
    void jumpPage(const RemoveWidgetType &type);
    void closeDialog();

private:
    void initUI();
    void initConnect();

    DTK_WIDGET_NAMESPACE::DPasswordEdit *pinEdit { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DPushButton *tipsBtn { Q_NULLPTR };
};

}

#endif // VAULTREMOVEBYTPMPINWIDGET_H
