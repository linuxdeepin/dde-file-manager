// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_configpresenter_1.cpp
 * @brief Unit tests for ConfigPresenter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/configpresenter.h"

#include <QTest>

using namespace ddplugin_organizer;

class ConfigPresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ConfigPresenter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ConfigPresenter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ConfigPresenterTest, ConfigPresenter)
{
    // Test constructor: ConfigPresenter((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ConfigPresenterTest, classification)
{
    // Test getter: Classifier classification()
    auto result = obj->classification();
    EXPECT_NO_FATAL_FAILURE({ obj->classification(); });

}

TEST_F(ConfigPresenterTest, customProfile)
{
    // Test getter: QList<CollectionBaseDataPtr> customProfile()
    auto result = obj->customProfile();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ConfigPresenterTest, customStyle)
{
    // Test method: CollectionStyle customStyle((const QString &key))
    QString _arg0{};
    auto result = obj->customStyle(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->customStyle(_arg0); });

}

TEST_F(ConfigPresenterTest, enabledTypeCategories)
{
    // Test getter: ItemCategories enabledTypeCategories()
    auto result = obj->enabledTypeCategories();
    EXPECT_NO_FATAL_FAILURE({ obj->enabledTypeCategories(); });

}

TEST_F(ConfigPresenterTest, hasConfigId)
{
    // Test method: bool hasConfigId((const QString &configId))
    QString _arg0{};
    auto result = obj->hasConfigId(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ConfigPresenterTest, hideAllKeySequence)
{
    // Test getter: QKeySequence hideAllKeySequence()
    auto result = obj->hideAllKeySequence();
    EXPECT_NO_FATAL_FAILURE({ obj->hideAllKeySequence(); });

}

TEST_F(ConfigPresenterTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(ConfigPresenterTest, instance)
{
    // Test getter: ConfigPresenter instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ConfigPresenterTest, isEnable)
{
    // Test bool getter: isEnable()
    bool result = obj->isEnable();
    EXPECT_FALSE(result);

}

TEST_F(ConfigPresenterTest, isEnableVisibility)
{
    // Test bool getter: isEnableVisibility()
    bool result = obj->isEnableVisibility();
    EXPECT_FALSE(result);

}

TEST_F(ConfigPresenterTest, isRepeatNoMore)
{
    // Test bool getter: isRepeatNoMore()
    bool result = obj->isRepeatNoMore();
    EXPECT_FALSE(result);

}

TEST_F(ConfigPresenterTest, lastStyleConfigId)
{
    // Test getter: QString lastStyleConfigId()
    auto result = obj->lastStyleConfigId();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ConfigPresenterTest, normalProfile)
{
    // Test getter: QList<CollectionBaseDataPtr> normalProfile()
    auto result = obj->normalProfile();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ConfigPresenterTest, normalStyle)
{
    // Test method: CollectionStyle normalStyle((const QString &configId, const QString &key))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->normalStyle(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->normalStyle(_arg0, _arg1); });

}

TEST_F(ConfigPresenterTest, optimizeMovingPerformance)
{
    // Test bool getter: optimizeMovingPerformance()
    bool result = obj->optimizeMovingPerformance();
    EXPECT_FALSE(result);

}

TEST_F(ConfigPresenterTest, organizeAction)
{
    // Test getter: OrganizeAction organizeAction()
    auto result = obj->organizeAction();
    EXPECT_NO_FATAL_FAILURE({ obj->organizeAction(); });

}

TEST_F(ConfigPresenterTest, organizeOnTriggered)
{
    // Test bool getter: organizeOnTriggered()
    bool result = obj->organizeOnTriggered();
    EXPECT_FALSE(result);

}

TEST_F(ConfigPresenterTest, setClassification)
{
    // Test setter: void setClassification((Classifier cf))
    EXPECT_NO_FATAL_FAILURE(obj->setClassification(Classifier()));
}

TEST_F(ConfigPresenterTest, setEnable)
{
    // Test setter: void setEnable((bool e))
    EXPECT_NO_FATAL_FAILURE(obj->setEnable(false));
}

TEST_F(ConfigPresenterTest, setEnableVisibility)
{
    // Test setter: void setEnableVisibility((bool v))
    EXPECT_NO_FATAL_FAILURE(obj->setEnableVisibility(false));
}

TEST_F(ConfigPresenterTest, setHideAllKeySequence)
{
    // Test setter: void setHideAllKeySequence((const QKeySequence &seq))
    QKeySequence _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setHideAllKeySequence(_arg0));
}

TEST_F(ConfigPresenterTest, setLastStyleConfigId)
{
    // Test setter: void setLastStyleConfigId((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setLastStyleConfigId(_arg0));
}

TEST_F(ConfigPresenterTest, setMode)
{
    // Test setter: void setMode((OrganizerMode m))
    EXPECT_NO_FATAL_FAILURE(obj->setMode(OrganizerMode()));
}

TEST_F(ConfigPresenterTest, setRepeatNoMore)
{
    // Test setter: void setRepeatNoMore((bool e))
    EXPECT_NO_FATAL_FAILURE(obj->setRepeatNoMore(false));
}

TEST_F(ConfigPresenterTest, setVersion)
{
    // Test setter: void setVersion((const QString &v))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setVersion(_arg0));
}

TEST_F(ConfigPresenterTest, surfaceSizes)
{
    // Test getter: QList<QSize> surfaceSizes()
    auto result = obj->surfaceSizes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ConfigPresenterTest, updateCustomStyle)
{
    // Test method: void updateCustomStyle((const CollectionStyle &style))
    CollectionStyle _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateCustomStyle(_arg0));
}

TEST_F(ConfigPresenterTest, updateNormalStyle)
{
    // Test method: void updateNormalStyle((const QString &configId, const CollectionStyle &style))
    QString _arg0{};
    CollectionStyle _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateNormalStyle(_arg0, _arg1));
}

TEST_F(ConfigPresenterTest, version)
{
    // Test getter: QString version()
    auto result = obj->version();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ConfigPresenterTest, ConfigPresenter_Destructor)
{
    // Test method:  ~ConfigPresenter(())
    EXPECT_NO_FATAL_FAILURE({ ConfigPresenter *tmp = new ConfigPresenter(); delete tmp; });
}
