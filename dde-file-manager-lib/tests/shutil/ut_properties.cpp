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
