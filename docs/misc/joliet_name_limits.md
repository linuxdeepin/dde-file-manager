# Joliet 文件名长度限制分析

## 模式说明

本文档分析在 `rockridge_off joliet_on joliet_long_name_on` 模式下的文件名长度限制因素。

---

## 1. 整体架构图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         ISO 9660 目录记录结构                                │
│                      (最大 254 字节, ECMA-119 规范)                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────────────────────────┐  ┌──────────────────────────────────────┐ │
│  │      固定头部 (33 字节)       │  │        可变部分 (最大 221 字节)       │ │
│  ├──────────────────────────────┤  ├──────────────────────────────────────┤ │
│  │ len_dr     (1)  记录总长度    │  │                                      │ │
│  │ len_xa     (1)  扩展属性长度  │  │    file_id: 文件标识符               │ │
│  │ block      (8)  起始块位置    │  │    ├── Joliet 标准: 最大 128 字节    │ │
│  │ length     (8)  数据长度      │  │    │   (64 个 UCS-2 字符)            │ │
│  │ rec_time   (7)  记录时间      │  │    │                                 │ │
│  │ flags      (1)  标志位        │  │    └── Joliet 扩展: 最大 206 字节    │ │
│  │ file_unit  (1)  文件单元大小  │  │        (103 个 UCS-2 字符)           │ │
│  │ gap_size   (1)  间隔大小      │  │                                      │ │
│  │ vol_seq    (4)  卷序号        │  │    padding: 填充字节 (0 或 1 字节)   │ │
│  │ len_fi     (1)  文件名长度    │  │    system_use: 系统使用区 (可选)     │ │
│  └──────────────────────────────┘  └──────────────────────────────────────┘ │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. 限制因素详解

### 2.1 ISO 9660 目录记录的硬性限制

根据 ECMA-119 规范（9.1 节），目录记录结构如下：

```c
// 文件: libisofs/ecma119.h:993-1009
struct ecma119_dir_record
{
    uint8_t len_dr        BP(1, 1);    // 目录记录总长度 (1 字节, 最大值 254)
    uint8_t len_xa        BP(2, 2);    // 扩展属性长度
    uint8_t block         BP(3, 10);   // 数据起始块 (8 字节)
    uint8_t length        BP(11, 18);  // 数据长度 (8 字节)
    uint8_t recording_time BP(19, 25); // 记录时间 (7 字节)
    uint8_t flags         BP(26, 26);  // 标志位
    uint8_t file_unit_size BP(27, 27); // 文件单元大小
    uint8_t interleave_gap_size BP(28, 28); // 间隔大小
    uint8_t vol_seq_number BP(29, 32); // 卷序号 (4 字节)
    uint8_t len_fi        BP(33, 33);  // 文件标识符长度
    uint8_t file_id       BP(34, 34);  // 文件标识符开始位置
    // padding field (如果 len_fi 是偶数则需要)
    // system use (系统使用区域)
};
```

**关键约束:**
- `len_dr` 是 1 字节，理论最大值为 255，但 ECMA-119 规定最大为 **254 字节**
- 固定头部占用 **33 字节**
- 剩余可用于文件名和系统使用区: **254 - 33 = 221 字节**

### 2.2 Joliet 规范的软性限制

```c
// 文件: libisofs/joliet.c:404-417
/*
 * From Joliet specs:
 * "ISO 9660 (Section 7.5.1) states that the sum of the following shall not
 *  exceed 30:
 *  - If there is a file name, the length of the file name.
 *  - If there is a file name extension, the length of the file name extension.
 *  On Joliet compliant media, however, the sum as calculated above shall not
 *  exceed 128 [bytes], to allow for longer file identifiers."
 *
 * I.e. the dot does not count.
 *
 * (We have an option to lift the limit from 64*2 to 103*2, which is the
 *  maximum to fit into an ISO 9660 directory record.)
 */
```

---

## 3. 两种模式对比

```
┌────────────────────────────────────────────────────────────────────────────┐
│                          文件名长度限制对比                                 │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│   标准 Joliet 模式                    扩展 Joliet 模式                     │
│   (joliet_long_names = 0)             (joliet_long_names = 1)              │
│                                                                            │
│   ┌──────────────────────┐            ┌──────────────────────┐             │
│   │  最大 64 个字符       │            │  最大 103 个字符      │             │
│   │  = 128 字节           │            │  = 206 字节          │             │
│   │  (符合 Joliet 规范)   │            │  (突破 Joliet 规范)   │             │
│   └──────────────────────┘            └──────────────────────┘             │
│                                                                            │
│   兼容性: ★★★★★                     兼容性: ★★★☆☆                      │
│   文件名长度: ★★☆☆☆                 文件名长度: ★★★★☆                  │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
```

### 数值总结表

| 参数 | 标准模式 | 扩展模式 | 单位 |
|------|---------|---------|------|
| maxchar | 64 | 103 | 字符 |
| 文件名最大字节数 | 128 | 206 | 字节 |
| UCS-2 编码 | 2 | 2 | 字节/字符 |

---

## 4. 代码实现分析

