// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "config/configpresenter.h"
#include "config/organizerconfig.h"
#include "mode/canvasorganizer.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <gtest/gtest.h>
#include <QKeySequence>
#include <QWidget>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE

namespace {
// fixture-local key used by the LastStyleConfigId round-trip test
const char *kLastStyleConfigIdTestKey = "test/last_style_config_id";
}   // namespace

class ConfigPresenterImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        presenter = ConfigPresenter::instance();
        presenter->initialize();

        // avoid real dconfig / file writes: back the DConfigManager API with
        // an in-memory map so set/get round-trips work without touching disk
        stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                       [this](DConfigManager *, const QString &, const QString &key, const QVariant &fallback) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return dconfigStore.value(key, fallback);
                       });
        stub.set_lamda(&DConfigManager::setValue,
                       [this](DConfigManager *, const QString &, const QString &key, const QVariant &value) {
                           __DBG_STUB_INVOKE__
                           dconfigStore[key] = value;
                       });
        stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        // restore predictable defaults
        presenter->setEnable(false);
        presenter->setMode(OrganizerMode::kNormalized);
        presenter->setClassification(Classifier::kType);
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    QMap<QString, QVariant> dconfigStore;
    ConfigPresenter *presenter = nullptr;
};

TEST_F(ConfigPresenterImpl, Initialize_ReturnsFalseWhenCalledAgain)
{
    EXPECT_FALSE(presenter->initialize());
}

TEST_F(ConfigPresenterImpl, Version_RoundTrip)
{
    stub.set_lamda(&OrganizerConfig::setVersion, [](OrganizerConfig *, const QString &) {
        __DBG_STUB_INVOKE__
    });
    presenter->setVersion("2.0.0");
    EXPECT_EQ(presenter->version(), "2.0.0");
}

TEST_F(ConfigPresenterImpl, Enable_RoundTrip)
{
    stub.set_lamda(&OrganizerConfig::setEnable, [](OrganizerConfig *, bool) {
        __DBG_STUB_INVOKE__
    });
    presenter->setEnable(true);
    EXPECT_TRUE(presenter->isEnable());
    presenter->setEnable(false);
    EXPECT_FALSE(presenter->isEnable());
}

TEST_F(ConfigPresenterImpl, EnableVisibility_RoundTrip)
{
    presenter->setEnableVisibility(true);
    EXPECT_TRUE(presenter->isEnableVisibility());
    presenter->setEnableVisibility(false);
    EXPECT_FALSE(presenter->isEnableVisibility());
}

TEST_F(ConfigPresenterImpl, Mode_RoundTrip)
{
    stub.set_lamda(&OrganizerConfig::setMode, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });
    presenter->setMode(OrganizerMode::kCustom);
    EXPECT_EQ(presenter->mode(), OrganizerMode::kCustom);
    presenter->setMode(OrganizerMode::kNormalized);
    EXPECT_EQ(presenter->mode(), OrganizerMode::kNormalized);
}

TEST_F(ConfigPresenterImpl, Classification_RoundTrip)
{
    stub.set_lamda(&OrganizerConfig::setClassification, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });
    presenter->setClassification(Classifier::kSize);
    EXPECT_EQ(presenter->classification(), Classifier::kSize);
    presenter->setClassification(Classifier::kType);
    EXPECT_EQ(presenter->classification(), Classifier::kType);
}

TEST_F(ConfigPresenterImpl, HideAllKeySequence_RoundTrip)
{
    QKeySequence seq("Ctrl+Shift+H");
    presenter->setHideAllKeySequence(seq);
    EXPECT_EQ(presenter->hideAllKeySequence(), seq);
}

TEST_F(ConfigPresenterImpl, RepeatNoMore_RoundTrip)
{
    presenter->setRepeatNoMore(true);
    EXPECT_TRUE(presenter->isRepeatNoMore());
    presenter->setRepeatNoMore(false);
    EXPECT_FALSE(presenter->isRepeatNoMore());
}

TEST_F(ConfigPresenterImpl, SurfaceSizes_DelegatesToConfig)
{
    QList<QSize> sizes = { QSize(1920, 1080) };
    stub.set_lamda(&OrganizerConfig::surfaceSizes, [sizes](OrganizerConfig *) -> QList<QSize> {
        __DBG_STUB_INVOKE__
        return sizes;
    });
    EXPECT_EQ(presenter->surfaceSizes(), sizes);
}

TEST_F(ConfigPresenterImpl, SetSurfaceInfo_WithEmptyList_DoesNotCrash)
{
    stub.set_lamda(&OrganizerConfig::setScreenInfo, [](OrganizerConfig *, const QMap<QString, QString> &) {
        __DBG_STUB_INVOKE__
    });
    EXPECT_NO_THROW(presenter->setSurfaceInfo({}));
}

TEST_F(ConfigPresenterImpl, LastStyleConfigId_RoundTrip)
{
    // back the config with the in-memory store so the set/get round-trip works
    stub.set_lamda(&OrganizerConfig::setLastStyleConfigId, [this](OrganizerConfig *, const QString &id) {
        __DBG_STUB_INVOKE__
        dconfigStore[kLastStyleConfigIdTestKey] = id;
    });
    stub.set_lamda(&OrganizerConfig::lastStyleConfigId, [this](const OrganizerConfig *) -> QString {
        __DBG_STUB_INVOKE__
        return dconfigStore.value(kLastStyleConfigIdTestKey).toString();
    });
    presenter->setLastStyleConfigId("cfg_1");
    EXPECT_EQ(presenter->lastStyleConfigId(), "cfg_1");
}

