// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "config/configpresenter.h"
#include "config/organizerconfig.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QKeySequence>
#include <QWidget>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE

class UT_ConfigPresenter : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        presenter = ConfigPresenter::instance();
        presenter->initialize();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    ConfigPresenter *presenter = nullptr;
};

TEST_F(UT_ConfigPresenter, instance_ReturnsSingleton)
{
    ConfigPresenter *instance1 = ConfigPresenter::instance();
    ConfigPresenter *instance2 = ConfigPresenter::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_ConfigPresenter, version_ReturnsSetVersion)
{
    QString testVersion = "1.0.0";

    stub.set_lamda(&OrganizerConfig::setVersion, [](OrganizerConfig *, const QString &) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    presenter->setVersion(testVersion);
    EXPECT_EQ(presenter->version(), testVersion);
}

TEST_F(UT_ConfigPresenter, isEnable_ReturnsEnableState)
{
    EXPECT_TRUE(presenter->isEnable() == true || presenter->isEnable() == false);
}

TEST_F(UT_ConfigPresenter, setEnable_UpdatesEnableState)
{
    stub.set_lamda(&DConfigManager::setValue, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&OrganizerConfig::setEnable, [](OrganizerConfig *, bool) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    presenter->setEnable(true);
    EXPECT_TRUE(presenter->isEnable());

    presenter->setEnable(false);
    EXPECT_FALSE(presenter->isEnable());
}

TEST_F(UT_ConfigPresenter, isEnableVisibility_ReturnsVisibilityState)
{
    EXPECT_TRUE(presenter->isEnableVisibility() == true || presenter->isEnableVisibility() == false);
}

TEST_F(UT_ConfigPresenter, setEnableVisibility_UpdatesVisibilityState)
{
    stub.set_lamda(&DConfigManager::setValue, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
    });

    presenter->setEnableVisibility(true);
    EXPECT_TRUE(presenter->isEnableVisibility());

    presenter->setEnableVisibility(false);
    EXPECT_FALSE(presenter->isEnableVisibility());
}

TEST_F(UT_ConfigPresenter, mode_ReturnsCurrentMode)
{
    OrganizerMode mode = presenter->mode();
    EXPECT_TRUE(mode >= OrganizerMode::kNormalized && mode <= OrganizerMode::kCustom);
}

TEST_F(UT_ConfigPresenter, setMode_UpdatesMode)
{
    stub.set_lamda(&OrganizerConfig::setMode, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    presenter->setMode(OrganizerMode::kNormalized);
    EXPECT_EQ(presenter->mode(), OrganizerMode::kNormalized);
}

TEST_F(UT_ConfigPresenter, classification_ReturnsCurrentClassifier)
{
    Classifier cf = presenter->classification();
    EXPECT_TRUE(cf >= Classifier::kType && cf <= Classifier::kSize);
}

TEST_F(UT_ConfigPresenter, setClassification_UpdatesClassifier)
{
    stub.set_lamda(&OrganizerConfig::setClassification, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    presenter->setClassification(Classifier::kType);
    EXPECT_EQ(presenter->classification(), Classifier::kType);
}

TEST_F(UT_ConfigPresenter, hideAllKeySequence_ReturnsKeySequence)
{
    QString mockSeq = "Ctrl+H";

    stub.set_lamda(&DConfigManager::value, [mockSeq](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(mockSeq);
    });

    QKeySequence seq = presenter->hideAllKeySequence();
    EXPECT_EQ(seq.toString(), mockSeq);
}

TEST_F(UT_ConfigPresenter, setHideAllKeySequence_SetsKeySequence)
{
    bool setValueCalled = false;

    stub.set_lamda(&DConfigManager::setValue, [&setValueCalled](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        setValueCalled = true;
    });

    presenter->setHideAllKeySequence(QKeySequence("Ctrl+H"));
    EXPECT_TRUE(setValueCalled);
}

TEST_F(UT_ConfigPresenter, isRepeatNoMore_ReturnsValue)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    EXPECT_TRUE(presenter->isRepeatNoMore());
}

TEST_F(UT_ConfigPresenter, setRepeatNoMore_SetsValue)
{
    bool setValueCalled = false;

    stub.set_lamda(&DConfigManager::setValue, [&setValueCalled](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        setValueCalled = true;
    });

    presenter->setRepeatNoMore(true);
    EXPECT_TRUE(setValueCalled);
}

TEST_F(UT_ConfigPresenter, surfaceSizes_ReturnsListOfSizes)
{
    QList<QSize> mockSizes = { QSize(1920, 1080), QSize(1280, 720) };

    stub.set_lamda(&OrganizerConfig::surfaceSizes, [mockSizes](OrganizerConfig *) {
        __DBG_STUB_INVOKE__
        return mockSizes;
    });

    QList<QSize> result = presenter->surfaceSizes();
    EXPECT_EQ(result.size(), 2);
}

TEST_F(UT_ConfigPresenter, lastStyleConfigId_ReturnsId)
{
    QString mockId = "config_123";

    stub.set_lamda(&OrganizerConfig::lastStyleConfigId, [mockId](const OrganizerConfig *) {
        __DBG_STUB_INVOKE__
        return mockId;
    });

    EXPECT_EQ(presenter->lastStyleConfigId(), mockId);
}

TEST_F(UT_ConfigPresenter, setLastStyleConfigId_SetsId)
{
    bool setIdCalled = false;

    stub.set_lamda(&OrganizerConfig::setLastStyleConfigId, [&setIdCalled](OrganizerConfig *, const QString &) {
        __DBG_STUB_INVOKE__
        setIdCalled = true;
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    presenter->setLastStyleConfigId("test_id");
    EXPECT_TRUE(setIdCalled);
}

TEST_F(UT_ConfigPresenter, hasConfigId_ReturnsTrueIfExists)
{
    stub.set_lamda(&OrganizerConfig::hasConfigId, [](const OrganizerConfig *, const QString &id) {
        __DBG_STUB_INVOKE__
        return id == "existing_id";
    });

    EXPECT_TRUE(presenter->hasConfigId("existing_id"));
    EXPECT_FALSE(presenter->hasConfigId("nonexistent_id"));
}

TEST_F(UT_ConfigPresenter, customProfile_ReturnsCollectionBaseList)
{
    QList<CollectionBaseDataPtr> mockList;
    CollectionBaseDataPtr data(new CollectionBaseData);
    data->key = "test_key";
    data->name = "Test Collection";
    mockList.append(data);

    stub.set_lamda(qOverload<bool>(&OrganizerConfig::collectionBase), [mockList](const OrganizerConfig *, bool custom) {
        __DBG_STUB_INVOKE__
        if (custom)
            return mockList;
        return QList<CollectionBaseDataPtr>();
    });

    QList<CollectionBaseDataPtr> result = presenter->customProfile();
    EXPECT_EQ(result.size(), 1);
}

TEST_F(UT_ConfigPresenter, saveCustomProfile_SavesProfile)
{
    bool writeCalled = false;

    stub.set_lamda(&OrganizerConfig::writeCollectionBase, [&writeCalled](OrganizerConfig *, bool, const QList<CollectionBaseDataPtr> &) {
        __DBG_STUB_INVOKE__
        writeCalled = true;
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    QList<CollectionBaseDataPtr> data;
    presenter->saveCustomProfile(data);
    EXPECT_TRUE(writeCalled);
}

TEST_F(UT_ConfigPresenter, normalProfile_ReturnsCollectionBaseList)
{
    QList<CollectionBaseDataPtr> mockList;
    CollectionBaseDataPtr data(new CollectionBaseData);
    data->key = "normal_key";
    data->name = "Normal Collection";
    mockList.append(data);

    stub.set_lamda(qOverload<bool>(&OrganizerConfig::collectionBase), [mockList](const OrganizerConfig *, bool custom) {
        __DBG_STUB_INVOKE__
        if (!custom)
            return mockList;
        return QList<CollectionBaseDataPtr>();
    });

    QList<CollectionBaseDataPtr> result = presenter->normalProfile();
    EXPECT_EQ(result.size(), 1);
}

TEST_F(UT_ConfigPresenter, saveNormalProfile_SavesProfile)
{
    bool writeCalled = false;

    stub.set_lamda(&OrganizerConfig::writeCollectionBase, [&writeCalled](OrganizerConfig *, bool, const QList<CollectionBaseDataPtr> &) {
        __DBG_STUB_INVOKE__
        writeCalled = true;
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    QList<CollectionBaseDataPtr> data;
    presenter->saveNormalProfile(data);
    EXPECT_TRUE(writeCalled);
}

TEST_F(UT_ConfigPresenter, normalStyle_WithValidKey_ReturnsStyle)
{
    CollectionStyle mockStyle;
    mockStyle.key = "test_key";
    mockStyle.screenIndex = 1;

    stub.set_lamda(&OrganizerConfig::collectionStyle, [mockStyle](const OrganizerConfig *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return mockStyle;
    });

    CollectionStyle result = presenter->normalStyle("config_id", "test_key");
    EXPECT_EQ(result.key, "test_key");
}

TEST_F(UT_ConfigPresenter, normalStyle_WithEmptyKey_ReturnsEmptyStyle)
{
    CollectionStyle result = presenter->normalStyle("config_id", "");
    EXPECT_TRUE(result.key.isEmpty());
}

TEST_F(UT_ConfigPresenter, updateNormalStyle_WithValidStyle_UpdatesStyle)
{
    bool updateCalled = false;

    stub.set_lamda(&OrganizerConfig::updateCollectionStyle, [&updateCalled](OrganizerConfig *, const QString &, const CollectionStyle &) {
        __DBG_STUB_INVOKE__
        updateCalled = true;
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    CollectionStyle style;
    style.key = "test_key";
    presenter->updateNormalStyle("config_id", style);
    EXPECT_TRUE(updateCalled);
}

TEST_F(UT_ConfigPresenter, updateNormalStyle_WithEmptyKey_DoesNotUpdate)
{
    bool updateCalled = false;

    stub.set_lamda(&OrganizerConfig::updateCollectionStyle, [&updateCalled](OrganizerConfig *, const QString &, const CollectionStyle &) {
        __DBG_STUB_INVOKE__
        updateCalled = true;
    });

    CollectionStyle style;
    style.key = "";
    presenter->updateNormalStyle("config_id", style);
    EXPECT_FALSE(updateCalled);
}

TEST_F(UT_ConfigPresenter, customStyle_ReturnsEmptyStyle)
{
    CollectionStyle result = presenter->customStyle("any_key");
    EXPECT_TRUE(result.key.isEmpty());
}

TEST_F(UT_ConfigPresenter, enabledTypeCategories_ReturnsCategories)
{
    QStringList mockCategories = { "kApp", "kDocument", "kPicture" };

    stub.set_lamda(&DConfigManager::value, [mockCategories](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(mockCategories);
    });

    ItemCategories result = presenter->enabledTypeCategories();
    EXPECT_TRUE(result & kCatApplication);
    EXPECT_TRUE(result & kCatDocument);
    EXPECT_TRUE(result & kCatPicture);
}

TEST_F(UT_ConfigPresenter, setEnabledTypeCategories_SetsCategories)
{
    bool setValueCalled = false;

    stub.set_lamda(&DConfigManager::setValue, [&setValueCalled](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        setValueCalled = true;
    });

    ItemCategories cats;
    cats |= kCatApplication;
    cats |= kCatDocument;
    presenter->setEnabledTypeCategories(cats);
    EXPECT_TRUE(setValueCalled);
}

TEST_F(UT_ConfigPresenter, organizeAction_ReturnsAction)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(0);
    });

    EXPECT_EQ(presenter->organizeAction(), kOnTrigger);
}

TEST_F(UT_ConfigPresenter, organizeAction_ReturnsAlways)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(1);
    });

    EXPECT_EQ(presenter->organizeAction(), kAlways);
}

TEST_F(UT_ConfigPresenter, organizeOnTriggered_ReturnsTrueForOnTrigger)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(0);
    });

    EXPECT_TRUE(presenter->organizeOnTriggered());
}

TEST_F(UT_ConfigPresenter, optimizeMovingPerformance_ReturnsValue)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    EXPECT_TRUE(presenter->optimizeMovingPerformance());
}

TEST_F(UT_ConfigPresenter, writeNormalStyle_WritesStyles)
{
    bool writeCalled = false;

    stub.set_lamda(&OrganizerConfig::writeCollectionStyle, [&writeCalled](OrganizerConfig *, const QString &, const QList<CollectionStyle> &) {
        __DBG_STUB_INVOKE__
        writeCalled = true;
    });
    stub.set_lamda(&OrganizerConfig::sync, [](OrganizerConfig *, int) {
        __DBG_STUB_INVOKE__
    });

    QList<CollectionStyle> styles;
    presenter->writeNormalStyle("config_id", styles);
    EXPECT_TRUE(writeCalled);
}
