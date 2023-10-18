// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/abstractentryfileentity.h>

namespace dfmbase {

using EntityCreator = std::function<AbstractEntryFileEntity *(const QUrl &url)>;
QHash<QString, EntityCreator> EntryEntityFactor::creators {};

/*!
 * \class EntryFileEntity
 * \brief class that present devices
 */
AbstractEntryFileEntity::AbstractEntryFileEntity(const QUrl &url)
    : QObject(nullptr), entryUrl(url)
{
}

AbstractEntryFileEntity::~AbstractEntryFileEntity()
{
}

}   // namespace dfmbase
