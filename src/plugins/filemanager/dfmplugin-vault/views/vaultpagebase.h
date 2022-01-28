/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef VAULTPAGEBASE_H
#define VAULTPAGEBASE_H

#include "dfmplugin_vault_global.h"

#include <DDialog>
DPVAULT_BEGIN_NAMESPACE
class VaultPageBase : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit VaultPageBase(QWidget *parent = nullptr);

protected:
    QWidget *wndptr { nullptr };   //! current window pointer.
};
DPVAULT_END_NAMESPACE
#endif   //VAULTPAGEBASE_H
