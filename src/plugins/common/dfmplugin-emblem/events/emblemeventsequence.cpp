// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emblemeventsequence.h"

#include <dfm-framework/event/event.h>

#include <QUrl>

Q_DECLARE_METATYPE(QList<QIcon> *)

DPF_USE_NAMESPACE
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
    return dpfHookSequence->run(DPF_MACRO_TO_STR(DPEMBLEM_NAMESPACE), "hook_ExtendEmblems_Fetch", url, emblems);
}

/*!
 * \brief EmblemEventSequence::doFetchCustomEmblems
 * \param url
 * \param emblems all emblems, include system emblems, extend emblems and gio emblems
 * \return
 */
bool EmblemEventSequence::doFetchCustomEmblems(const QUrl &url, QList<QIcon> *emblems)
{
    return dpfHookSequence->run(DPF_MACRO_TO_STR(DPEMBLEM_NAMESPACE), "hook_CustomEmblems_Fetch", url, emblems);
}

EmblemEventSequence::EmblemEventSequence(QObject *parent)
    : QObject(parent)
{
}
