/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHMENU_H
#define SEARCHMENU_H

#include "dfmplugin_search_global.h"

#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"
#include "dfm-base/dfm_actiontype_defines.h"

#include <QUrl>

DPSEARCH_BEGIN_NAMESPACE

namespace SearchScene {
extern const char *const kSearchMenu;
}   // namespace SearchScene

class SearchMenu : public DFMBASE_NAMESPACE::AbstractMenu
{
public:
    explicit SearchMenu(QObject *parent = nullptr);
    QMenu *build(QWidget *parent,
                 MenuMode mode,
                 const QUrl &rootUrl,
                 const QUrl &foucsUrl,
                 const QList<QUrl> &selected = {},
                 QVariant customData = QVariant()) override;

private:
    enum SearchActionType {
        kOpenFileLocation = DFMBASE_NAMESPACE::ActionType::kActCustomBase + 1
    };

    void actionBusiness(QAction *act) override;

    void assemblesEmptyAreaActions(QMenu *menu);
    void assemblesNormalActions(QMenu *menu);

    void openFileLocation();
    QAction *createAction(const SearchActionType type, const QString &text, bool isDisabled = false);
    QMenu *createMenu(QWidget *parent,
                      const QString &scene,
                      DFMBASE_NAMESPACE::AbstractMenu::MenuMode mode,
                      const QUrl &rootUrl,
                      const QUrl &foucsUrl,
                      const QList<QUrl> selected,
                      bool onDesktop = false,
                      DFMBASE_NAMESPACE::ExtensionType flags = DFMBASE_NAMESPACE::ExtensionType::kAllExtensionAction,
                      QVariant customData = QVariant());

private:
    QList<QUrl> selectedUrls;
};

DPSEARCH_END_NAMESPACE

#endif   // SEARCHMENU_H
