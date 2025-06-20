// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INNERDESKTOPAPPFILTER_H
#define INNERDESKTOPAPPFILTER_H

#include "models/modeldatahandler.h"

#include <QMap>
#include <QList>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QGSettings>
#endif

namespace ddplugin_organizer {

class InnerDesktopAppFilter : public QObject, public ModelDataHandler
{
    Q_OBJECT
public:
    explicit InnerDesktopAppFilter(QObject *parent = nullptr);
    void update();
    void refreshModel();
    bool acceptInsert(const QUrl &url) override;
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override;
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;
public slots:
    void changed(const QString &key);

protected:
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGSettings *gsettings = nullptr;
#endif
    QMap<QString, QUrl> keys;
    QMap<QString, bool> hidden;
};

}

#endif   // INNERDESKTOPAPPFILTER_H
