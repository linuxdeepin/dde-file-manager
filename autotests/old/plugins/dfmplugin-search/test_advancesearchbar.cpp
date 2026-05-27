// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfm-base/utils/sortutils.h"
#include "topwidget/advancesearchbar.h"
#include "topwidget/advancesearchbar_p.h"
#include "utils/searchhelper.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/private/syncfileinfo_p.h>

#include "stubext.h"

#include <dfm-framework/event/event.h>

#include <QHideEvent>
#include <DComboBox>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(AdvanceSearchBarTest, ut_resetForm)
{
    stub_ext::StubExt st;
    st.set_lamda(&AdvanceSearchBar::onOptionChanged, [] {});

    AdvanceSearchBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.resetForm());
}

TEST(AdvanceSearchBarTest, ut_refreshOptions)
{
    stub_ext::StubExt st;
    st.set_lamda(&AdvanceSearchBarPrivate::refreshOptions, [] {});

    AdvanceSearchBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.refreshOptions(QUrl()));
}

TEST(AdvanceSearchBarTest, ut_onOptionChanged)
{
    AdvanceSearchBar bar;
    stub_ext::StubExt st;
    FileManagerWindow window(QUrl::fromLocalFile("/home"));
    st.set_lamda(&FileManagerWindowsManager::findWindowId, [] { return 123; });
    st.set_lamda(&FileManagerWindowsManager::findWindowById, [&window] { return &window; });
    st.set_lamda(&FileManagerWindow::currentUrl, [] { return QUrl::fromLocalFile("/home"); });

    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64, const QUrl &, QVariant &&);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(bar.onOptionChanged());
}

TEST(AdvanceSearchBarTest, ut_onResetButtonPressed)
{
    stub_ext::StubExt st;
    st.set_lamda(&AdvanceSearchBar::resetForm, [] {});

    AdvanceSearchBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.onResetButtonPressed());
}

TEST(AdvanceSearchBarTest, ut_hideEvent)
{
    stub_ext::StubExt st;
    st.set_lamda(&FileManagerWindowsManager::findWindowId, [] { return 123; });
    FileManagerWindow window(QUrl::fromLocalFile("/home"));
    st.set_lamda(&FileManagerWindowsManager::findWindowById, [&window] { return &window; });
    st.set_lamda(&FileManagerWindow::isMinimized, [] { return false; });
    st.set_lamda(&AdvanceSearchBar::resetForm, [] {});
    st.set_lamda(VADDR(QScrollArea, hideEvent), [] {});

    AdvanceSearchBar bar;
    QHideEvent event;
    EXPECT_NO_FATAL_FAILURE(bar.hideEvent(&event));
}

TEST(AdvanceSearchBarPrivateTest, ut_refreshOptions_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&AdvanceSearchBar::resetForm, [] {});

    AdvanceSearchBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.d->refreshOptions(QUrl()));
}

TEST(AdvanceSearchBarPrivateTest, ut_refreshOptions_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&AdvanceSearchBar::onOptionChanged, [] {});

    AdvanceSearchBar bar;
    auto searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kSearchRange]->currentData();
    formData[AdvanceSearchBarPrivate::kFileType] = bar.d->asbCombos[AdvanceSearchBarPrivate::kFileType]->currentData();
    formData[AdvanceSearchBarPrivate::kSizeRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kSizeRange]->currentData();
    formData[AdvanceSearchBarPrivate::kDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kDateRange]->currentData();
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kAccessDateRange]->currentData();
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kCreateDateRange]->currentData();
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = searchUrl;
    bar.d->filterInfoCache[searchUrl] = formData;

    EXPECT_NO_FATAL_FAILURE(bar.d->refreshOptions(searchUrl));
}

TEST(AdvanceSearchBarPrivateTest, ut_refreshOptions_3)
{
    stub_ext::StubExt st;
    st.set_lamda(&AdvanceSearchBar::onOptionChanged, [] {});

    AdvanceSearchBar bar;
    auto searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kSearchRange]->itemData(1);
    formData[AdvanceSearchBarPrivate::kFileType] = bar.d->asbCombos[AdvanceSearchBarPrivate::kFileType]->itemData(1);
    formData[AdvanceSearchBarPrivate::kSizeRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kSizeRange]->itemData(1);
    formData[AdvanceSearchBarPrivate::kDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kDateRange]->itemData(1);
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kAccessDateRange]->itemData(1);
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kCreateDateRange]->itemData(1);
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = searchUrl;
    bar.d->filterInfoCache[searchUrl] = formData;

    EXPECT_NO_FATAL_FAILURE(bar.d->refreshOptions(searchUrl));
}