TEST_F(ConfigPresenterImpl, HasConfigId_DelegatesToConfig)
{
    stub.set_lamda(&OrganizerConfig::hasConfigId, [](const OrganizerConfig *, const QString &id) -> bool {
        __DBG_STUB_INVOKE__
        return id == "exists";
    });
    EXPECT_TRUE(presenter->hasConfigId("exists"));
    EXPECT_FALSE(presenter->hasConfigId("missing"));
}

TEST_F(ConfigPresenterImpl, CustomProfile_SaveAndLoad)
{
    QList<CollectionBaseDataPtr> list;
    CollectionBaseDataPtr data(new CollectionBaseData);
    data->key = "custom_key";
    list.append(data);

    stub.set_lamda(qOverload<bool>(&OrganizerConfig::collectionBase), [list](const OrganizerConfig *, bool custom) -> QList<CollectionBaseDataPtr> {
        __DBG_STUB_INVOKE__
        return custom ? list : QList<CollectionBaseDataPtr>();
    });
    stub.set_lamda(&OrganizerConfig::writeCollectionBase,
                   [](OrganizerConfig *, bool, const QList<CollectionBaseDataPtr> &) {
                       __DBG_STUB_INVOKE__
                   });

    EXPECT_EQ(presenter->customProfile().size(), 1);
    EXPECT_NO_THROW(presenter->saveCustomProfile(list));
}

TEST_F(ConfigPresenterImpl, NormalProfile_SaveAndLoad)
{
    QList<CollectionBaseDataPtr> list;
    CollectionBaseDataPtr data(new CollectionBaseData);
    data->key = "normal_key";
    list.append(data);

    stub.set_lamda(qOverload<bool>(&OrganizerConfig::collectionBase), [list](const OrganizerConfig *, bool custom) -> QList<CollectionBaseDataPtr> {
        __DBG_STUB_INVOKE__
        return custom ? QList<CollectionBaseDataPtr>() : list;
    });
    stub.set_lamda(&OrganizerConfig::writeCollectionBase,
                   [](OrganizerConfig *, bool, const QList<CollectionBaseDataPtr> &) {
                       __DBG_STUB_INVOKE__
                   });

    EXPECT_EQ(presenter->normalProfile().size(), 1);
    EXPECT_NO_THROW(presenter->saveNormalProfile(list));
}

TEST_F(ConfigPresenterImpl, NormalStyle_WithEmptyKey_ReturnsEmpty)
{
    CollectionStyle style = presenter->normalStyle("cfg", "");
    EXPECT_TRUE(style.key.isEmpty());
}

TEST_F(ConfigPresenterImpl, NormalStyle_WithKey_DelegatesToConfig)
{
    CollectionStyle mockStyle;
    mockStyle.key = "key";
    stub.set_lamda(&OrganizerConfig::collectionStyle, [mockStyle](const OrganizerConfig *, const QString &, const QString &) -> CollectionStyle {
        __DBG_STUB_INVOKE__
        return mockStyle;
    });
    EXPECT_EQ(presenter->normalStyle("cfg", "key").key, "key");
}

TEST_F(ConfigPresenterImpl, UpdateNormalStyle_DoesNotCrash)
{
    stub.set_lamda(&OrganizerConfig::updateCollectionStyle, [](OrganizerConfig *, const QString &, const CollectionStyle &) {
        __DBG_STUB_INVOKE__
    });
    CollectionStyle style;
    style.key = "key";
    EXPECT_NO_THROW(presenter->updateNormalStyle("cfg", style));
}

TEST_F(ConfigPresenterImpl, WriteNormalStyle_DoesNotCrash)
{
    stub.set_lamda(&OrganizerConfig::writeCollectionStyle, [](OrganizerConfig *, const QString &, const QList<CollectionStyle> &) {
        __DBG_STUB_INVOKE__
    });
    EXPECT_NO_THROW(presenter->writeNormalStyle("cfg", {}));
}

TEST_F(ConfigPresenterImpl, CustomStyle_ReturnsEmpty)
{
    CollectionStyle style = presenter->customStyle("any_key");
    EXPECT_TRUE(style.key.isEmpty());
}

TEST_F(ConfigPresenterImpl, UpdateCustomStyle_DoesNotCrash)
{
    CollectionStyle style;
    style.key = "key";
    EXPECT_NO_THROW(presenter->updateCustomStyle(style));
}

TEST_F(ConfigPresenterImpl, WriteCustomStyle_DoesNotCrash)
{
    EXPECT_NO_THROW(presenter->writeCustomStyle({}));
}

TEST_F(ConfigPresenterImpl, EnabledTypeCategories_RoundTrip)
{
    QStringList stored = { "kApp", "kDocument" };
    stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                   [stored](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return stored;
                   });

    ItemCategories cats = presenter->enabledTypeCategories();
    EXPECT_TRUE(cats & kCatApplication);
    EXPECT_TRUE(cats & kCatDocument);

    ItemCategories newCats = static_cast<ItemCategory>(kCatPicture | kCatVideo);
    EXPECT_NO_THROW(presenter->setEnabledTypeCategories(newCats));
}

TEST_F(ConfigPresenterImpl, OrganizeAction_MapsValues)
{
    stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(0);
                   });
    EXPECT_EQ(presenter->organizeAction(), kOnTrigger);
    EXPECT_TRUE(presenter->organizeOnTriggered());

    stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(1);
                   });
    EXPECT_EQ(presenter->organizeAction(), kAlways);
    EXPECT_FALSE(presenter->organizeOnTriggered());
}

TEST_F(ConfigPresenterImpl, OptimizeMovingPerformance_DelegatesToConfig)
{
    stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(true);
                   });
    EXPECT_TRUE(presenter->optimizeMovingPerformance());
}
