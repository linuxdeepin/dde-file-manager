// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHHINTCONTROLLER_H
#define SEARCHHINTCONTROLLER_H

#include "dfmplugin_search_global.h"

#include <QUrl>
#include <QVariantMap>

namespace dfmplugin_search {

class SearchHintController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchHintController)

public:
    static SearchHintController *instance();

    static bool shouldShowAuthHint(const QUrl &url, QString *text);
    static void onAuthHintAction(const QString &id);

    static void authorizeSearchExperience();
    static void dismissAuthHint();

    static void showHint(const QVariantMap &content);
    static void closeHint();
    static void updateHint(const QVariantMap &updates);

private:
    explicit SearchHintController(QObject *parent = nullptr);
    ~SearchHintController() override;

    QStringList disabledSearchModes();
};

}

#endif   // SEARCHHINTCONTROLLER_H
