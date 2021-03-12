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

#include "properties.h"

#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDebug>

/**
 * @brief Creates properties
 * @param fileName
 * @param group
 */
Properties::Properties(const QString &fileName, const QString &group)
{
    if (!fileName.isEmpty()) {
        load(fileName, group);
    }
}
//---------------------------------------------------------------------------

/**
 * @brief Creates properties
 * @param other properies
 */
Properties::Properties(const Properties &other)
    : data(other.data)
{

}
//---------------------------------------------------------------------------


/**
 * @brief Loads property file
 * @param fileName
 * @param group
 * @return true if load was successful
 */
bool Properties::load(const QString &fileName, const QString &group)
{

    // NOTE: This class is used for reading of property files instead of QSettings
    // class, which considers separator ';' as comment

    // Try open file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    // Clear old data
    data.clear();

    // Indicator whether group was found or not, if name of group was not
    // specified, groupFound is always true
    bool groupFound = group.isEmpty();

    // Read propeties
    QTextStream in(&file);
    while (!in.atEnd()) {

        // Read new line
        QString line = in.readLine();

        // Skip empty line or line with invalid format
        if (line.trimmed().isEmpty()) {
            continue;
        }

        // Read group
        // NOTE: symbols '[' and ']' can be found not only in group names, but
        // only group can start with '['
        if (!group.isEmpty() && line.trimmed().startsWith("[")) {
            QString tmp = line.trimmed().replace("[", "").replace("]", "");
            groupFound = group.trimmed().compare(tmp) == 0;
        }

        // If we are in correct group and line contains assignment then read data
        int first_equal = line.indexOf('=');

        if (groupFound && first_equal >= 0) {
            data.insert(line.left(first_equal).trimmed(), line.mid(first_equal + 1).trimmed());
        }
    }
    file.close();

    return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Saves properties to file
 * @param fileName
 * @param group
 * @return true if success
 */
bool Properties::save(const QString &fileName, const QString &group)
{

    // Try open file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // Write group
    QTextStream out(&file);
    if (!group.isEmpty()) {
        out << "[" + group + "]\n";
    }

    // Write data
    foreach (QString key, data.keys()) {
        out << key << "=" << data.value(key).toString() << "\n";
    }

    // Exit
    file.close();
    return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns true if property with given key is present in properties
 * @param key
 * @return true if property with given key is present in properties
 */
bool Properties::contains(const QString &key) const
{
    return data.contains(key);
}
//---------------------------------------------------------------------------

/**
 * @brief Returns value
 * @param key
 * @param defaultValue
 * @return value
 */
QVariant Properties::value(const QString &key, const QVariant &defaultValue)
{
    return data.value(key, defaultValue);
}
//---------------------------------------------------------------------------

/**
 * @brief Sets value to properties
 * @param key
 * @param value
 */
void Properties::set(const QString &key, const QVariant &value)
{
    if (data.contains(key)) {
        data.take(key);
    }
    data.insert(key, value);
}
//---------------------------------------------------------------------------

/**
 * @brief Returns keys (names of properties)
 * @param key
 * @param value
 */
QStringList Properties::getKeys() const
{
    return data.keys();
}
//---------------------------------------------------------------------------