### 4.1 核心常量定义

```c
// 文件: libisofs/joliet.h:21-23
/* was formerly 66 = 64 + 2. Now 105 = 103 + 2.
*/
#define LIBISO_JOLIET_NAME_MAX 105
```

这个宏定义了 Joliet 名称的最大缓冲区大小：
- 103 个字符 + 1 个点 + 1 个结束符 = 105

### 4.2 文件名处理函数

```c
// 文件: libisofs/util.c:1216-1295
/*
   bit0= no_force_dots
   bit1= allow 103 characters rather than 64  <-- 关键标志位
*/
uint16_t *iso_j_file_id(const uint16_t *src, int flag)
{
    uint16_t *dot, *retval = NULL;
    size_t lname, lext, lnname, lnext, pos, i, maxchar = 64;  // 默认 64 字符
    uint16_t *dest = NULL, c;

    LIBISO_ALLOC_MEM_VOID(dest, uint16_t, LIBISO_JOLIET_NAME_MAX);

    if (src == NULL) {
        goto ex;
    }
    if (flag & 2)
        maxchar = 103;  // 如果 bit1 置位，扩展到 103 字符

    dot = ucsrchr(src, '.');

    /*
     * 名称和扩展名的长度可以自由分配，总和不超过 maxchar
     * 如果原始文件名过长，优先裁剪扩展名，但保留至少 3 个字符的扩展名
     */
    if (dot == NULL || cmp_ucsbe(dot + 1, '\0') == 0) {
        // 无扩展名的情况
        lname = ucslen(src);
        lnname = (lname > maxchar) ? maxchar : lname;
        lext = lnext = 0;
    } else {
        // 有扩展名的情况
        lext = ucslen(dot + 1);
        lname = ucslen(src) - lext - 1;
        lnext = (ucslen(src) > maxchar + 1 && lext > 3)
                ? (lname < maxchar - 3 ? maxchar - lname : 3)
                : lext;
        lnname = (ucslen(src) > maxchar + 1) ? maxchar - lnext : lname;
    }
    // ... 后续处理 ...
}
```

### 4.3 目录名处理函数

```c
// 文件: libisofs/util.c:1299-1330
/* @param flag bit1= allow 103 characters rather than 64
*/
uint16_t *iso_j_dir_id(const uint16_t *src, int flag)
{
    size_t len, i, maxchar = 64;  // 默认 64 字符
    uint16_t *dest = NULL, *retval = NULL;

    LIBISO_ALLOC_MEM_VOID(dest, uint16_t, LIBISO_JOLIET_NAME_MAX);

    if (src == NULL) {
        goto ex;
    }
    if (flag & 2)
        maxchar = 103;  // 扩展模式

    len = ucslen(src);
    if (len > maxchar) {
        len = maxchar;  // 直接截断
    }
    // ... 字符转换处理 ...
}
```

### 4.4 选项传递流程

```c
// 文件: libisofs/joliet.c:77-82
if (node_type == LIBISO_DIR) {
    jname = iso_j_dir_id(ucs_name, opts->joliet_long_names << 1);
} else {
    jname = iso_j_file_id(ucs_name,
             (opts->joliet_long_names << 1) | !!(opts->no_force_dots & 2));
}
```

**流程图:**

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        Joliet 文件名处理流程                             │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
                    ┌───────────────────────────────┐
                    │    iso_get_joliet_name()      │
                    │    (libisofs/joliet.c:33)     │
                    └───────────────┬───────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │                               │
                    ▼                               ▼
        ┌───────────────────┐           ┌───────────────────┐
        │   是目录节点?      │           │   是文件节点?      │
        └─────────┬─────────┘           └─────────┬─────────┘
                  │                               │
                  ▼                               ▼
        ┌───────────────────┐           ┌───────────────────┐
        │  iso_j_dir_id()   │           │  iso_j_file_id()  │
        │  (util.c:1299)    │           │  (util.c:1216)    │
        └─────────┬─────────┘           └─────────┬─────────┘
                  │                               │
                  └───────────────┬───────────────┘
                                  │
                                  ▼
                    ┌───────────────────────────────┐
                    │   检查 flag & 2               │
                    │   (joliet_long_names << 1)    │
                    └───────────────┬───────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │                               │
                    ▼                               ▼
            ┌───────────────┐               ┌───────────────┐
            │  maxchar = 64 │               │ maxchar = 103 │
            │  (标准模式)    │               │  (扩展模式)   │
            └───────────────┘               └───────────────┘
```

---

## 5. 103 字符限制的由来

### 5.1 计算推导

```
ISO 9660 目录记录最大长度:                    254 字节
减去固定头部:                                - 33 字节
剩余可用空间:                                = 221 字节

Joliet 文件名使用 UCS-2 编码 (2 字节/字符)
可用于文件名的字符数:                        221 / 2 = 110.5

但是还需要考虑:
- 版本号 ";1" (4 字节, 可选)                 - 4 字节
- 填充字节 (偶数对齐)                        - 1 字节

保守计算:
(221 - 4 - 1) / 2 = 108 字符

