// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHREDUTILS_H
#define SHREDUTILS_H

#include "dfmplugin_utils_global.h"

#include <QObject>

namespace dfmplugin_utils {
class ProgressDialog;
class ShredUtils : public QObject
{
    Q_OBJECT

public:
    static ShredUtils *instance();

    void setShredEnabled(bool enable);
    bool isShredEnabled();
    void initDconfig();
    bool isValidFile(const QUrl &file);
    void shredfile(const QList<QUrl> &fileList, quint64 winId);

    static QWidget *createShredSettingItem(QObject *opt);

private:
    explicit ShredUtils(QObject *parent = nullptr);
    ~ShredUtils();

    bool confirmAndDisplayFiles(const QList<QUrl> &fileList);
};

}   // namespace dfmplugin_utils

#endif   // SHREDUTILS_H
