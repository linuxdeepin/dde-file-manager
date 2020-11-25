#include <gtest/gtest.h>

#define private public
#define protected public
#include "dcustomactionparser.h"
#include "dcustomactiondefine.h"
#include "dcustomactiondata.h"


TEST(DCustomActionParser, action_name_dynamic_arg_empty)
{
    DCustomActionParser parser;
    DCustomActionData actData;

    actData.m_name = "n";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "n %f";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "name";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "name%f";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "name %F";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"n\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"n %F\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"name\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"name%f\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"name %f\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

}

TEST(DCustomActionParser, action_name_dynamic_arg_not_empty)
{
    DCustomActionParser parser;
    DCustomActionData actData;

    actData.m_name = "%a";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "n%a";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "%aname";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "na%ame";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "name%a";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "name %a";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"n%d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "\"%dname\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "\"na%dme\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "\"name%d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "\"name %d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "n%a%b";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "%a%bname";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "na%a%bme";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "name%a%b";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "name %a%b";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"n%a%b\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"%a%bname\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"na%a%bme\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"name%a%b\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"name %a%b\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "%ana%bme%d";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "%ana%bme %d";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"%ana%bme%d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"%ana%bme %d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);
}

TEST(DCustomActionParser, exec_dynamic_arg_empty)
{
    DCustomActionParser parser;
    DCustomActionData actData;

    actData.m_name = "n";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "%a/home/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "%a/home/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home%a/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home/test%a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home/test %a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home/test/%a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home/test/ %a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"n\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"%a/home/test\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home%a/test\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test%a\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test %a\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test\"%a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test\" %a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test/%a\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test/ %a\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

}

TEST(DCustomActionParser, exec_dynamic_arg_not_empty)
{
    DCustomActionParser parser;
    DCustomActionData actData;

    actData.m_command = "%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "%f%F%u%U";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "%a%f%u%U";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "%f/home/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "%a%f/home/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/%fhome/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/%f%ahome/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/%a%fhome/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home%f/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test/%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test/%a%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test %f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test %a%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test --test%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test --test%f%F";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test --test%a --test01%F";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test --test%f --test01%F";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "\"/home/test/%f\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "\"/home/test/%a%f\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test/ \"%f\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test/ \"%a%f\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test \"--%f%F\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

}
//后续持续补齐
