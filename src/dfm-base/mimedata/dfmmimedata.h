// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMMIMEDATA_H
#define DFMMIMEDATA_H

#include <dfm-base/dfm_base_global.h>

#include <QSharedPointer>
#include <QVariant>

namespace dfmbase {

class DFMMimeDataPrivate;
class DFMMimeData
{
public:
    explicit DFMMimeData();
    DFMMimeData(const DFMMimeData &other);
    ~DFMMimeData();

    DFMMimeData &operator=(const DFMMimeData &fileinfo);
    DFMMimeData &operator=(DFMMimeData &&other) noexcept { swap(other); return *this; }

    void swap(DFMMimeData &other) noexcept
    { qSwap(d, other.d); }

    QString version() const;
    bool isValid() const;
    void clear();

    void setUrls(const QList<QUrl> &urls);
    QList<QUrl> urls() const;

    bool canTrash() const;
    bool canDelete() const;
    bool isTrashFile() const;

    void setAttritube(const QString &name, const QVariant &value);
    QVariant attritube(const QString &name, const QVariant &defaultValue = {}) const;

    QByteArray toByteArray();
    static DFMMimeData fromByteArray(const QByteArray &data);

private:
    QSharedDataPointer<DFMMimeDataPrivate> d;
};

}   // namespace dfmbase

#endif   // DFMMIMEDATA_H
