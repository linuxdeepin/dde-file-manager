# 数据库使用指南

> DDE File Manager 数据库操作规范，包括 SQLite ORM、连接池和配置文件管理。

---

## 概述

项目使用 **SQLite** 作为嵌入式数据库，通过自定义 ORM 和连接池实现数据访问。

**特点**:
- 线程安全的连接池管理
- 基于 Qt 元对象系统的 ORM 实现
- 事务支持
- JSON 配置文件管理

---

## SQLite 数据库

### 连接池管理

**优先使用连接池**: 每个线程有独立的数据库连接，自动管理生命周期。

```cpp
// 获取数据库连接
QSqlDatabase db = SqliteConnectionPool::instance().openConnection(databaseName);

// 连接自动在线程结束时清理
```

**示例位置**: `src/dfm-base/base/db/sqliteconnectionpool.cpp`

```cpp
QSqlDatabase SqliteConnectionPool::openConnection(const QString &databaseName)
{
    assert(!databaseName.isEmpty());
    assert(QUrl::fromLocalFile(databaseName).isValid());

    QString baseConnectionName = "conn_" + QString::number(quint64(QThread::currentThread()), 16);
    QString fullConnectionName = baseConnectionName + "_" + d->makeConnectionName(databaseName);

    if (QSqlDatabase::contains(fullConnectionName)) {
        QSqlDatabase existingDb = QSqlDatabase::database(fullConnectionName);
        QSqlQuery query(kTestSql, existingDb);
        if (query.lastError().type() != QSqlError::NoError && !existingDb.open()) {
            qCCritical(logDFMBase) << "Failed to open existing SQLite database connection";
            return QSqlDatabase();
        }
        return existingDb;
    }

    // 为新线程创建连接
    if (qApp != nullptr) {
        QObject::connect(QThread::currentThread(), &QThread::finished, qApp, [fullConnectionName] {
            if (QSqlDatabase::contains(fullConnectionName)) {
                QSqlDatabase::removeDatabase(fullConnectionName);
            }
        });
    }
    return d->createConnection(databaseName, fullConnectionName);
}
```

**关键点**:
- 连接名称包含线程 ID，确保线程隔离
- 重复使用时先测试连接有效性
- 线程结束时自动清理连接

---

### ORM 操作

#### 1. 事务模式

**所有数据库操作必须使用事务**，确保数据一致性。

```cpp
// 标准事务模式
bool ret = handle->transaction([tmpData, this]() -> bool {
    for (auto dataIt = tmpData.begin(); dataIt != tmpData.end(); ++dataIt) {
        bool ret = tagFile(dataIt.key(), dataIt.value());
        if (!ret) {
            fmCritical() << "Transaction failed for file:" << dataIt.key();
            return ret;
        }
    }
    return true;
});

if (!ret) {
    fmCritical() << "Transaction failed, rollback executed";
}
```

**示例位置**: `src/plugins/daemon/tag/tagdbhandler.cpp`

#### 2. 插入操作

```cpp
template<typename T>
int insert(const T &entity, bool customPK = false)
{
    const QStringList &fieldNames { SqliteHelper::fieldNames<T>() };

    QString fmtFields;
    QString fmtValues;

    for (int i = startIndex; i != fieldNames.size(); ++i) {
        fmtFields += (fieldNames[i] + ",");
        fmtValues += (serializeValue(fieldNames[i]) + ",");
    }

    int lastId = -1;
    if (excute("INSERT INTO " + SqliteHelper::tableName<T>()
               + "(" + fmtFields + ") VALUES (" + fmtValues + ");",
               [&lastId](QSqlQuery *query) {
                   Q_ASSERT(query);
                   lastId = query->lastInsertId().toInt();
               }))
        return -1;
    return lastId;
}
```

**示例位置**: `src/dfm-base/base/db/sqlitehandle.h`

#### 3. 查询操作

```cpp
QVariantMap TagDbHandler::getAllTags()
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    finally.dismiss();

    const auto &tagPropertyBean = handle->query<TagProperty>().toBeans();
    if (tagPropertyBean.isEmpty()) {
        fmDebug() << "No tags found in database";
        return {};
    }

    QVariantMap tagPropertyMap;
    for (auto &bean : tagPropertyBean)
        tagPropertyMap.insert(bean->getTagName(), QVariant { bean->getTagColor() });

    return tagPropertyMap;
}
```