libisofs 选择 103 作为安全边界，留有余量用于:
- 版本号扩展
- 系统使用区
- 边界安全
```

### 5.2 目录记录大小计算代码

```c
// 文件: libisofs/joliet.c:673-683
/**
 * Compute the size of a directory entry for a single node
 */
static
size_t calc_dirent_len(Ecma119Image *t, JolietNode *n)
{
    /* note than name len is always even, so we always need the pad byte */
    int ret = n->name ? ucslen(n->name) * 2 + 34 : 34;
    if (n->type == JOLIET_FILE && !(t->opts->omit_version_numbers & 3)) {
        /* take into account version numbers */
        ret += 4;  // ";1" 占用 4 字节 (UCS-2 编码)
    }
    return ret;
}
```

**计算验证:**
- 103 个字符 × 2 字节 = 206 字节
- 加上固定头部: 206 + 34 = 240 字节
- 加上版本号: 240 + 4 = 244 字节
- 加上填充: 244 + 0/1 = 244/245 字节
- 小于 254 字节 ✓

---

## 6. 名称冲突处理 (Name Mangling)

当同一目录下存在重名文件时，系统会自动添加数字后缀：

```c
// 文件: libisofs/joliet.c:418-439
static
int mangle_single_dir(Ecma119Image *t, JolietNode *dir)
{
    int ret;
    int i, nchildren, maxchar = 64;
    // ...

    if (t->opts->joliet_long_names)
        maxchar = 103;  // 扩展模式下使用 103

    // ... 冲突检测和处理 ...
}
```

**Mangling 规则图示:**

```
原始文件名冲突时的处理:

┌──────────────────────────────────────────────────────────────────┐
│  假设 maxchar = 103, 有 3 个文件同名 "very_long_filename.txt"    │
└──────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────────────┐
│  文件 1: very_long_filename0.txt    (添加数字 0)                 │
│  文件 2: very_long_filename1.txt    (添加数字 1)                 │
│  文件 3: very_long_filename2.txt    (添加数字 2)                 │
└──────────────────────────────────────────────────────────────────┘

如果文件名过长需要为数字腾出空间:

┌──────────────────────────────────────────────────────────────────┐
│  原始: "a]" (103 字符, 假设)                                    │
│  ──────────────────────────────────────────────────────────────  │
│  处理后: "a_b_c_d...xyz12.txt" (保留扩展名至少 3 字符)           │
│          ├─ name 部分被截断                                      │
│          ├─ 数字后缀 (1-7 位)                                    │
│          └─ ext 部分保留                                         │
└──────────────────────────────────────────────────────────────────┘
```

---

## 7. 限制因素总结图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    Joliet 文件名长度限制因素层次图                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Level 1: 物理层限制 (ECMA-119)                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  目录记录最大 254 字节                                                 │  │
│  │  固定头部 33 字节，剩余 221 字节可用                                   │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                              │                                              │
│                              ▼                                              │
│  Level 2: 编码层限制 (UCS-2/UTF-16)                                         │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  每个字符占用 2 字节                                                   │  │
│  │  理论最大: 221 / 2 ≈ 110 字符                                         │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                              │                                              │
│                              ▼                                              │
│  Level 3: 规范层限制 (Joliet Specification)                                 │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  标准 Joliet: 64 字符 (128 字节)                                       │  │
│  │  为了广泛兼容性而设定                                                  │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                              │                                              │
│                              ▼                                              │
│  Level 4: 实现层限制 (libisofs)                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  扩展模式: 103 字符 (206 字节)                                         │  │
│  │  考虑版本号、填充、安全边界                                            │  │
│  │  LIBISO_JOLIET_NAME_MAX = 105 (含点和结束符)                          │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 8. 配置选项 API

```c
// 文件: libisofs/ecma119.h:192-199
struct iso_write_opts {
    // ...

    /**
     * Allow paths in the Joliet tree to have more than 240 characters.
     */
    unsigned int joliet_longer_paths :1;  // 路径长度扩展

    /**
     * Allow Joliet names up to 103 characters rather than 64.
     */
    unsigned int joliet_long_names :1;    // 文件名长度扩展 <-- 本文关注点

