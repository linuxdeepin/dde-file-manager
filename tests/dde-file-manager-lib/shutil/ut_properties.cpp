/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "shutil/properties.h"
#include "shutil/fileutils.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <memory>

namespace  {
    class TestProperties : public testing::Test {
    public:
        void SetUp() override
        {
           mProperties.reset(new Properties("nothingfile"));
        }
        void TearDown() override
        {
        }

    public:
            std::shared_ptr<Properties> mProperties = nullptr;

            // return file folder
            QString getTestFolder()
            {
                return QString("%1/test_shutil/property").arg(QDir::currentPath());
            }

            void makeFolder(const QString& folderPath)
            {
                QDir dir;
                if (!dir.exists(folderPath))
                {
                    dir.mkpath(folderPath);
                }
                DUrl url = DUrl::fromLocalFile(folderPath);
                FileUtils::mkpath(url);
            }

            // return file path
            QString createOneFile(const QString& filename, const QString& folderPath)
            {
                makeFolder(folderPath);

                QString filePath = QString("%1/%2").arg(folderPath).arg(filename);

                QString content= "";
                if(!FileUtils::isFileExists(filePath)){
                    FileUtils::writeTextFile(filePath,content);
                }
                return filePath;
            }

            // return file path
            QString createDefaultFile(const QString& filename)
            {
                return createOneFile(filename, getTestFolder());
            }
    };
}

TEST_F(TestProperties, can_set_values_and_save_load_values)
{
    QString filePath = createDefaultFile("oneProperty.pro");

    QMap<QString, QString> keyValueMap;

    keyValueMap["pro1"] = "value1";
    keyValueMap["pro2"] = "value2";
    keyValueMap["pro3"] = "value3";
    keyValueMap["pro4"] = "value4";
    keyValueMap["pro5"] = "value5";

    QMap<QString, QString>::iterator ite = keyValueMap.begin();
    for (; ite != keyValueMap.end(); ++ite) {
        mProperties->set(ite.key(), ite.value() );
    }

    mProperties->set("pro4", "value4" );

    EXPECT_TRUE( mProperties->getKeys().contains("pro1") );
    EXPECT_TRUE( mProperties->contains("pro2") );
    EXPECT_EQ("value3", mProperties->value("pro3").toString() );

    EXPECT_TRUE(mProperties->save(filePath));
    EXPECT_TRUE(mProperties->load(filePath));

    EXPECT_TRUE( mProperties->getKeys().contains("pro1") );
    EXPECT_TRUE( mProperties->contains("pro2") );
    EXPECT_EQ("value3", mProperties->value("pro3").toString() );
}