**示例位置**: `src/plugins/daemon/tag/tagdbhandler.cpp`

---

### 数据库模式

#### 表结构示例 (标签系统)

```sql
-- 标签属性表
CREATE TABLE tag_properties (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    tag_name TEXT UNIQUE NOT NULL,
    tag_color TEXT NOT NULL
);
```

**要点**:
- 主键自动递增: `INTEGER PRIMARY KEY AUTOINCREMENT`
- 唯一约束: `UNIQUE NOT NULL`

---

### 命名约定

| 元素 | 约定 | 示例 |
|------|------|------|
| 表名 | 小写+下划线 | `tag_properties` |
| 字段名 | 小写+下划线 | `tag_name`, `tag_color` |
| 实体类 | PascalCase | `TagProperty` |
| getter | `get`+字段名 | `getTagName()` |
| setter | `set`+字段名 | `setTagName()` |

---

## JSON 配置文件管理

### Settings 类

**三层配置系统**: 默认值 → 回退值 → 用户设置

```cpp
Settings(QString settingFile,
          QString jsonTemplateFile = "",
          QString fallbackSettingFile = "",
          QObject *parent = nullptr);
```

**参数说明**:
- `settingFile`: 用户设置文件
- `jsonTemplateFile`: 默认模板文件
- `fallbackSettingFile`: 回退设置文件

---

### 读取配置

```cpp
Settings *settings = Settings::instance();

QVariant value = settings->value("TestGroup", "key1");
```

**优先级**: 用户设置 → 回退值 → 默认模板

---

### 写入配置

```cpp
bool Settings::sync()
{
    // 使用 QSaveFile 原子写入，防止崩溃导致数据损坏
    QSaveFile file(d->settingFile);

    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(logDFMBase) << "Failed to open settings file for writing";
        return false;
    }

    if (file.write(json) != json.size()) {
        qCWarning(logDFMBase) << "Failed to write settings data";
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        qCWarning(logDFMBase) << "Failed to commit settings file";
        return false;
    }

    d->makeSettingFileToDirty(false);
    return true;
}
```

**示例位置**: `src/dfm-base/base/application/settings.cpp`

**关键点**:
- 使用 `QSaveFile` 实现原子写入
- 写入失败时调用 `cancelWriting()`
- 成功后调用 `commit()` 原子替换原文件

---

### JSON 结构

```json
{
    "TestGroup": {
        "key1": "value1",
        "key2": 123
    },
    "__private__": {
        "internal": "data"
    }
}
```

**约定**:
- 公共配置: 直接使用组名
- 私有配置: 使用 `__` 前缀和后缀

---

## 最佳实践

### Do
- 所有数据库操作使用事务
- 使用连接池获取数据库连接
- 配置写入使用 QSaveFile 原子操作
- 使用统一的 ORM 方法 (insert/query/update/delete)

### Don't
- 不要直接操作 QSqlDatabase (使用 SqliteConnectionPool)
- 不要在 UI 线程执行耗时数据库操作
- 不要跳过事务包装
- 不要硬编码文件路径

---

## 错误处理

```cpp
// 检查连接有效性
if (!db.isOpen() || db.lastError().type() != QSqlError::NoError) {
    fmCritical() << "Database error:" << db.lastError().text();
    return false;
}

// 检查查询结果
if (!query.exec()) {
    fmCritical() << "Query failed:" << query.lastError().text();
    return false;
}
```

---

## 常见问题

### Q: 如何处理并发访问?

**A**: 连接池为每个线程分配独立连接，使用事务隔离。

### Q: 如何批量插入?

**A**: 在事务中批量执行插入操作。

```cpp
handle->transaction([&]() {
    for (const auto &item : items) {
        if (!insert(item)) return false;
    }
    return true;
});
```

---

## 参考文档

- [Qt 内存管理陷阱](../big-question/qt-memory-management-pitfalls.md) - SQLite 连接生命周期
- [错误处理指南](error-handling.md) - 事务回滚模式