    // ...
};
```

---

## 9. 结论

在 `rockridge_off joliet_on joliet_long_name_on` 模式下：

| 限制因素 | 具体值 | 说明 |
|---------|--------|------|
| **最终文件名限制** | **103 字符** | joliet_long_names 启用 |
| 字节数 | 206 字节 | UCS-2 编码 |
| 目录名限制 | 103 字符 | 与文件名相同 |
| 路径长度限制 | 240 字节 | 除非启用 joliet_longer_paths |

**限制优先级链:**
```
ECMA-119 (254字节) → UCS-2编码 (110字符) → libisofs实现 (103字符)
```

**为什么是 103 而不是更大?**
1. 需要为版本号 ";1" 预留空间 (4 字节)
2. 需要考虑填充对齐 (0-1 字节)
3. 保留安全余量用于特殊情况
4. 与名称冲突处理机制兼容 (需要数字后缀空间)

---

## 10. ISO 树（IsoNode）的文件名限制

除了 Joliet 树的限制外，libisofs 的内部节点树（ISO 树）也有自己的文件名长度限制。

### 10.1 核心常量定义

```c
// 文件: libisofs/node.h:33-43
/* Maximum length of a leaf name in the libisofs node tree. This is currently
   restricted by the implemented maximum length of a Rock Ridge name.
   This might later become larger and may then be limited to smaller values.

   Rock Ridge specs do not impose an explicit limit on name length.
   But 255 is also specified by
     http://pubs.opengroup.org/onlinepubs/009695399/basedefs/limits.h.html
   which says
     NAME_MAX >= _XOPEN_NAME_MAX = 255
*/
#define LIBISOFS_NODE_NAME_MAX 255
```

**说明:**
- ISO 树中的节点名称最大为 **255 字节**
- 这个限制来源于 POSIX 标准中的 `NAME_MAX`
- Rock Ridge 规范本身没有明确的名称长度限制

### 10.2 名称验证函数

```c
// 文件: libisofs/node.c:1245-1275
int iso_node_is_valid_name(const char *name)
{
    /* a name can't be NULL */
    if (name == NULL) {
        return ISO_NULL_POINTER;
    }

    /* guard against the empty string or big names... */
    if (name[0] == '\0')
        goto rr_reserved;
    if (strlen(name) > LIBISOFS_NODE_NAME_MAX)    // <-- 255 字节检查
        return ISO_RR_NAME_TOO_LONG;

    /* ...against "." and ".." names... */
    if (!strcmp(name, ".") || !strcmp(name, ".."))
        goto rr_reserved;

    /* ...and against names with '/' */
    if (strchr(name, '/') != NULL)
        goto rr_reserved;

    return 1;
    // ...
}
```

### 10.3 名称截断机制

当文件名超过限制时，libisofs 提供了截断机制：

```c
// 文件: libisofs/image.c:1100-1108
int iso_image_set_truncate_mode(IsoImage *img, int mode, int length)
{
    if (mode < 0 || mode > 1)
        return ISO_WRONG_ARG_VALUE;
    if (length < 64 || length > LIBISOFS_NODE_NAME_MAX)  // 64 到 255
        return ISO_WRONG_ARG_VALUE;
    img->truncate_mode = mode;
    img->truncate_length = length;
    return ISO_SUCCESS;
}
```

**截断模式说明:**

```c
// 文件: libisofs/libisofs.h:3264-3274
// mode = 0: 不截断，如果文件名超长则报错 ISO_RR_NAME_TOO_LONG
// mode = 1: 截断到 length，末尾 33 字节用 ':' + MD5 哈希值替换
//           (默认模式)
// length: 最大字节数，范围 64 到 255，默认 255
```

### 10.4 ISO 树与 Joliet 树的关系图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    libisofs 文件名处理完整流程                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                     用户输入文件名                                    │   │
│  │                    (任意长度的 UTF-8)                                │   │
│  └─────────────────────────────────┬───────────────────────────────────┘   │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │              Level 0: ISO 树节点名称检查                              │   │
│  │                                                                      │   │
│  │  LIBISOFS_NODE_NAME_MAX = 255 字节                                   │   │
│  │  ├── 检查: iso_node_is_valid_name()                                  │   │
│  │  ├── 截断: iso_image_truncate_name() (mode=1 时)                     │   │
│  │  └── 编码: UTF-8 (与系统一致)                                        │   │
│  └─────────────────────────────────┬───────────────────────────────────┘   │
│                                    │                                        │
│          ┌─────────────────────────┼─────────────────────────┐              │
│          │                         │                         │              │
│          ▼                         ▼                         ▼              │
│  ┌───────────────┐        ┌───────────────┐        ┌───────────────┐       │
│  │   ECMA-119    │        │    Joliet     │        │  Rock Ridge   │       │
│  │   (ISO 9660)  │        │     树        │        │     树        │       │
│  ├───────────────┤        ├───────────────┤        ├───────────────┤       │
│  │ 8.3 或 31 字符│        │ 64/103 字符   │        │ 255 字节      │       │
│  │ (ASCII 子集)  │        │ (UCS-2 编码)  │        │ (保留原名)    │       │
│  └───────────────┘        └───────────────┘        └───────────────┘       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10.5 在 rockridge_off 模式下的影响

当 Rock Ridge 关闭时：

```
┌─────────────────────────────────────────────────────────────────────────────┐
│              rockridge_off joliet_on joliet_long_name_on 模式               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  输入文件名 ─────────────────────────────────────────────────────────────►  │
│       │                                                                     │
│       ▼                                                                     │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │ 第一道检查: ISO 树节点                                                │   │
│  │ ├── 限制: 255 字节 (UTF-8)                                           │   │
│  │ └── 超长处理: 截断 + MD5 后缀 (mode=1)                               │   │
│  │              或报错 (mode=0)                                          │   │
│  └─────────────────────────────────┬───────────────────────────────────┘   │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │ 第二道检查: Joliet 树 (实际写入 ISO 镜像)                             │   │
│  │ ├── 限制: 103 字符 (UCS-2, 206 字节)                                 │   │
│  │ ├── 转码: UTF-8 → UCS-2                                              │   │
│  │ └── 超长处理: 直接截断                                                │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  ════════════════════════════════════════════════════════════════════════  │
│                                                                             │
│  最终有效限制: min(255 字节 UTF-8, 103 字符 UCS-2)                         │
│                                                                             │
│  实际情况分析:                                                              │
│  ├── 纯 ASCII: min(255, 103) = 103 字符                                    │
│  ├── 中文 UTF-8: 每字符 3 字节, 255/3 ≈ 85 字符                           │
│  │              但 UCS-2 中仍是 103 字符                                   │
│  │              所以中文情况下: 85 字符 (受 ISO 树限制)                    │
│  └── 最终瓶颈取决于字符编码                                                │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10.6 完整限制链条

