// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEICONPROVIDER_H
#define LOCALFILEICONPROVIDER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QFileIconProvider>
#include <QScopedPointer>

namespace dfmbase {

class LocalFileIconProviderPrivate;
class LocalFileIconProvider : public QFileIconProvider
{
public:
    LocalFileIconProvider();
    ~LocalFileIconProvider() override;
    using QFileIconProvider::icon;

    static LocalFileIconProvider *globalProvider();

    QIcon icon(const QFileInfo &info) const override;
    QIcon icon(const QString &path) const;
    QIcon icon(const QFileInfo &info, const QIcon &feedback) const;
    QIcon icon(const QString &path, const QIcon &feedback) const;
    QIcon icon(FileInfoPointer info, const QIcon &feedback = QIcon());

private:
    QScopedPointer<LocalFileIconProviderPrivate> d;

    Q_DISABLE_COPY(LocalFileIconProvider)
};

}

#endif   // LOCALFILEICONPROVIDER_H
