// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGFILEHELPER_H
#define TAGFILEHELPER_H

#include "dfmplugin_tag_global.h"

#include <QObject>

DPTAG_BEGIN_NAMESPACE

class TagFileHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TagFileHelper)
public:
    static TagFileHelper *instance();
    inline static QString scheme()
    {
        return "tag";
    }

    // file operation
    bool openFileInPlugin(quint64 windowId, const QList<QUrl> urls);

private:
    explicit TagFileHelper(QObject *parent = nullptr);
};
DPTAG_END_NAMESPACE

#endif   // TAGFILEHELPER_H