```
                    ISO 树限制                    Joliet 树限制
                        │                             │
                        ▼                             ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                                                                          │
│   UTF-8 输入 ───► 255 字节检查 ───► UCS-2 转码 ───► 103 字符检查 ───►   │
│                        │                                 │               │
│                        │                                 │               │
│              超长时截断/报错                      超长时直接截断          │
│                                                                          │
└──────────────────────────────────────────────────────────────────────────┘

最终写入 ISO 的 Joliet 文件名: 最多 103 个 UCS-2 字符 (206 字节)
```

---

## 11. 综合限制总结表

| 限制层级 | 适用范围 | 最大值 | 编码 | 相关代码 |
|---------|---------|--------|------|----------|
| **ISO 树节点** | libisofs 内部 | 255 字节 | UTF-8 | `node.h:43` |
| **Joliet 标准** | Joliet 规范 | 64 字符 (128 字节) | UCS-2 | `joliet.c:411` |
| **Joliet 扩展** | joliet_long_names=1 | 103 字符 (206 字节) | UCS-2 | `util.c:1229` |
| **ECMA-119** | 目录记录 | 254 字节 | - | `ecma119.h:996` |

在 `rockridge_off joliet_on joliet_long_name_on` 模式下的**实际有效限制**:

| 字符类型 | ISO 树瓶颈 | Joliet 瓶颈 | 实际限制 |
|---------|-----------|------------|---------|
| 纯 ASCII | 255 字符 | 103 字符 | **103 字符** |
| 中文 (UTF-8) | ~85 字符 | 103 字符 | **~85 字符** |
| 混合字符 | 取决于比例 | 103 字符 | **视情况而定** |

---

## 12. 相关源文件

- `libisofs/node.h:43` - LIBISOFS_NODE_NAME_MAX 定义 (255)
- `libisofs/node.c:1245-1275` - iso_node_is_valid_name 名称验证
- `libisofs/image.c:1100-1108` - iso_image_set_truncate_mode 截断设置
- `libisofs/libisofs.h:3232-3280` - 截断模式 API 文档
- `libisofs/joliet.h:23` - LIBISO_JOLIET_NAME_MAX 定义 (105)
- `libisofs/joliet.c:404-417` - Joliet 规范说明注释
- `libisofs/joliet.c:418-608` - mangle_single_dir 名称冲突处理
- `libisofs/util.c:1216-1295` - iso_j_file_id 文件名处理
- `libisofs/util.c:1299-1330` - iso_j_dir_id 目录名处理
- `libisofs/ecma119.h:993-1009` - 目录记录结构定义
- `libisofs/ecma119.h:197-199` - joliet_long_names 选项定义

---

## 13. 命令执行调用栈分析

以下是执行命令时与文件名长度检查相关的完整调用栈：

```bash
xorriso -outdev A.iso -joliet on -rockridge off \
        -compliance joliet_long_names:joliet_long_paths:joliet_utf16 \
        -volid "刻录测试素材" -map ./ / -commit
```

