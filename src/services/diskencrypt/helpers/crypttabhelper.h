// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CRYPTTABHELPER_H
#define CRYPTTABHELPER_H

#include "diskencrypt_global.h"

FILE_ENCRYPT_BEGIN_NS

namespace crypttab_helper {
struct CryptItem
{
    QString target;
    QString source;
    QString keyFile;
    QStringList options;

    bool operator==(const CryptItem &other) const
    {
        return target == other.target && source == other.source && keyFile == other.keyFile && options == other.options;
    }
};

QList<CryptItem> cryptItems(const QString &crypttabFile = QString());
void saveCryptItems(const QList<CryptItem> &items, bool doUpdateInitramfs = true);

bool addCryptOption(const QString &activeName, const QString &opt);
bool removeCryptItem(const QString &activeName);
bool insertCryptItem(const CryptItem &item);
bool updateCryptTab();
bool mergeCryptTab();

void updateInitramfs();
}   // namespace crypttab_helper

FILE_ENCRYPT_END_NS

#endif   // CRYPTTABHELPER_H
