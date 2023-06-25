// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEWMENUHELPER_H
#define FILEVIEWMENUHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QString>

namespace dfmplugin_workspace {

class FileView;
class FileViewMenuHelper : public QObject
{
    Q_OBJECT
public:
    explicit FileViewMenuHelper(FileView *view = nullptr);
    static bool disableMenu();
    void showEmptyAreaMenu();
    void showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags);

    void setMenuScene(const QString &scene);
    void setWaitCursor();
    void reloadCursor();

private:
    QString currentMenuScene() const;

    FileView *view { nullptr };
};

}

#endif   // FILEVIEWMENUHELPER_H
