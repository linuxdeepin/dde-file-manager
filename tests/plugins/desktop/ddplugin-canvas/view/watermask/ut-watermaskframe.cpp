// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "watermask/deepinlicensehelper.h"
#include "watermask/watermaskframe.h"
#include "stubext.h"

#include <QLabel>
#include <QLayout>
#include <QApplication>
#include <QDir>
#include <QSaveFile>

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_canvas;

TEST(WaterMaskFrame, loadConfig_empty)
{
    WaterMaskFrame wid("");
    EXPECT_TRUE(wid.configFile.isEmpty());
    wid.loadConfig();

    EXPECT_TRUE(wid.configInfos.isEmpty());

    qApp->processEvents();
}

TEST(WaterMaskFrame, loadConfig_existed)
{
    const QString dirPath("/tmp/ut-dde-desktop/watermask");
    QDir dir(dirPath);
    dir.mkpath(dirPath);
    const QString filePath(dir.filePath("test.xxx"));
    QSaveFile file(filePath);
    file.open(QSaveFile::WriteOnly);
    file.commit();
    WaterMaskFrame wid(filePath);

    EXPECT_EQ(wid.configFile, filePath);
    wid.loadConfig();

    ASSERT_EQ(wid.configInfos.size(), 1);

    auto cfg = wid.configInfos.first();
    EXPECT_FALSE(cfg.valid);
}

TEST(WaterMaskFrame, usingcn)
{
    stub_ext::StubExt stub;
    QLocale l;
    stub.set_lamda(ADDR(QLocale, system), [&l](){
        return l;
    });

    l = QLocale("zh_CN");
    EXPECT_TRUE(WaterMaskFrame::usingCn());

    l = QLocale("zh_TW");
    EXPECT_TRUE(WaterMaskFrame::usingCn());

    l = QLocale("zh_HK");
    EXPECT_TRUE(WaterMaskFrame::usingCn());

    l = QLocale("ug_CN");
    EXPECT_TRUE(WaterMaskFrame::usingCn());

    l = QLocale("bo_CN");
    EXPECT_TRUE(WaterMaskFrame::usingCn());

    l = QLocale("en_US");
    EXPECT_FALSE(WaterMaskFrame::usingCn());

    l = QLocale("en_GB");
    EXPECT_FALSE(WaterMaskFrame::usingCn());
}

using namespace testing;
namespace  {
    class WaterMaskFrameTest : public Test
    {
    public:
        WaterMaskFrameTest() : Test() {}

        virtual void SetUp() override {
            if (isValid())
                p = new WaterMaskFrame(path);
        }

        virtual void TearDown() override {
            delete  p;
        }
        bool isValid() {
            QFile file(path);
            if (file.open(QFile::ReadOnly)) {
                QString datas = file.readAll();
                file.close();
                return datas.contains("maskLogoUri")
                        && datas.contains("maskLogoGovernmentCnUri")
                        && datas.contains("maskLogoGovernmentEnUri")
                        && datas.contains("maskLogoEnterpriseCnUri")
                        && datas.contains("maskLogoEnterpriseEnUri");
            }
            return false;
        }

        const QString path = "/usr/share/deepin/dde-desktop-watermask.json";
        WaterMaskFrame *p = nullptr;
    };
}

TEST_F(WaterMaskFrameTest, loadConfig_system)
{
    if (!p)
        return;
    p->loadConfig();

    EXPECT_TRUE(p->configInfos.contains("default"));
    WaterMaskFrame::ConfigInfo cfg = p->configInfos.value("default");
    EXPECT_EQ(cfg.maskLogoUri.isEmpty(), !cfg.valid);

    EXPECT_TRUE(p->configInfos.contains("gov-en"));
    cfg = p->configInfos.value("gov-en");
    EXPECT_EQ(cfg.maskLogoUri.isEmpty(), !cfg.valid);

    EXPECT_TRUE(p->configInfos.contains("gov-cn"));
    cfg = p->configInfos.value("gov-cn");
    EXPECT_EQ(cfg.maskLogoUri.isEmpty(), !cfg.valid);

    EXPECT_TRUE(p->configInfos.contains("ent-en"));
    cfg = p->configInfos.value("ent-en");
    EXPECT_EQ(cfg.maskLogoUri.isEmpty(), !cfg.valid);

    EXPECT_TRUE(p->configInfos.contains("ent-cn"));
    cfg = p->configInfos.value("ent-cn");
    EXPECT_EQ(cfg.maskLogoUri.isEmpty(), !cfg.valid);
}

TEST_F(WaterMaskFrameTest, update)
{
    if (!p)
        return;

    WaterMaskFrame::ConfigInfo cfg;
    cfg.maskLogoUri = "/usr/share/deepin/uos_logo.svg";
    cfg.valid = true;

    p->textLabel->setText("text");

    p->update(cfg, true);

    EXPECT_TRUE(p->textLabel->text().isEmpty());
}


