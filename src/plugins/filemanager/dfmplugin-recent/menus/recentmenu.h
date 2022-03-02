/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef RECENTMENU_H
#define RECENTMENU_H

#include "dfmplugin_recent_global.h"
#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"

#include <QUrl>

DPRECENT_BEGIN_NAMESPACE

namespace RecentScene {
extern const char *const kRecentMenu;
}   // namespace MenuScene

class RecentMenu : public DFMBASE_NAMESPACE::AbstractMenu
{
public:
    explicit RecentMenu(QObject *parent = nullptr);
    QMenu *build(QWidget *parent,
                 MenuMode mode,
                 const QUrl &rootUrl,
                 const QUrl &foucsUrl,
                 const QList<QUrl> &selected = {},
                 QVariant customData = QVariant()) override;

private:
    enum class RecentActionType : int {
        kRemove = DFMBASE_NAMESPACE::ActionType::kActCustomBase + 1,
    };

    void removeRecent();
    void actionBusiness(QAction *act) override;
    void assemblesEmptyAreaActions(QMenu *menu);
    void assemblesNormalActions(QMenu *menu);

    void filterEmptyActions(QMenu *menu);
    void filterNormalActions(QMenu *menu);
    void filterActions(QMenu *menu, const QVector<DFMBASE_NAMESPACE::ActionType> &typeList);

    QAction *createAction(const RecentActionType type, const QString &text, bool isDisabled = false);
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
    QUrl rootUrl;
    QUrl foucsUrl;
    QList<QUrl> selectedUrls;
};

DPRECENT_END_NAMESPACE

#endif   // RECENTMENU_H
