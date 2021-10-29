/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "dialogservice.h"

DSC_USE_NAMESPACE

DDialog *DialogService::showQueryScanningDialog(const QString &title)
{
    DDialog *d = new DDialog;
    d->setTitle(title);
    d->setAttribute(Qt::WA_DeleteOnClose);
    Qt::WindowFlags flags = d->windowFlags();
    d->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d->setIcon(QIcon::fromTheme("dialog-warning"));
    d->addButton(QObject::tr("Cancel","button"));
    d->addButton(QObject::tr("Stop","button"), true, DDialog::ButtonWarning); // 终止
    d->setMaximumWidth(640);
    d->show();
    return d;
}

void DialogService::showErrorDialog(const QString &title, const QString &message)
{
    DDialog d(title, message);
    Qt::WindowFlags flags = d.windowFlags();
    // dialog show top
    d.setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d.setIcon(QIcon::fromTheme("dialog-error"));
    d.addButton(tr("Confirm","button"), true, DDialog::ButtonRecommend);
    d.setMaximumWidth(640);
    d.exec();
}

DialogService::DialogService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<DialogService>()
{

}

dfm_service_common::DialogService::~DialogService()
{

}
