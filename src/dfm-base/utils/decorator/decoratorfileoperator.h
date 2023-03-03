// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