### 13.1 调用栈总览图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         xorriso 命令执行调用栈                               │
│                    (文件名长度检查相关)                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  阶段 1: xorriso 命令解析与入口                                       │   │
│  │  ════════════════════════════════════════════════════════════════   │   │
│  │                                                                      │   │
│  │  parse_exec.c:1477  Xorriso_cmd_map()                               │   │
│  │       │                                                              │   │
│  │       ▼                                                              │   │
│  │  opts_i_o.c:507   Xorriso_option_map()                              │   │
│  │       │           ├── 规范化 disk_path → eff_origin                  │   │
│  │       │           └── 规范化 iso_path  → eff_dest                    │   │
│  │       ▼                                                              │   │
│  │  iso_manip.c:789  Xorriso_graft_in()    ←── 核心导入函数             │   │
│  │                                                                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  阶段 2: 文件/目录递归处理                                            │   │
│  │  ════════════════════════════════════════════════════════════════   │   │
│  │                                                                      │   │
│  │  Xorriso_graft_in()                                                 │   │
│  │       │                                                              │   │
│  │       ├──[目录]──► iso_manip.c:356  Xorriso_add_tree()              │   │
│  │       │                  │                                           │   │
│  │       │                  ├── Dirseq 迭代目录内容                      │   │
│  │       │                  │                                           │   │
│  │       │                  └── 对每个条目递归调用:                       │   │
│  │       │                       ├── Xorriso_add_tree() [子目录]        │   │
│  │       │                       └── Xorriso_tree_graft_node() [文件]   │   │
│  │       │                                                              │   │
│  │       └──[文件]──► iso_manip.c:175  Xorriso_tree_graft_node()       │   │
│  │                                                                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  阶段 3: xorriso 层文件名截断检查                                     │   │
│  │  ════════════════════════════════════════════════════════════════   │   │
│  │                                                                      │   │
│  │  iso_manip.c:175  Xorriso_tree_graft_node()                         │   │
│  │       │                                                              │   │
│  │       ├── [L192] if(strlen(eff_name) > xorriso->file_name_limit)    │   │
│  │       │          ├── file_name_limit 默认 255                        │   │
│  │       │          └── 调用 iso_truncate_leaf_name() 截断              │   │
│  │       │                                                              │   │
│  │       └── [L238] iso_tree_add_new_node()  ───────────────────────►  │   │
│  │                                                                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  阶段 4: libisofs ISO树节点创建                                       │   │
│  │  ════════════════════════════════════════════════════════════════   │   │
│  │                                                                      │   │
│  │  tree.c:621  iso_tree_add_new_node()                                │   │
│  │       │                                                              │   │
│  │       ├── [L639] iso_image_truncate_name()  ←── 255字节截断检查      │   │
│  │       │              │                                               │   │
│  │       │              └── image.c:1120                                │   │
│  │       │                   └── iso_truncate_rr_name()                 │   │
│  │       │                                                              │   │
│  │       ├── [L644] iso_dir_exists()  检查是否重名                      │   │
│  │       │                                                              │   │
│  │       ├── [L656] builder->create_node()                             │   │
│  │       │              │                                               │   │
│  │       │              └── image_builder_create_node()                 │   │
│  │       │                   └── 创建 IsoNode, 设置 node->name          │   │
│  │       │                                                              │   │
│  │       └── [L671] iso_dir_insert()  插入到目录树                      │   │
│  │                                                                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════   │
│  │                    -commit 触发 ISO 写入                               │   │
│  ═══════════════════════════════════════════════════════════════════════   │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  阶段 5: Joliet 树创建与文件名转换                                    │   │
│  │  ════════════════════════════════════════════════════════════════   │   │
│  │                                                                      │   │
│  │  joliet.c:1267  joliet_writer_create()                              │   │
│  │       │                                                              │   │
│  │       └── [L1285] joliet_tree_create()                              │   │
│  │                │                                                     │   │
│  │                └── joliet.c:635                                      │   │
│  │                     │                                                │   │
│  │                     └── [L644] create_tree()  递归创建 Joliet 树     │   │
│  │                              │                                       │   │
│  │                              └── joliet.c:228                        │   │
│  │                                                                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  阶段 6: Joliet 文件名处理 (核心限制检查)                             │   │
│  │  ════════════════════════════════════════════════════════════════   │   │
│  │                                                                      │   │
│  │  joliet.c:228  create_tree()                                        │   │
│  │       │                                                              │   │
│  │       └── [L242] get_joliet_name()                                  │   │
│  │                │                                                     │   │
│  │                └── joliet.c:106                                      │   │
│  │                     │                                                │   │
│  │                     └── iso_get_joliet_name()                        │   │
│  │                              │                                       │   │
│  │                              └── joliet.c:33                         │   │
│  │                                                                      │   │
│  │  joliet.c:33  iso_get_joliet_name()                                 │   │
│  │       │                                                              │   │
│  │       ├── [L48-76] 字符集转换                                        │   │
│  │       │     ├── joliet_utf16=1: str2utf16be()                       │   │
│  │       │     └── joliet_utf16=0: str2ucs()                           │   │
│  │       │                                                              │   │
│  │       └── [L77-82] Joliet 名称格式化                                 │   │
│  │             │                                                        │   │
│  │             ├──[目录]──► iso_j_dir_id(ucs_name, flag)               │   │
│  │             │                 │                                      │   │
│  │             │                 └── util.c:1299                        │   │
│  │             │                      ├── maxchar = 64 (默认)           │   │
│  │             │                      └── maxchar = 103 (long_names)    │   │
│  │             │                                                        │   │
│  │             └──[文件]──► iso_j_file_id(ucs_name, flag)              │   │
│  │                               │                                      │   │
│  │                               └── util.c:1216                        │   │
│  │                                    ├── maxchar = 64 (默认)           │   │
│  │                                    └── maxchar = 103 (long_names)    │   │
│  │                                                                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  阶段 7: Joliet 路径长度检查                                          │   │
│  │  ════════════════════════════════════════════════════════════════   │   │
│  │                                                                      │   │
│  │  joliet.c:228  create_tree()                                        │   │
│  │       │                                                              │   │
│  │       └── [L246-259] 路径长度检查                                    │   │
│  │             │                                                        │   │
│  │             ├── max_path = pathlen + 1 + ucslen(jname) * 2          │   │
│  │             │                                                        │   │
│  │             └── if (!joliet_longer_paths && max_path > 240)         │   │
│  │                  └── 报错: ISO_FILE_IMGPATH_WRONG                    │   │
│  │                                                                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 13.2 关键函数调用链 (简化版)

