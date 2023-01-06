// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_BACKEND_CHINESE2PINYIN_H_
#define SERVICE_BACKEND_CHINESE2PINYIN_H_

#include <QString>

namespace Pinyin {
QString Chinese2Pinyin(const QString& words);
};

#endif  // SERVICE_BACKEND_CHINESE2PINYIN_H_
