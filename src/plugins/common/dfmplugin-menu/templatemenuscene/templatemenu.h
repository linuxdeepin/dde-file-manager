// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEMENU_H
#define TEMPLATEMENU_H

#include <QObject>
#include <QAction>

namespace dfmplugin_menu {

class TemplateMenuPrivate;
class TemplateMenu : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TemplateMenu)
public:
    explicit TemplateMenu(QObject *parent = nullptr);
    ~TemplateMenu();

    void loadTemplateFile();
    QList<QAction *> actionList();
private:
    QScopedPointer<TemplateMenuPrivate> d;
};

} // dfmplugin_menu

#endif   // TEMPLATEMENU_H