```
用户执行: xorriso -map ./ /
                │
                ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  xorriso/parse_exec.c:1477                                              │
│  └─► Xorriso_option_map(disk_path="./" , iso_path="/")                 │
│                                                                         │
│  xorriso/opts_i_o.c:534                                                │
│  └─► Xorriso_graft_in(eff_origin, eff_dest)                            │
│                                                                         │
│  xorriso/iso_manip.c:1012 [目录] 或 :1026 [文件]                        │
│  └─► Xorriso_add_tree() 或 Xorriso_tree_graft_node()                   │
│                                                                         │
│  xorriso/iso_manip.c:192-207  【第一道检查: xorriso 层】                │
│  └─► if(strlen(eff_name) > xorriso->file_name_limit)                   │
│       └─► iso_truncate_leaf_name(1, 255, trunc_name, 0)                │
│                                                                         │
│  xorriso/iso_manip.c:238                                               │
│  └─► iso_tree_add_new_node(volume, dir, eff_name, disk_path, node)     │
└─────────────────────────────────────────────────────────────────────────┘
                │
                ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  libisofs/tree.c:639  【第二道检查: ISO树层】                           │
│  └─► iso_image_truncate_name(image, name, &namept, 0)                  │
│       │                                                                 │
│       └─► libisofs/image.c:1120                                        │
│            └─► if(strlen(name) > image->truncate_length)  // 255字节   │
│                 └─► iso_truncate_rr_name()                             │
│                                                                         │
│  libisofs/tree.c:656                                                   │
│  └─► builder->create_node() → 创建 IsoNode                             │
│                                                                         │
│  libisofs/tree.c:671                                                   │
│  └─► iso_dir_insert(parent, new, pos, ISO_REPLACE_NEVER)              │
└─────────────────────────────────────────────────────────────────────────┘
                │
                │  (iso_node_is_valid_name 在设置节点名称时被调用)
                │
                ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  libisofs/node.c:1245  【名称有效性检查】                               │
│  └─► iso_node_is_valid_name(name)                                      │
│       ├─► if(name == NULL) return ISO_NULL_POINTER                     │
│       ├─► if(name[0] == '\0') goto rr_reserved                         │
│       ├─► if(strlen(name) > 255) return ISO_RR_NAME_TOO_LONG   ◄──关键 │
│       ├─► if(!strcmp(name, ".") || !strcmp(name, "..")) goto reserved  │
│       └─► if(strchr(name, '/') != NULL) goto rr_reserved               │
└─────────────────────────────────────────────────────────────────────────┘
                │
                │  (-commit 触发写入时)
                │
                ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  libisofs/joliet.c:1285  【Joliet树创建】                               │
│  └─► joliet_tree_create(t)                                             │
│       │                                                                 │
│       └─► joliet.c:644  create_tree(t, iso, &root, 0)                  │
│                │                                                        │
│                └─► joliet.c:242  get_joliet_name(t, iso, &jname)       │
│                     │                                                   │
│                     └─► iso_get_joliet_name()                          │
└─────────────────────────────────────────────────────────────────────────┘
                │
                ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  libisofs/joliet.c:33  【第三道检查: Joliet层】                         │
│  └─► iso_get_joliet_name(opts, charset, imgid, node_name, ...)         │
│       │                                                                 │
│       ├─► [L48] if(opts->joliet_utf16)                                 │
│       │         └─► str2utf16be(charset, node_name, &ucs_name)         │
│       │    else                                                         │
│       │         └─► str2ucs(charset, node_name, &ucs_name)             │
│       │                                                                 │
│       └─► [L77-82] Joliet 名称格式化                                   │
│            │                                                            │
│            ├─► [目录] iso_j_dir_id(ucs_name, joliet_long_names << 1)   │
│            │         └─► util.c:1299                                    │
│            │              ├─► maxchar = 64  (默认)                      │
│            │              └─► maxchar = 103 (joliet_long_names=1)  ◄── │
│            │                                                            │
│            └─► [文件] iso_j_file_id(ucs_name, flag)                    │
│                      └─► util.c:1216                                    │
│                           ├─► maxchar = 64  (默认)                      │
│                           └─► maxchar = 103 (joliet_long_names=1)  ◄── │
└─────────────────────────────────────────────────────────────────────────┘
                │
                ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  libisofs/util.c:1216  【Joliet文件名截断】                             │
│  └─► iso_j_file_id(src, flag)                                          │
│       │                                                                 │
│       ├─► [L1219] maxchar = 64                                         │
│       ├─► [L1228-1229] if(flag & 2) maxchar = 103   ◄── 关键限制       │
│       │                                                                 │
│       ├─► [L1231] dot = ucsrchr(src, '.')  找扩展名                    │
│       │                                                                 │
│       ├─► [L1239-1250] 计算 name 和 ext 的长度分配                     │
│       │    └─► 总长度不超过 maxchar                                    │
│       │                                                                 │
│       └─► [L1259-1287] 字符转换，无效字符替换为 '_'                    │
│                                                                         │
│  libisofs/util.c:1299  【Joliet目录名截断】                             │
│  └─► iso_j_dir_id(src, flag)                                           │
│       │                                                                 │
│       ├─► [L1301] maxchar = 64                                         │
│       ├─► [L1309-1310] if(flag & 2) maxchar = 103   ◄── 关键限制       │
│       │                                                                 │
│       └─► [L1313-1314] if(len > maxchar) len = maxchar  直接截断       │
└─────────────────────────────────────────────────────────────────────────┘
```

