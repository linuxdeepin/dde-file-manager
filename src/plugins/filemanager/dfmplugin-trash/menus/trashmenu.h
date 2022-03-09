/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TRASHMENU_H
#define TRASHMENU_H

#include "dfmplugin_trash_global.h"

#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"
#include "dfm-base/dfm_actiontype_defines.h"

#include <QUrl>

DPTRASH_BEGIN_NAMESPACE

namespace TrashScene {
extern const char *const kTrashMenu;
}   // namespace MenuScene

class TrashMenu : public DFMBASE_NAMESPACE::AbstractMenu
{
public:
    explicit TrashMenu(QObject *parent = nullptr);
    QMenu *build(QWidget *parent,
                 MenuMode mode,
                 const QUrl &rootUrl,
                 const QUrl &focusUrl,
                 const QList<QUrl> &selected = {},
                 QVariant customData = QVariant()) override;

private:
    enum TrashActionType {
        kRestore = DFMBASE_NAMESPACE::ActionType::kActCustomBase + 1,
        kRestoreAll,
        kEmptyTrash
    };

    void actionBusiness(QAction *act) override;

    void assemblesEmptyAreaActions(QMenu *menu);
    void assemblesNormalActions(QMenu *menu);

    void filterEmptyActions(QMenu *menu);
    void filterNormalActions(QMenu *menu);
    void filterActions(QMenu *menu, const QVector<DFMBASE_NAMESPACE::ActionType> &typeList);

    void emptyTrash();
    void restore();
    void restoreAll();
    QAction *createAction(const TrashActionType type, const QString &text, bool isDisabled = false);
    QMenu *createMenu(QWidget *parent,
                      const QString &scene,
                      DFMBASE_NAMESPACE::AbstractMenu::MenuMode mode,
                      const QUrl &rootUrl,
                      const QUrl &focusUrl,
                      const QList<QUrl> selected,
                      bool onDesktop = false,
                      DFMBASE_NAMESPACE::ExtensionType flags = DFMBASE_NAMESPACE::ExtensionType::kAllExtensionAction,
                      QVariant customData = QVariant());

private:
    QUrl rootUrl;
    QUrl focusUrl;
    QList<QUrl> selectedUrls;
};

DPTRASH_END_NAMESPACE

#endif   // TRASHMENU_H