TEST(AdvanceSearchBarPrivateTest, ut_shouldVisiableByFilterRule_1)
{
    AdvanceSearchBar bar;
    EXPECT_TRUE(bar.d->shouldVisiableByFilterRule(nullptr, QVariant()));

    QMap<int, QVariant> formData;
    auto searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    formData[AdvanceSearchBarPrivate::kSearchRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kSearchRange]->currentData();
    formData[AdvanceSearchBarPrivate::kFileType] = bar.d->asbCombos[AdvanceSearchBarPrivate::kFileType]->currentData();
    formData[AdvanceSearchBarPrivate::kSizeRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kSizeRange]->currentData();
    formData[AdvanceSearchBarPrivate::kDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kDateRange]->currentData();
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kAccessDateRange]->currentData();
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = bar.d->asbCombos[AdvanceSearchBarPrivate::kCreateDateRange]->currentData();
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = searchUrl;
    EXPECT_TRUE(bar.d->shouldVisiableByFilterRule(nullptr, QVariant::fromValue(formData)));

    formData[AdvanceSearchBarPrivate::kSearchRange] = false;
    EXPECT_FALSE(bar.d->shouldVisiableByFilterRule(nullptr, QVariant::fromValue(formData)));
}

TEST(AdvanceSearchBarPrivateTest, ut_shouldVisiableByFilterRule_2)
{
    auto searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    stub_ext::StubExt st;
    typedef bool (QFileInfo::*Exists)() const;
    auto exists = static_cast<Exists>(&QFileInfo::exists);
    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&AdvanceSearchBarPrivate::parseFilterData, [&searchUrl] {
        AdvanceSearchBarPrivate::FileFilter filter;
        filter.comboValid[AdvanceSearchBarPrivate::kSearchRange] = true;
        filter.includeSubDir = false;
        filter.currentUrl = searchUrl;
        return filter;
    });

    AdvanceSearchBar bar;
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = false;
    formData[AdvanceSearchBarPrivate::kFileType] = QVariant();
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = searchUrl;

    SortFileInfo info;
    info.setUrl(QUrl::fromLocalFile("/home/test/test"));
    EXPECT_FALSE(bar.d->shouldVisiableByFilterRule(&info, QVariant::fromValue(formData)));
}

TEST(AdvanceSearchBarPrivateTest, ut_shouldVisiableByFilterRule_3)
{
    stub_ext::StubExt st;
    typedef bool (QFileInfo::*Exists)() const;
    auto exists = static_cast<Exists>(&QFileInfo::exists);
    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(SortUtils::accurateDisplayType, [] { return "test/test"; });
    st.set_lamda(&AdvanceSearchBarPrivate::parseFilterData, [] {
        AdvanceSearchBarPrivate::FileFilter filter;
        filter.comboValid[AdvanceSearchBarPrivate::kFileType] = true;
        filter.typeString = "audio";
        return filter;
    });

    AdvanceSearchBar bar;
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = true;
    formData[AdvanceSearchBarPrivate::kFileType] = "test";
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = QUrl();

    SortFileInfo info;
    info.setUrl(QUrl::fromLocalFile("/home/test/test"));
    EXPECT_FALSE(bar.d->shouldVisiableByFilterRule(&info, QVariant::fromValue(formData)));
}

TEST(AdvanceSearchBarPrivateTest, ut_shouldVisiableByFilterRule_4)
{
    stub_ext::StubExt st;
    typedef bool (QFileInfo::*Exists)() const;
    auto exists = static_cast<Exists>(&QFileInfo::exists);
    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&SortFileInfo::fileSize, [] { return 10; });
    st.set_lamda(&AdvanceSearchBarPrivate::parseFilterData, [] {
        AdvanceSearchBarPrivate::FileFilter filter;
        filter.comboValid[AdvanceSearchBarPrivate::kFileType] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kSizeRange] = true;
        filter.sizeRange = QPair<quint64, quint64>(100, 1024);
        return filter;
    });

    AdvanceSearchBar bar;
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = true;
    formData[AdvanceSearchBarPrivate::kFileType] = QVariant();
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant::fromValue(QPair<quint64, quint64>(100, 1024));
    formData[AdvanceSearchBarPrivate::kDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = QUrl();

    SortFileInfo info;
    info.setUrl(QUrl::fromLocalFile("/home/test/test"));
    EXPECT_FALSE(bar.d->shouldVisiableByFilterRule(&info, QVariant::fromValue(formData)));
}

TEST(AdvanceSearchBarPrivateTest, ut_shouldVisiableByFilterRule_5)
{
    stub_ext::StubExt st;
    typedef bool (QFileInfo::*Exists)() const;
    auto exists = static_cast<Exists>(&QFileInfo::exists);
    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&SortFileInfo::lastModifiedTime, [] { return QDateTime::currentDateTime().toSecsSinceEpoch(); });
    st.set_lamda(&AdvanceSearchBarPrivate::parseFilterData, [] {
        AdvanceSearchBarPrivate::FileFilter filter;
        filter.comboValid[AdvanceSearchBarPrivate::kFileType] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kSizeRange] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kDateRange] = true;

        QDate today = QDate::currentDate();
        filter.dateRangeStart = QDateTime(QDate(today.year(), 1, 1), {}).addYears(-1);
        filter.dateRangeEnd = QDateTime(QDate(today.year(), 1, 1), {});
        return filter;
    });

    AdvanceSearchBar bar;
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = true;
    formData[AdvanceSearchBarPrivate::kFileType] = QVariant();
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kDateRange] = 730;
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = QUrl();

    SortFileInfo info;
    info.setUrl(QUrl::fromLocalFile("/home/test/test"));
    EXPECT_FALSE(bar.d->shouldVisiableByFilterRule(&info, QVariant::fromValue(formData)));
}

