// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/utils/searchhelper.cpp
// (uncovered functions -> test case)
//   SearchHelper::viewModelUrl(QUrl const&) ..................... ViewModelUrl_* (2 cases)
//   SearchHelper::createCheckBoxWithTextIndex(QObject*) ......... Factory_TextIndex_BuildsBoundWidget
//   createCheckBoxWithTextIndex {lambda(Qt::CheckState)#1} ...... Factory_TextIndex_BuildsBoundWidget (via setChecked)
//   SearchHelper::createCheckBoxWithFileIndex(QObject*) ......... Factory_FileIndex_BuildsBoundWidget
//   createCheckBoxWithFileIndex {lambda#1} ...................... Factory_FileIndex_BuildsBoundWidget
//   SearchHelper::createCheckBoxWithOcrIndex(QObject*) .......... Factory_OcrIndex_BuildsBoundWidget
//   createCheckBoxWithOcrIndex {lambda#1} ....................... Factory_OcrIndex_BuildsBoundWidget
//   SearchHelper::createCheckBoxWithSemanticIndex(QObject*) ..... Factory_SemanticIndex_BuildsBoundWidget
//   createCheckBoxWithSemanticIndex {lambda#1} .................. Factory_SemanticIndex_BuildsBoundWidget
//   QMetaTypeId<QString*>::qt_metatype_id() (searchhelper.cpp L32) .. exercised via signal/slot registration in this TU
// The DSettingsOption argument is built from a real Dtk DSettings json tree.
// ============================================================================

#include <gtest/gtest.h>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>
#include <QByteArray>
#include <QWidget>
#include <QCheckBox>
#include <QPointer>

#include <DSettings>
#include <DSettingsOption>

#include "utils/searchhelper.h"
#include "utils/checkboxwithfileindex.h"
#include "utils/checkboxwithtextindex.h"
#include "utils/checkboxwithocrindex.h"
#include "utils/checkboxwithsemanticindex.h"

using namespace dfmplugin_search;

namespace {

const char kSettingsJson[] = R"json({
    "groups": [{
        "key": "search",
        "options": [{
            "key": "toggle",
            "type": "checkbox",
            "text": "Caption",
            "default": true
        }]
    }]
})json";

Dtk::Core::DSettingsOption *makeOption(bool defaultValue)
{
    auto settings = Dtk::Core::DSettings::fromJson(QByteArray(kSettingsJson));   // QPointer<DSettings>
    auto option = settings->option(QStringLiteral("search.toggle"));   // QPointer<DSettingsOption>
    option->setValue(defaultValue);
    return option;
}

}   // namespace

class UT_SearchHelperCov : public testing::Test
{
protected:
    void TearDown() override
    {
        qDeleteAll(widgets);
        widgets.clear();
        qDeleteAll(options);
        options.clear();
    }

    template<typename T>
    T *track(T *widget)
    {
        widgets.append(widget);
        return widget;
    }

    QList<QWidget *> widgets;
    QList<QPointer<Dtk::Core::DSettingsOption>> options;
};

// ---------- viewModelUrl ----------

TEST_F(UT_SearchHelperCov, ViewModelUrl_SearchUrlWithExtraParams_KeepsOnlyUrlParam)
{
    // Arrange
    QUrl url = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home/user/docs"), QStringLiteral("kw"), QStringLiteral("7"));

    // Act
    QUrl viewUrl = SearchHelper::viewModelUrl(url);

    // Assert
    QUrlQuery query(viewUrl.query());
    EXPECT_EQ(query.queryItemValue("url"), QString("file:///home/user/docs"));
    EXPECT_TRUE(query.queryItemValue("keyword").isEmpty());   // dropped
    EXPECT_TRUE(query.queryItemValue("winId").isEmpty());
}

TEST_F(UT_SearchHelperCov, ViewModelUrl_NonSearchScheme_ReturnsUnchanged)
{
    // Arrange
    QUrl url = QUrl::fromLocalFile("/home/user/plain");

    // Act
    QUrl viewUrl = SearchHelper::viewModelUrl(url);

    // Assert
    EXPECT_EQ(viewUrl, url);
    EXPECT_FALSE(viewUrl.hasQuery());
}

// ---------- settings widget factories ----------

TEST_F(UT_SearchHelperCov, Factory_TextIndex_BuildsBoundWidget)
{
    // Arrange
    auto *option = makeOption(true);
    options.append(option);

    // Act
    QWidget *widget = SearchHelper::createCheckBoxWithTextIndex(option);
    track(widget);

    // Assert
    auto *box = qobject_cast<CheckBoxWithTextIndex *>(widget);
    ASSERT_NE(box, nullptr);
    EXPECT_TRUE(box->isChecked());
    auto *checkBox = box->findChild<QCheckBox *>(QStringLiteral("CheckBox"));
    ASSERT_NE(checkBox, nullptr);
    EXPECT_EQ(checkBox->text(), QString("Caption"));

    // the checkStateChanged lambda writes back into the option
    box->setChecked(false);
    EXPECT_FALSE(option->value().toBool());
    box->setChecked(true);
    EXPECT_TRUE(option->value().toBool());
}

TEST_F(UT_SearchHelperCov, Factory_FileIndex_BuildsBoundWidget)
{
    // Arrange
    auto *option = makeOption(false);
    options.append(option);

    // Act
    QWidget *widget = SearchHelper::createCheckBoxWithFileIndex(option);
    track(widget);

    // Assert
    auto *box = qobject_cast<CheckBoxWithFileIndex *>(widget);
    ASSERT_NE(box, nullptr);
    EXPECT_EQ(box->findChild<QCheckBox *>(QStringLiteral("CheckBox"))->text(), QString("Caption"));
    EXPECT_FALSE(box->isChecked());

    box->setChecked(true);
    EXPECT_TRUE(option->value().toBool());   // lambda wrote Checked back
}

TEST_F(UT_SearchHelperCov, Factory_OcrIndex_BuildsBoundWidget)
{
    // Arrange
    auto *option = makeOption(false);
    options.append(option);

    // Act
    QWidget *widget = SearchHelper::createCheckBoxWithOcrIndex(option);
    track(widget);

    // Assert
    auto *box = qobject_cast<CheckBoxWithOcrIndex *>(widget);
    ASSERT_NE(box, nullptr);
    EXPECT_EQ(box->findChild<QCheckBox *>(QStringLiteral("CheckBox"))->text(), QString("Caption"));
    EXPECT_FALSE(box->isChecked());

    box->setChecked(true);
    EXPECT_TRUE(option->value().toBool());
}

TEST_F(UT_SearchHelperCov, Factory_SemanticIndex_BuildsBoundWidget)
{
    // Arrange
    auto *option = makeOption(true);
    options.append(option);

    // Act
    QWidget *widget = SearchHelper::createCheckBoxWithSemanticIndex(option);
    track(widget);

    // Assert
    auto *box = qobject_cast<CheckBoxWithSemanticIndex *>(widget);
    ASSERT_NE(box, nullptr);
    EXPECT_EQ(box->findChild<QCheckBox *>(QStringLiteral("CheckBox"))->text(), QString("Caption"));
    EXPECT_TRUE(box->isChecked());

    box->setChecked(false);
    EXPECT_FALSE(option->value().toBool());   // lambda wrote Unchecked back
}
