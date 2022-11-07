/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DPCCONFIRMWIDGET_H
#define DPCCONFIRMWIDGET_H

#include "dfmplugin_titlebar_global.h"

#include <DWidget>

class QDBusInterface;

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DFloatingWidget;
class DToolTip;
class DLabel;
class DSuggestButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_titlebar {

class DPCConfirmWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit DPCConfirmWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void sigCloseDialog();
    void sigConfirmed();

public Q_SLOTS:
    void checkPasswordLength(const QString &pwd);
    void onSaveBtnClicked();
    void onPasswordChecked(int result);

private:
    void initUI();
    void initConnect();
    void showToolTips(const QString &msg, QWidget *w);
    bool checkRepeatPassword();
    bool checkNewPassword();
    void setEnabled(bool enabled);

private:
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *oldPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *newPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *repeatPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *toolTipFrame { nullptr };
    DTK_WIDGET_NAMESPACE::DToolTip *toolTip { nullptr };
    DTK_WIDGET_NAMESPACE::DSuggestButton *saveBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *cancelBtn { nullptr };
    QWidget *parentWidget { nullptr };
    QSharedPointer<QDBusInterface> accessControlInter { nullptr };
};

}

#endif   // DPCCONFIRMWIDGET_H
