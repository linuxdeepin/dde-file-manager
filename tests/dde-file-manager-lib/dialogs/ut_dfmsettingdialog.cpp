// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "interfaces/dfmapplication.h"
#include "stub.h"
#include <gtest/gtest.h>
#include <DSettingsOption>
#include <QVariant>

#define protected public
#include "dialogs/dfmsettingdialog.h"
#include "dialogs/private/dfmsettingdialog_p.h"

namespace  {
    class TestDFMSettingDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
//            QObject*(*stu_instance)() = []()->QObject*{
//                QObject * obj = new QObject();
//                return obj;
//            };
//            Stub stu2;
//            stu2.set(ADDR(DFMApplication, instance), stu_instance);
            m_pTesting = new DFMSettingDialog();
            std::cout << "start TestDFMSettingDialog";
        }
        void TearDown() override
        {
            delete  m_pTesting;
            m_pTesting = nullptr;
            std::cout << "end TestDFMSettingDialog";
        }
    public:
        DFMSettingDialog *m_pTesting;
    };
}

TEST_F(TestDFMSettingDialog, testInit)
{
    EXPECT_NE(nullptr, m_pTesting);
}

TEST_F(TestDFMSettingDialog, testInit2)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

//    QObject*(*stu_instance)() = []()->QObject*{
//        QObject * obj = new QObject();
//        return obj;
//    };
//    Stub stu2;
//    stu2.set(ADDR(DFMApplication, instance), stu_instance);

    DFMSettingDialog dlg;
    EXPECT_EQ(dlg.width(), 700);
}

TEST_F(TestDFMSettingDialog, testCreateAutoMountCheckBox)
{
    Dtk::Core::DSettingsOption opt;
    m_pTesting->createAutoMountCheckBox(&opt);
    emit opt.valueChanged("");
    emit DFMSettingDialog::AutoMountCheckBox->stateChanged(0);
    bool b = DFMSettingDialog::AutoMountOpenCheckBox->isEnabled();
    EXPECT_EQ(b, false);
}

TEST_F(TestDFMSettingDialog, testCreateAutoMountCheckBox2)
{
    Dtk::Core::DSettingsOption opt;
    m_pTesting->createAutoMountCheckBox(&opt);
    emit DFMSettingDialog::AutoMountCheckBox->stateChanged(2);
    bool b = DFMSettingDialog::AutoMountOpenCheckBox->isEnabled();
    EXPECT_EQ(b, true);
}

TEST_F(TestDFMSettingDialog, testCreateAutoMountOpenCheckBox)
{
    Dtk::Core::DSettingsOption opt;
    m_pTesting->createAutoMountOpenCheckBox(&opt);
    QString str = DFMSettingDialog::AutoMountOpenCheckBox->text();
    EXPECT_TRUE(str == "Open after auto mount");
}

TEST_F(TestDFMSettingDialog, testCreateAutoMountOpenCheckBox2)
{
    bool(*stub_toBool)() = []()->bool{
        return false;
    };
    Stub stu;
    stu.set(ADDR(QVariant, toBool), stub_toBool);

    QCheckBox box;
    DFMSettingDialog::AutoMountCheckBox = &box ;
    Dtk::Core::DSettingsOption opt;
    m_pTesting->createAutoMountOpenCheckBox(&opt);
    bool b = box.isEnabled();
    EXPECT_EQ(b, true);
}

TEST_F(TestDFMSettingDialog, testCreateAutoMountOpenCheckBox3)
{
    Dtk::Core::DSettingsOption opt;
    m_pTesting->createAutoMountOpenCheckBox(&opt);
    emit DFMSettingDialog::AutoMountOpenCheckBox->stateChanged(0);
    bool b = opt.value().toBool();
    EXPECT_EQ(false, b);
}

TEST_F(TestDFMSettingDialog, testCreateAutoMountOpenCheckBox4)
{
    Dtk::Core::DSettingsOption opt;
    m_pTesting->createAutoMountOpenCheckBox(&opt);
    emit opt.valueChanged("");
    emit DFMSettingDialog::AutoMountOpenCheckBox->stateChanged(2);
    bool b = opt.value().toBool();
    EXPECT_EQ(true, b);
}

