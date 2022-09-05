// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEICONPROVIDER_H
#define DFILEICONPROVIDER_H

#include <QFileIconProvider>

#include "dfmglobal.h"

class DFileInfo;
class DGvfsFileInfo;

DFM_BEGIN_NAMESPACE

class DFileIconProviderPrivate;
class DFileIconProvider : public QFileIconProvider
{
public:
    DFileIconProvider();
    ~DFileIconProvider() override;

    static DFileIconProvider *globalProvider();

    QIcon icon(const QFileInfo &info) const override;
    QIcon icon(const QFileInfo &info, const QIcon &feedback) const ;
    QIcon icon(const DFileInfo &info, const QIcon &feedback = QIcon()) const;
    QIcon icon(const DGvfsFileInfo &info, const QIcon &feedback = QIcon()) const;

private:
    QScopedPointer<DFileIconProviderPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFileIconProvider)
    Q_DISABLE_COPY(DFileIconProvider)
};

DFM_END_NAMESPACE

#endif // DFILEICONPROVIDER_H
