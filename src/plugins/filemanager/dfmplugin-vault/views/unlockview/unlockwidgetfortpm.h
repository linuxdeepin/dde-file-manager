// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNLOCKWIDGETFORTPM_H
#define UNLOCKWIDGETFORTPM_H

#include "dfmplugin_vault_global.h"

#include <DPasswordEdit>
#include <DLabel>
#include <DPushButton>

#include <QWidget>

namespace dfmplugin_vault {
class UnlockWidgetForTpm : public QWidget
{
    Q_OBJECT
public:
    explicit UnlockWidgetForTpm(QWidget *parent = nullptr);

    QStringList btnText() const;
    QString titleText() const;
    void buttonClicked(int index, const QString &text);

public Q_SLOTS:
    void vaultUnlocked(int state);
    void pinEditFocusChanged(bool onFocus);
    void showHintInfo();
    void pinEditTextChanged(const QString &text);

protected:
    bool eventFilter(QObject *obj, QEvent *evt) override;

Q_SIGNALS:
    void signalJump(const PageType &type);
    void sigCloseDialog();
    void sigBtnEnabled(int index, bool state);
    void setAllowClose(bool value);

private:
    void initUI();
    void initConnect();

    DTK_WIDGET_NAMESPACE::DPasswordEdit *pinEdit { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DPushButton *tipsBtn { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *forgetPin { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *infoLabel { Q_NULLPTR };
};
}

#endif // UNLOCKWIDGETFORTPM_H
