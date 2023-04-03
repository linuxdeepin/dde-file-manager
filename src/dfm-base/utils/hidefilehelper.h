// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIDEFILEHELPER_H
#define HIDEFILEHELPER_H

#include <dfm-base/dfm_base_global.h>

#include <QUrl>
#include <QScopedPointer>

namespace dfmbase {
class HideFileHelperPrivate;
class HideFileHelper
{
public:
    explicit HideFileHelper(const QUrl &dir);
    ~HideFileHelper();

    QUrl dirUrl() const;
    QUrl fileUrl() const;
    bool save() const;
    bool insert(const QString &name);
    bool remove(const QString &name);
    bool contains(const QString &name);
    QSet<QString> hideFileList() const;

private:
    QScopedPointer<HideFileHelperPrivate> d;
};
}

#endif   // HIDEFILEHELPER_H
