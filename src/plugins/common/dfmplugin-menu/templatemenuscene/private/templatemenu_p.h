// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEMENU_P_H
#define TEMPLATEMENU_P_H

#include "dfmplugin_menu_global.h"
#include "templatemenuscene/templatemenu.h"
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <QSharedData>

namespace dfmplugin_menu {

class TemplateMenu;
class TemplateMenuPrivate : public QSharedData
{
public:
    explicit TemplateMenuPrivate(TemplateMenu *qq);
    ~TemplateMenuPrivate();

    void loadTemplatePaths();

    void createActionByNormalFile(const QString &path);
    void createActionByDesktopFile(const QDir &dir, const QString &path);
    void traverseFolderToCreateActions(const QString &path, bool isDesktopEntryFolderPath);

public:
    TemplateMenu *q;
    QList<QAction *> templateActions;
    QString templateFolderPath;
    QStringList templateFileNames;
};

}

#endif   // TEMPLATEMENU_P_H
