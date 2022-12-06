// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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
    static TemplateMenu *instance();
    ~TemplateMenu();

    void loadTemplateFile();
    QList<QAction *> actionList();

protected:
    explicit TemplateMenu(QObject *parent = nullptr);

private:
    QScopedPointer<TemplateMenuPrivate> d;
};

} // dfmplugin_menu

#endif   // TEMPLATEMENU_H
