// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/abstractframe.h>

using namespace dfmbase;

AbstractFrame::AbstractFrame(QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f)
{
}

AbstractFrame::~AbstractFrame()
{
}