### 13.3 关键代码位置索引

| 检查阶段 | 文件:行号 | 函数 | 限制值 |
|---------|----------|------|--------|
| **xorriso 截断** | iso_manip.c:192 | Xorriso_tree_graft_node | 255 字节 |
| **ISO树截断** | tree.c:639 | iso_image_truncate_name | 255 字节 |
| **名称有效性** | node.c:1255 | iso_node_is_valid_name | 255 字节 |
| **Joliet 入口** | joliet.c:33 | iso_get_joliet_name | - |
| **Joliet 文件名** | util.c:1229 | iso_j_file_id | 64/103 字符 |
| **Joliet 目录名** | util.c:1310 | iso_j_dir_id | 64/103 字符 |
| **Joliet 路径** | joliet.c:247 | create_tree | 240 字节 |

### 13.4 执行流程时序图

```
时间 ──────────────────────────────────────────────────────────────────────►

     xorriso                    libisoburn              libisofs
        │                           │                       │
        │  -map ./ /                │                       │
        ├──────────────────────────►│                       │
        │                           │                       │
        │  Xorriso_option_map()     │                       │
        │  Xorriso_graft_in()       │                       │
        │                           │                       │
        │  [255字节截断检查]         │                       │
        │  iso_truncate_leaf_name() │                       │
        │                           │                       │
        │                           │  iso_tree_add_new_node()
        │                           ├──────────────────────►│
        │                           │                       │
        │                           │  [255字节截断检查]     │
        │                           │  iso_image_truncate_name()
        │                           │                       │
        │                           │  [名称有效性检查]      │
        │                           │  iso_node_is_valid_name()
        │                           │                       │
        │                           │  builder->create_node()
        │                           │  iso_dir_insert()     │
        │                           │◄──────────────────────┤
        │                           │                       │
        │  -commit                  │                       │
        ├──────────────────────────►│                       │
        │                           │                       │
        │                           │  joliet_writer_create()
        │                           ├──────────────────────►│
        │                           │                       │
        │                           │  joliet_tree_create() │
        │                           │  create_tree()        │
        │                           │                       │
        │                           │  [Joliet名称转换]     │
        │                           │  iso_get_joliet_name()│
        │                           │                       │
        │                           │  [103字符截断]        │
        │                           │  iso_j_file_id()      │
        │                           │  iso_j_dir_id()       │
        │                           │                       │
        │                           │  [路径长度检查]        │
        │                           │  max_path > 240 ?     │
        │                           │◄──────────────────────┤
        │                           │                       │
        │  写入 ISO 镜像            │                       │
        │◄──────────────────────────┤                       │
        │                           │                       │
```

### 13.5 三层检查机制总结

```
┌────────────────────────────────────────────────────────────────────────────┐
│                     文件名长度三层检查机制                                   │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  Layer 1: xorriso 层 (导入时)                                              │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │  位置: iso_manip.c:192  Xorriso_tree_graft_node()                  │   │
│  │  限制: xorriso->file_name_limit (默认 255 字节)                    │   │
│  │  处理: iso_truncate_leaf_name() 截断 + MD5 后缀                    │   │
│  │  编码: UTF-8                                                        │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                              │                                             │
│                              ▼                                             │
│  Layer 2: libisofs ISO树层 (节点创建时)                                    │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │  位置: tree.c:639      iso_image_truncate_name()                   │   │
│  │        node.c:1255     iso_node_is_valid_name()                    │   │
│  │  限制: LIBISOFS_NODE_NAME_MAX = 255 字节                           │   │
│  │  处理: 截断 + MD5 后缀 (mode=1) 或报错 (mode=0)                    │   │
│  │  编码: UTF-8                                                        │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                              │                                             │
│                              ▼                                             │
│  Layer 3: libisofs Joliet层 (写入时)                                       │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │  位置: util.c:1216     iso_j_file_id()                             │   │
│  │        util.c:1299     iso_j_dir_id()                              │   │
│  │        joliet.c:247    create_tree() [路径检查]                    │   │
│  │  限制: 64 字符 (标准) / 103 字符 (joliet_long_names)               │   │
│  │        240 字节路径 (标准) / 无限制 (joliet_long_paths)            │   │
│  │  处理: 直接截断                                                     │   │
│  │  编码: UCS-2 / UTF-16BE                                            │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
```
