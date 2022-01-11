/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef DESKTOPFILEACTIONS_H
#define DESKTOPFILEACTIONS_H

#include "dfm-base/interfaces/abstractfileactions.h"

DFMBASE_BEGIN_NAMESPACE

class DesktopFileActions : public AbstractFileActions
{
public:
    explicit DesktopFileActions() = delete;
    explicit DesktopFileActions(AbstractFileInfoPointer fInfo);
    ~DesktopFileActions();

    QVector<ActionType> menuActionList(AbtMenuType type = AbtMenuType::kSingleFile) const;
    QSet<ActionType> disableMenuActionList() const;

    static QMap<QString, QVariant> getDesktopFileInfo(const QUrl &fileUrl);
    void updateInfo(const QUrl &fileUrl);

private:
    // todo(Lee): DesktopDecoratesPrivate
    QString name;
    QString genericName;
    QString exec;
    QString iconName;
    QString type;
    QStringList categories;
    QStringList mimeType;
    QString deepinID;
    QString deepinVendor;
};

DFMBASE_END_NAMESPACE
#endif   // DESKTOPFILEACTIONS_H
