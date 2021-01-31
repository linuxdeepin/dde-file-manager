#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QStandardPaths>
#include <QProcess>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QUuid>

#define private public
#include "config/config.h"

using namespace testing;

TEST(configtest, removeConfigList)
{
    QString desktoppath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QUuid  id = QUuid::createUuid();
    QString filename = id.toString() + ".ini";

    QFile file(desktoppath + '/' + filename);
    if (file.exists() || !file.open(QIODevice::NewOnly | QIODevice::ReadWrite)) {
        return;
    }

    QSettings settting(desktoppath + '/' + filename, QSettings::IniFormat);
    //增加节点内容
    settting.beginGroup("person");
    settting.setValue("name", "xiaoc");
    settting.setValue("sex", "man");
    settting.endGroup();
    Config* conf = Config::instance();
    QSettings* oldsetting = conf->m_settings;
    conf->m_settings = &settting;
    conf->removeConfigList("person", QStringList() << "name" << "sex");

    EXPECT_FALSE(conf->m_settings->contains("name"));
    EXPECT_FALSE(conf->m_settings->contains("sex"));

    file.close();
    QDir dir(desktoppath);
    dir.remove(filename);
    conf->m_settings = oldsetting;
}
