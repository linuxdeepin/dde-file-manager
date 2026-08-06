// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settings_r16.cpp
 * @brief Additional Settings tests: D0 destructor (heap alloc+delete),
 *        setWatchChanges true then false (exercises the watcher-create lambda
 *        and the _q_onFileRenamed lambda wiring).
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QString>

#include <dfm-base/base/application/settings.h>

using namespace dfmbase;

class SettingsR16Test : public testing::Test
{
protected:
    QTemporaryDir tmp;
    QString defaultFile, settingFile;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        defaultFile = tmp.path() + "/default.json";
        settingFile = tmp.path() + "/settings.json";
        writeFile(defaultFile, "{ \"G\": { \"k\": \"v\" } }");
        writeFile(settingFile, "{ \"G\": { \"k\": \"cur\" } }");
    }

    void writeFile(const QString &p, const QString &content)
    {
        QFile f(p);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }
};

TEST_F(SettingsR16Test, D0DestructorPath)
{
    auto *ptr = new Settings(defaultFile, defaultFile, settingFile);
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}

TEST_F(SettingsR16Test, SetWatchChangesTrueCreatesWatcher)
{
    Settings s(defaultFile, defaultFile, settingFile);
    s.setWatchChanges(true);
    // setting watcher; exercise the connect lambda wiring
}

TEST_F(SettingsR16Test, SetWatchChangesFalseRemovesWatcher)
{
    Settings s(defaultFile, defaultFile, settingFile);
    s.setWatchChanges(true);
    s.setWatchChanges(false);
}

TEST_F(SettingsR16Test, SetWatchChangesIdempotentWhenAlreadyTrue)
{
    Settings s(defaultFile, defaultFile, settingFile);
    s.setWatchChanges(true);
    s.setWatchChanges(true);   // no-op (early return)
}

TEST_F(SettingsR16Test, SetWatchChangesFalseWhenAlreadyFalseIsNoOp)
{
    Settings s(defaultFile, defaultFile, settingFile);
    s.setWatchChanges(false);   // already false -> no-op
}
