/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "emblemeventsequence.h"

#include "services/common/emblem/emblem_defines.h"

Q_DECLARE_METATYPE(QList<QIcon> *)

DPF_USE_NAMESPACE
DSC_USE_NAMESPACE
DPEMBLEM_USE_NAMESPACE

EmblemEventSequence *EmblemEventSequence::instance()
{
    static EmblemEventSequence ins;
    return &ins;
}
/*!
 * \brief EmblemEventSequence::doFetchExtendEmblems
 * \param url
 * \param emblems empty list, put extend emblems in this list
 * \return true: Except for system emblems, other emblems will not be added
 *
 * Extend emblem base on system emblem.
 *
 */
bool EmblemEventSequence::doFetchExtendEmblems(const QUrl &url, QList<QIcon> *emblems)
{
    return sequence()->run(Emblem::EventType::kFetchExtendEmblems, url, emblems);
}

/*!
 * \brief EmblemEventSequence::doFetchCustomEmblems
 * \param url
 * \param emblems all emblems, include system emblems, extend emblems and gio emblems
 * \return
 */
bool EmblemEventSequence::doFetchCustomEmblems(const QUrl &url, QList<QIcon> *emblems)
{
    return sequence()->run(Emblem::EventType::kFetchCustomEmblems, url, emblems);
}

EmblemEventSequence::EmblemEventSequence(QObject *parent)
    : QObject(parent)
{
}

EventSequenceManager *EmblemEventSequence::sequence()
{
    return &dpfInstance.eventSequence();
}
