// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/normalized/normalizedmode_p.h"
#include "models/collectionmodel_p.h"
#include "mode/normalized/type/typeclassifier.h"
#include "config/configpresenter.h"

#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

#include <stubext.h>

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(NormalizedMode, removeClassifier)
{
    NormalizedMode mode;
    CollectionModel model;
    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, []() {
        return ItemCategories(0);
    });

    TypeClassifier *type = new TypeClassifier;
    mode.d->classifier = type;
    ModelDataHandler *handler = type->dataHandler();
    model.d->handler = handler;
    mode.model = &model;

    mode.removeClassifier();
    EXPECT_EQ(mode.d->classifier, nullptr);
    EXPECT_EQ(model.handler(), nullptr);
    EXPECT_NE(mode.d->broker, nullptr);
}