namespace  {
    class TestSettingBackend : public testing::Test
    {
    public:
        void SetUp() override
        {
//            QObject*(*stu_instance)() = []()->QObject*{
//                QObject * obj = new QObject();
//                return obj;
//            };
//            Stub stu2;
//            stu2.set(ADDR(DFMApplication, instance), stu_instance);


            m_pTesting = new SettingBackend();
            std::cout << "start TestSettingBackend";
        }
        void TearDown() override
        {
            delete  m_pTesting;
            m_pTesting = nullptr;
            std::cout << "end TestSettingBackend";
        }
    public:
        SettingBackend *m_pTesting;

    };
}

TEST_F(TestSettingBackend, testDoSetOption)
{
    void(*stu_setAppAttribute)(DFMApplication::ApplicationAttribute aa, const QVariant &value) = [](DFMApplication::ApplicationAttribute aa, const QVariant &value)->void{
        int a = 0;
    };
    Stub stu2;
    stu2.set(ADDR(DFMApplication, setAppAttribute), stu_setAppAttribute);

    void(*stu_setGenericAttribute)(DFMApplication::GenericAttribute ga, const QVariant &value) = [](DFMApplication::GenericAttribute ga, const QVariant &value)->void{
        int a = 0;
    };
    Stub stu3;
    stu3.set(ADDR(DFMApplication, setGenericAttribute), stu_setGenericAttribute);

    QString key("base.open_action.open_file_action");
    QVariant value("");
    EXPECT_NO_FATAL_FAILURE(m_pTesting->doSetOption(key, value));
}

TEST_F(TestSettingBackend, testDoSetOption2)
{
    void(*stu_setAppAttribute)(DFMApplication::ApplicationAttribute aa, const QVariant &value) = [](DFMApplication::ApplicationAttribute aa, const QVariant &value)->void{
        int a = 0;
    };
    Stub stu2;
    stu2.set(ADDR(DFMApplication, setAppAttribute), stu_setAppAttribute);


    void(*stu_setGenericAttribute)(DFMApplication::GenericAttribute ga, const QVariant &value) = [](DFMApplication::GenericAttribute ga, const QVariant &value)->void{
        int a = 0;
    };
    Stub stu3;
    stu3.set(ADDR(DFMApplication, setGenericAttribute), stu_setGenericAttribute);

    QString key("base.hidden_files.show_suffix");
    QVariant value("");
    EXPECT_NO_FATAL_FAILURE(m_pTesting->doSetOption(key, value));
}

TEST_F(TestSettingBackend, testDoSetOption3)
{
    void(*stu_setAppAttribute)(DFMApplication::ApplicationAttribute aa, const QVariant &value) = [](DFMApplication::ApplicationAttribute aa, const QVariant &value)->void{
        int a = 0;
    };
    Stub stu2;
    stu2.set(ADDR(DFMApplication, setAppAttribute), stu_setAppAttribute);

    void(*stu_setGenericAttribute)(DFMApplication::GenericAttribute ga, const QVariant &value) = [](DFMApplication::GenericAttribute ga, const QVariant &value)->void{
        int a = 0;
    };
    Stub stu3;
    stu3.set(ADDR(DFMApplication, setGenericAttribute), stu_setGenericAttribute);

    QString key("advance.other.hide_system_partition");
    QVariant value("");
    EXPECT_NO_FATAL_FAILURE(m_pTesting->doSetOption(key, value));
}

TEST_F(TestSettingBackend, testOnValueChanged)
{
    int attribute(dde_file_manager::DFMApplication::GenericAttribute::GA_HiddenSystemPartition);
    QVariant value;
    EXPECT_NO_FATAL_FAILURE(m_pTesting->onValueChanged(attribute, value));
}

TEST_F(TestSettingBackend, testDoSync)
{
    EXPECT_NO_FATAL_FAILURE(m_pTesting->doSync());
}
