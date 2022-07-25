/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#ifndef DECORATORFILEOPERATOR_H
#define DECORATORFILEOPERATOR_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-io/core/doperator.h>

namespace dfmbase {

class DecoratorFileOperatorPrivate;
class DecoratorFileOperator
{
public:
    explicit DecoratorFileOperator(const QString &filePath);
    explicit DecoratorFileOperator(const QUrl &url);
    explicit DecoratorFileOperator(QSharedPointer<DFMIO::DOperator> dfileOperator);
    ~DecoratorFileOperator() = default;

public:
    QSharedPointer<DFMIO::DOperator> operatorPtr();
    bool deleteFile();

    DFMIOError lastError() const;

private:
    QSharedPointer<DecoratorFileOperatorPrivate> d = nullptr;
};

}

#endif   // DECORATORFILEOPERATOR_H
