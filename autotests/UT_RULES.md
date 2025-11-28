---
description:
globs:
alwaysApply: false
---
# Qt/C++ 单元测试规范

本规范定义了 Qt/C++项目中单元测试的组织结构、编写标准和最佳实践。

## 项目结构规范

### 目录组织
- 源代码放置在 `src/` 目录
- 测试代码放置在 `autotests/` 目录，结构与源代码目录对应
- 第三方库放置在 `3rdparty/` 目录，包含 cpp-stub 等 mock 工具
- 测试文件命名格式：`test_classname.cpp`
- cmake环境已经完全配置正确，你不需要改动任何cmake配置
- 文件开头写License：
```c++
// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
```

## 测试编写原则

### AIR 原则（必须遵守）
- **Automatic（自动化）**：测试必须全自动执行，无需人工干预
- **Independent（独立性）**：测试用例之间不能有依赖关系或执行顺序要求
- **Repeatable（可重复）**：测试结果不受外部环境影响，可重复执行

### 测试用例质量要求
- 每个测试用例必须有明确的检查点（断言）
- 测试用例命名采用格式：`[MethodUnderTest]_[Scenario]_[ExpectedResult]`
- 优先使用 `EXPECT_*` 系列断言而非 `ASSERT_*`，避免内存泄漏风险
- 使用 `TEST_F` 宏组织相关测试，通过 `SetUp()` 和 `TearDown()` 管理资源
- 为了能通过CI覆盖率检测最低80%的目标，你要尽可能的覆盖业务代码

### 测试范围界定
- 单元测试针对单个类或函数的内部逻辑，不测试模块间交互
- 对外部依赖（文件系统、网络、硬件、数据库等）必须进行打桩隔离
- 避免测试中包含时间依赖、异步流程、全局变量等不稳定因素
- 由于配置了编译器属性，你可以无视类的private和protect自由访问这些成员变量和属性，这样可以便于测试

## 打桩策略

### 工具选择
- 使用**cpp-stub**进行函数级别的打桩，适用于普通函数和成员函数
- 使用**gmock**进行接口级别的 mock，适用于虚函数接口
- 结合**QTest**进行 Qt 特有功能测试（信号槽、GUI 交互）

### 打桩原则
- 对所有外部依赖进行打桩，确保测试的独立性和可重复性，比如依赖特定的系统环境的接口、网络，特别是界面相关的接口（show、exec）
- 对于不破坏用户数据的接口，可以选择不打桩，而用一些方式模拟，比如临时文件，避免打桩太多使得本身的功能测试功能丧失
- 如果是清晰、简单的场景，可以使用真实接口，比如创建一些临时文件测试 IO 相关的接口是否正确
- 通过打桩控制不同的执行分支，实现完整的场景覆盖
- 在 `SetUp()` 中初始化打桩，在 `TearDown()` 中清理资源
- 如果需要对第三方接口打桩，需要参考源码中第三方代码的头文件、命名空间也要引入单元测试

### 打桩教程

- 头文件 `#include "stubext.h"`
- 对任何接口打桩，必须要包含原始接口本身的头文件，具体什么头文件请参考源码
- 只能对函数打桩，其他的任何都不能打桩
- 打桩的原始函数如果有默认参数，打桩时不能省略默认参数
- 定义打桩对象 `    stub_ext::StubExt stub;`
	- 也可以作为 `testing::Test` 派生类的成员对象，但是一定要在 `TearDown` 中调用 `stub.clear();` 
- 打桩一个成员函数：
	```c++ 
	stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
     __DBG_STUB_INVOKE__
     QVariantMap map;
     map[DeviceProperty::kOpticalBlank] = true;
     return map;
	 });

stub.set_lamda(ADDR(QThread, start), [&triggerStart](QThread *obj, QThread::Priority) {
    __DBG_STUB_INVOKE__
    triggerStart = true;
    AbstractBurnJob *job { qobject_cast<AbstractBurnJob *>(obj) };
    EXPECT_TRUE(job);
    EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kImageUrl).toUrl(),
              QUrl::fromLocalFile("/tmp/test.iso"));
    EXPECT_EQ(job->property(AbstractBurnJob::PropertyType::kSpeeds).toInt(),
              1);
});

// 也可以这么写
stub.set_lamda(&DConfigManager::value, [&] { __DBG_STUB_INVOKE__ return ret; });

	```

- 打桩虚函数，使用  `VADDR`
```c++
stub.set_lamda(VADDR(DDialog, exec), [&trigger] {
    __DBG_STUB_INVOKE__
    trigger = true;
    return 0;
});
```

- 打桩普通C接口：
```c++
        stub.set_lamda(syscall, [this](long number, ...) -> long {
            __DBG_STUB_INVOKE__
            if (!mockSyscallSuccess) {
                errno = mockSyscallErrno;
                return -1;
            }
            errno = 0;
            return 0;
        });
```

