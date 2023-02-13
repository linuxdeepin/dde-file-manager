// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIZEFORMATHELPER_H
#define SIZEFORMATHELPER_H

#include <QString>

/*!
 * \brief current only support for `DiskControlItem`!
 */
class SizeFormatHelper
{
public:
    static QString formatDiskSize(const quint64 num);
    static QString sizeString(const QString &str);
};

#endif   // SIZEFORMATHELPER_H
