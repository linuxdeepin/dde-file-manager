// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PINYINPROCESSOR_H
#define PINYINPROCESSOR_H

#include "service_textindex_global.h"

#include <QHash>
#include <QString>
#include <QStringList>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class PinyinProcessor
{
public:
    static PinyinProcessor &instance();

    // 字典查找顺序（依次尝试，首个加载成功的生效）：
    // 1. XDG 用户数据目录：$XDG_DATA_HOME/deepin/dde-file-manager/pinyin.txt
    //    （默认 ~/.local/share，支持 OEM/用户自定义覆盖）
    // 2. 系统安装目录：PINYIN_DICT_INSTALL_DIR/pinyin.txt（路径由 CMake 注入）
    static QStringList dictionaryLookupPaths();

    bool loadDictionary(const QString &path);
    bool isLoaded() const;

    // 将文件名转换为拼音全拼和首字母缩写（对齐 deepin-anything 的行为）：
    // - 遍历每个 UTF-16 字符，在字典中找到的第一个汉字拼音（去声调）拼接进 pinyinFull，
    //   其首字母拼接进 pinyinAcronym。
    // - 不在字典中的字符（英文、数字、符号）原样拼入两者（保持 anything 一致）。
    // - 输出整体小写化（与 anything 写入索引时 toLower 一致）。
    void convertToPinyin(const QString &fileName,
                         QString &pinyinFull,
                         QString &pinyinAcronym) const;

private:
    PinyinProcessor();

    QString removeTone(const QString &pinyin) const;

    QHash<QString, QString> m_pinyinMap;   // 汉字（UTF-16，1 或 surrogate pair 2 个单元）→ 第一个拼音（已去声调）
    bool m_loaded { false };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // PINYINPROCESSOR_H
