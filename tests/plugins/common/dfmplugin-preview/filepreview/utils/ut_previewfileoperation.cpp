// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/previewfileoperation.h"

#include <gtest/gtest.h>

#include <QUrl>

#include <dfm-framework/event/event.h>

Q_DECLARE_METATYPE(bool *)

DPFILEPREVIEW_USE_NAMESPACE
DPF_USE_NAMESPACE

TEST(UT_PreviewFileOperation, openFileHandle)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, QList<QUrl> &, bool *&&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    PreviewFileOperation op;
    bool isOk = PreviewFileOperation::openFileHandle(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}
