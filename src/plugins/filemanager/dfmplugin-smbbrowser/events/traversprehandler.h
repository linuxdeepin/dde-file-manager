// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRAVERSPREHANDLER_H
#define TRAVERSPREHANDLER_H

#include "dfmplugin_smbbrowser_global.h"

#include <QUrl>
#include <QObject>

#include <functional>

DPSMBBROWSER_BEGIN_NAMESPACE

namespace travers_prehandler {
using Handler = std::function<void()>;
void networkAccessPrehandler(quint64 winId, const QUrl &url, Handler after);
void smbAccessPrehandler(quint64 winId, const QUrl &url, Handler after);

void doChangeCurrentUrl(quint64 winId, const QString &mpt, const QString &subPath, const QUrl &sourceUrl);
void onSmbRootMounted(const QString &mountSource, Handler after);
}   // namespace travers_prehandler

namespace prehandler_utils {
QString splitMountSource(const QString &source, QString *subPath = nullptr);
}   // namespace prehandler_utils

DPSMBBROWSER_END_NAMESPACE

using PrehandlerFunc = std::function<void(quint64 winId, const QUrl &url, std::function<void()> after)>;
Q_DECLARE_METATYPE(PrehandlerFunc);

#endif   // TRAVERSPREHANDLER_H
