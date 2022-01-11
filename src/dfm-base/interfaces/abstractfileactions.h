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
#ifndef ABSTRACTFILEACTIONS_H
#define ABSTRACTFILEACTIONS_H

#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QVector>
#include <QSet>

#define AbtMenuType AbstractMenu::MenuType

DFMBASE_BEGIN_NAMESPACE
class AbstractFileActions : public AbstractFileInfo
{

public:
    explicit AbstractFileActions() = delete;
    explicit AbstractFileActions(AbstractFileInfoPointer fInfo);
    virtual ~AbstractFileActions();
    virtual QVector<ActionType> menuActionList(AbtMenuType type = AbtMenuType::kSingleFile) const;
    virtual QSet<ActionType> disableMenuActionList() const;
    QSharedPointer<AbstractFileInfo> getFileInfo() const;
    void setFileInfo(QSharedPointer<AbstractFileInfo> fInfo);

private:
    QSharedPointer<AbstractFileInfo> fileInfo;
};

typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileActions> AbstractDecoratesInfoPointer;

DFMBASE_END_NAMESPACE
#endif   // ABSTRACTFILEACTIONS_H
