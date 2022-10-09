/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "mode/normalized/normalizedmode_p.h"
#include "models/collectionmodel_p.h"
#include "mode/normalized/type/typeclassifier.h"
#include "config/configpresenter.h"

#include "dfm-base/dfm_global_defines.h"

#include "dpf.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(NormalizedMode, removeClassifier)
{
    NormalizedMode mode;
    CollectionModel model;
    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, [](){
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