TEST(AdvanceSearchBarPrivateTest, ut_shouldVisiableByFilterRule_6)
{
    stub_ext::StubExt st;
    typedef bool (QFileInfo::*Exists)() const;
    auto exists = static_cast<Exists>(&QFileInfo::exists);
    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&SortFileInfo::lastReadTime, [] { return QDateTime::currentDateTime().toSecsSinceEpoch(); });
    st.set_lamda(&AdvanceSearchBarPrivate::parseFilterData, [] {
        AdvanceSearchBarPrivate::FileFilter filter;
        filter.comboValid[AdvanceSearchBarPrivate::kFileType] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kSizeRange] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kDateRange] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kAccessDateRange] = true;

        QDate today = QDate::currentDate();
        filter.accessDateRangeStart = QDateTime(QDate(today.year(), 1, 1), {}).addYears(-1);
        filter.accessDateRangeEnd = QDateTime(QDate(today.year(), 1, 1), {});
        return filter;
    });

    AdvanceSearchBar bar;
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = true;
    formData[AdvanceSearchBarPrivate::kFileType] = QVariant();
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = 730;
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = QUrl();

    SortFileInfo info;
    info.setUrl(QUrl::fromLocalFile("/home/test/test"));
    EXPECT_FALSE(bar.d->shouldVisiableByFilterRule(&info, QVariant::fromValue(formData)));
}

TEST(AdvanceSearchBarPrivateTest, ut_shouldVisiableByFilterRule_7)
{
    stub_ext::StubExt st;
    typedef bool (QFileInfo::*Exists)() const;
    auto exists = static_cast<Exists>(&QFileInfo::exists);
    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&SortFileInfo::createTime, [] { return QDateTime::currentDateTime().toSecsSinceEpoch(); });
    st.set_lamda(&AdvanceSearchBarPrivate::parseFilterData, [] {
        AdvanceSearchBarPrivate::FileFilter filter;
        filter.comboValid[AdvanceSearchBarPrivate::kFileType] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kSizeRange] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kDateRange] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kAccessDateRange] = false;
        filter.comboValid[AdvanceSearchBarPrivate::kCreateDateRange] = true;

        QDate today = QDate::currentDate();
        filter.createDateRangeStart = QDateTime(QDate(today.year(), 1, 1), {}).addYears(-1);
        filter.createDateRangeEnd = QDateTime(QDate(today.year(), 1, 1), {});
        return filter;
    });

    AdvanceSearchBar bar;
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = true;
    formData[AdvanceSearchBarPrivate::kFileType] = QVariant();
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = QVariant();
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = 730;
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = QUrl();

    SortFileInfo info;
    info.setUrl(QUrl::fromLocalFile("/home/test/test"));
    EXPECT_FALSE(bar.d->shouldVisiableByFilterRule(&info, QVariant::fromValue(formData)));
}

TEST(AdvanceSearchBarPrivateTest, ut_parseFilterData_1)
{
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = true;
    formData[AdvanceSearchBarPrivate::kFileType] = "test";
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant::fromValue(QPair<quint64, quint64>(100, 1024));
    formData[AdvanceSearchBarPrivate::kDateRange] = 1;
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = 2;
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = 7;
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = QUrl();

    AdvanceSearchBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.d->parseFilterData(formData));
}

TEST(AdvanceSearchBarPrivateTest, ut_parseFilterData_2)
{
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = true;
    formData[AdvanceSearchBarPrivate::kFileType] = "test";
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant::fromValue(QPair<quint64, quint64>(100, 1024));
    formData[AdvanceSearchBarPrivate::kDateRange] = 14;
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = 30;
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = 60;
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = QUrl();

    AdvanceSearchBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.d->parseFilterData(formData));
}

TEST(AdvanceSearchBarPrivateTest, ut_parseFilterData_3)
{
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = true;
    formData[AdvanceSearchBarPrivate::kFileType] = "test";
    formData[AdvanceSearchBarPrivate::kSizeRange] = QVariant::fromValue(QPair<quint64, quint64>(100, 1024));
    formData[AdvanceSearchBarPrivate::kDateRange] = 0;
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = 365;
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = 730;
    formData[AdvanceSearchBarPrivate::kCurrentUrl] = QUrl();

    AdvanceSearchBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.d->parseFilterData(formData));
}
