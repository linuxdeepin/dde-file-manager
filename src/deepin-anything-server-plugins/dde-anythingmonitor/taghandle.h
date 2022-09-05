// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGHANDLE_H
#define TAGHANDLE_H

#include <dasinterface.h>


using namespace DAS_NAMESPACE;

class TagHandle : public DASInterface
{
    Q_OBJECT

public:
    explicit TagHandle(QObject *const parent = nullptr);
    virtual ~TagHandle() = default;

    TagHandle(const TagHandle &other) = delete;
    TagHandle &operator=(const TagHandle &other) = delete;


    virtual void onFileCreate(const QByteArrayList &files) override;
    virtual void onFileDelete(const QByteArrayList &files) override;
    virtual void onFileRename(const QList<QPair<QByteArray, QByteArray>> &files) override;
};

#endif // TAGHANDLE_H