TEST_F(WaterMaskFrameTest, stateChanged)
{
    if (!p)
        return;

    stub_ext::StubExt stub;
    WaterMaskFrame::ConfigInfo cfg;
    bool normal = false;;
    stub.set_lamda(ADDR(WaterMaskFrame, update), [&cfg, &normal](WaterMaskFrame *self, const WaterMaskFrame::ConfigInfo &c, bool nor){
        normal = nor;
        cfg = c;
    });

    bool show = true;
    stub.set_lamda(ADDR(WaterMaskFrame, showLicenseState), [&show](){
        return show;
    });

    bool cn = true;
    stub.set_lamda(ADDR(WaterMaskFrame, usingCn), [&cn](){
        return cn;
    });

    p->textLabel->clear();
    p->stateChanged(0, 1);
    EXPECT_EQ(p->curState, 0);
    EXPECT_EQ(p->curProperty, 1);
    EXPECT_TRUE(normal);
    EXPECT_TRUE(cfg.maskLogoUri.isEmpty());
    EXPECT_EQ(p->textLabel->text(), "Not authorized");

    {
        WaterMaskFrame::ConfigInfo c;
        c.valid = true;
        c.maskLogoUri = "1";
        p->configInfos.insert("default", c);
    }

    p->stateChanged(1, 1);
    EXPECT_EQ(p->curState, 1);
    EXPECT_EQ(p->curProperty, 1);
    EXPECT_TRUE(normal);
    EXPECT_EQ(cfg.maskLogoUri, "1");
    EXPECT_TRUE(p->textLabel->text().isEmpty());

    {
        WaterMaskFrame::ConfigInfo c;
        c.valid = true;
        c.maskLogoUri = "gov-cn";
        p->configInfos.insert("gov-cn", c);
    }

    p->stateChanged(1, 1);
    EXPECT_EQ(p->curState, 1);
    EXPECT_EQ(p->curProperty, 1);
    EXPECT_FALSE(normal);
    EXPECT_EQ(cfg.maskLogoUri, "gov-cn");

    p->stateChanged(1, 2);
    EXPECT_EQ(p->curState, 1);
    EXPECT_EQ(p->curProperty, 2);
    EXPECT_TRUE(normal);
    EXPECT_EQ(cfg.maskLogoUri, "1");

    {
        WaterMaskFrame::ConfigInfo c;
        c.valid = true;
        c.maskLogoUri = "ent-cn";
        p->configInfos.insert("ent-cn", c);
    }

    p->stateChanged(1, 2);
    EXPECT_EQ(p->curState, 1);
    EXPECT_EQ(p->curProperty, 2);
    EXPECT_FALSE(normal);
    EXPECT_EQ(cfg.maskLogoUri, "ent-cn");

    // en
    cn = false;

    p->stateChanged(1, 1);
    EXPECT_EQ(p->curState, 1);
    EXPECT_EQ(p->curProperty, 1);
    EXPECT_TRUE(normal);
    EXPECT_EQ(cfg.maskLogoUri, "1");

    {
        WaterMaskFrame::ConfigInfo c;
        c.valid = true;
        c.maskLogoUri = "gov-en";
        p->configInfos.insert("gov-en", c);
    }

    p->stateChanged(1, 1);
    EXPECT_EQ(p->curState, 1);
    EXPECT_EQ(p->curProperty, 1);
    EXPECT_FALSE(normal);
    EXPECT_EQ(cfg.maskLogoUri, "gov-en");

    p->stateChanged(1, 2);
    EXPECT_EQ(p->curState, 1);
    EXPECT_EQ(p->curProperty, 2);
    EXPECT_TRUE(normal);
    EXPECT_EQ(cfg.maskLogoUri, "1");

    {
        WaterMaskFrame::ConfigInfo c;
        c.valid = true;
        c.maskLogoUri = "ent-en";
        p->configInfos.insert("ent-en", c);
    }

    p->stateChanged(1, 2);
    EXPECT_EQ(p->curState, 1);
    EXPECT_EQ(p->curProperty, 2);
    EXPECT_FALSE(normal);
    EXPECT_EQ(cfg.maskLogoUri, "ent-en");

    //
    show = false;
    p->textLabel->clear();
    p->stateChanged(0, 0);
    EXPECT_TRUE(p->textLabel->text().isEmpty());

    p->stateChanged(2, 0);
    EXPECT_TRUE(p->textLabel->text().isEmpty());

    p->stateChanged(4, 0);
    EXPECT_TRUE(p->textLabel->text().isEmpty());

    show = true;
    p->stateChanged(3, 0);
    EXPECT_EQ(p->textLabel->text(), "In trial period");
}
