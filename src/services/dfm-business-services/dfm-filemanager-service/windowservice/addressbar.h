/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef DFMADDRESSBAR_H
#define DFMADDRESSBAR_H

#include "dfm_filemanager_service_global.h"

#include <QLineEdit>

DSB_FM_BEGIN_NAMESPACE

class DFMAddressBarPrivate;
class DFMAddressBar : public QLineEdit
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMAddressBar)
    DFMAddressBarPrivate * const d;

public:
    explicit DFMAddressBar(QWidget* parent = nullptr);
    bool completerViewVisible();

Q_SIGNALS:
    void editingFinishedUrl(const QUrl &url);
    void editingFinishedSearch(const QString &url);
};

DSB_FM_END_NAMESPACE

#endif //DFMADDRESSBAR_H
