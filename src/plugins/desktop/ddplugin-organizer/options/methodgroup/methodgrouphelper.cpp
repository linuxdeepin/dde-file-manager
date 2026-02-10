// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "methodgrouphelper.h"
#include "typemethodgroup.h"

using namespace ddplugin_organizer;

MethodGroupHelper::MethodGroupHelper(QObject *parent) : QObject(parent)
{

}

MethodGroupHelper *MethodGroupHelper::create(Classifier id)
{
    MethodGroupHelper *ret = nullptr;
    switch (id) {
    case kType:
        ret = new TypeMethodGroup();
        break;
    default:
        break;
    }

    return ret;
}

MethodGroupHelper::~MethodGroupHelper()
{
}

void MethodGroupHelper::release()
{

}

bool MethodGroupHelper::build()
{
    return false;
}