- 打桩重载函数
```c++
    using MimeTypeForFileFunc = QMimeType (QMimeDatabase::*)(const QString &, QMimeDatabase::MatchMode) const;

    stub.set_lamda(static_cast<MimeTypeForFileFunc>(&QMimeDatabase::mimeTypeForFile),
                   [](QMimeDatabase *, const QString &fileName, QMimeDatabase::MatchMode) -> QMimeType {
                       __DBG_STUB_INVOKE__
                       QMimeType mockType;
                       return mockType;
                   });
```

- 原则上避免给构造函数打桩
- 如果影响测试，一定要打桩构造函数，请参考：
```c++
//for linux
#include<iostream>
#include "stub.h"
using namespace std;


template<class T>
void * get_ctor_addr(bool start = true)
{
	//the start vairable must be true, or the compiler will optimize out.
    if(start) goto Start;
Call_Constructor:
    //This line of code will not be executed.
	//The purpose of the code is to allow the compiler to generate the assembly code that calls the constructor.
    T();
Start:
    //The address of the line of code T() obtained by assembly
    char * p = (char*)&&Call_Constructor;//https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
    //CALL rel32
	void * ret = 0;
	char pos;
	char call = 0xe8;
	do{
		pos = *p;
		if(pos == call)
		{
			ret = p + 5 + (*(int*)(p+1));
		}
		
	}while(!ret&&(++p));
    
    return ret;
}


class A {
public:
    A(){cout << "I am A_constructor" << endl;}
};

class B {
public:
    B(){cout << "I am B_constructor" << endl;}
};


int main()
{
    Stub stub;
    auto xa = get_ctor_addr<A>();
    auto xb = get_ctor_addr<B>();
    stub.set(xa, xb);
    A aa;
    return 0;
}
```

- 模板函数打桩：
```c++
#include<iostream>
#include "stub.h"
using namespace std;
class A{
public:
   template<typename T>
   int foo(T a)
   {   
        cout<<"I am A_foo"<<endl;
        return 0;
   }
};

int foo_stub(void* obj, int x)
{   
    A* o= (A*)obj;
    cout<<"I am foo_stub"<<endl;
    return 0;
}


int main()
{
    Stub stub;
    stub.set((int(A::*)(int))ADDR(A,foo), foo_stub);
    A a;
    a.foo(5);
    return 0;
}
```

- 重载且是虚函数的函数打桩：
```c++
//for linux gcc
#include<iostream>
#include "stub.h"
using namespace std;
class A{
    int i;
public:
    virtual int foo(int a){
        cout<<"I am A_foo"<<endl;
        return 0;
    }
    virtual int foo(double a){
        cout<<"I am A_foo"<<endl;
        return 0;
    }
};

int foo_stub(void* obj, int a)
{
    A* o= (A*)obj;
    cout<<"I am foo_stub"<<endl;
    return 0;
}


int main()
{
    typedef int (*fptr)(A*,int);
    fptr A_foo = (fptr)((int(A::*)(int))&A::foo);
    Stub stub;
    stub.set(A_foo, foo_stub);
    A a;
    a.foo(1);
    return 0;
}
```

## 代码质量保证

### 覆盖率要求
- 使用 lcov 工具生成代码覆盖率报告
- 关注场景覆盖而非单纯的代码行覆盖率
- 排除测试代码和第三方库的覆盖率统计

### 性能考虑
- 测试执行速度要快，避免长时间运行的操作，不要编写性能测试的用例！
- 合理使用并行测试执行
- 及时清理测试过程中创建的资源，防止内存泄漏

### 环境隔离
- 测试不能修改系统环境或删除系统资源
- 测试创建的临时文件和数据必须在测试结束后清理
- 避免测试对生产环境造成任何影响

## Qt 特定测试指导

### 信号槽测试
- 使用 QSignalSpy 监控信号发射, 原则测试代码不能用QObject::connect, 应该使用 QSignalSpy
- 通过打桩验证信号槽的调用关系
- 测试信号参数的正确性

### GUI 测试集成
- 结合 QTest 进行键盘鼠标事件模拟
- 使用 GTest 的断言进行结果验证
- 避免依赖真实的用户界面环境

## 常见问题避免

### 测试稳定性
- 不要在测试中使用随机数或当前时间
- 避免依赖网络连接或外部服务
- 不要在测试中使用 sleep 或延时操作
- 有重大逻辑缺陷的代码可以跳过单元测试，并告诉我哪里有缺陷
- 重载函数你没按要求打桩是你最常犯的错误，务必谨记重载函数打桩方法！

### 测试独立性
- 每个测试用例都应该能够单独运行
- 测试用例之间不能共享状态或数据
- 避免使用全局变量或静态变量传递测试数据

### 资源管理
- 在 SetUp () 中创建的资源必须在 TearDown () 中释放
- 使用 RAII 原则管理动态分配的内存
- 确保异常情况下资源也能正确释放

通过遵循这些规范，可以构建高质量、可维护的 Qt/C++单元测试套件，有效保障代码质量和系统稳定性。