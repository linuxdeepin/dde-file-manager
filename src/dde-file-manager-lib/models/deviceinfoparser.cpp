/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
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

#include "deviceinfoparser.h"
#include <QProcess>
#include "Logger.h"


const QString Deviceype_Computer = "Computer";
const QString Devicetype_Name = "Name";
const QString Devicetype_lshw_Class_Prefix = "*-";
const char Devicetype_Separator = ':';
const QString Devicetype_Stitching_Symbol = "_";


DCORE_USE_NAMESPACE
DeviceInfoParser::DeviceInfoParser(): QObject()
{

}

DeviceInfoParser::~DeviceInfoParser()
{

}

void DeviceInfoParser::refreshDabase()
{
    loadLshwDatabase();

    emit loadFinished();
}

const QString &DeviceInfoParser::queryData(const QString &toolname, const QString &firstKey, const QString &secondKey)
{
    static QString result("");
    if (false == m_toolDatabase.contains(toolname)) {
        return result;
    }

    if (false == m_toolDatabase[toolname].contains(firstKey)) {
        return result;
    }

    if (false == m_toolDatabase[toolname][firstKey].contains(secondKey)) {
        return result;
    }

    return m_toolDatabase[toolname][firstKey][secondKey];
}

QStringList DeviceInfoParser::getLshwCDRomList()
{
    QStringList cdromList;

    if (false == m_toolDatabaseSecondOrder.contains("lshw")) {
        return cdromList;
    }

    foreach (const QString &fk, m_toolDatabaseSecondOrder["lshw"]) {
        if (fk.contains("cdrom")) {
            cdromList.push_back(fk);
            continue;
        }
    }

    return cdromList;
}

bool DeviceInfoParser::isInternalDevice(const QString &device)
{
    QStringList cdromList = getLshwCDRomList();

    bool isInternalDevice = true;
    foreach (QString cdrom, cdromList) {
        const QString logicalName = queryData("lshw", cdrom, "logical name");
        if (logicalName.contains(device)) {
            if (!cdrom.contains("sata")) {
                isInternalDevice = false;
            }
        }
    }
    return isInternalDevice;
}

bool DeviceInfoParser::loadLshwDatabase()
{
    if (false == executeProcess("lshw")) {
        return false;
    }

    QString lshwOut = m_standOutput;

    // lshw
    DatabaseMap lshwDatabase_;
    QStringList secondOrder;

    int startIndex = 0;
    int lineNumber = -1;
    QStringList deviceType;
    QMap<QString, QString> DeviceInfoMap;

    for (int i = 0; i < lshwOut.size(); ++i) {
        if (lshwOut[i] != '\n' && i != lshwOut.size() - 1) {
            continue;
        }

        ++lineNumber;

        QString line = lshwOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if (line.trimmed().isEmpty()) {
            dWarning("DeviceInfoParser::loadLshwDatabase lshw output contains empty line!");
            continue;
        }

        if (lineNumber == 0) {
            DeviceInfoMap[Devicetype_Name] = line.trimmed();
            deviceType.push_back(Devicetype_lshw_Class_Prefix + Deviceype_Computer);
            continue;
        }

        if (line.contains(Devicetype_lshw_Class_Prefix)) {
            QString deviceTypeName;
            foreach (auto dt, deviceType) {
                if (deviceTypeName.isEmpty() == false) {
                    deviceTypeName += Devicetype_Stitching_Symbol;
                }
                deviceTypeName += dt.trimmed().remove(Devicetype_lshw_Class_Prefix);
                continue;
            }

            lshwDatabase_[deviceTypeName] = DeviceInfoMap;
            secondOrder.push_back(deviceTypeName);

            DeviceInfoMap.clear();

            QString typeStr =  line;
            DeviceInfoMap["Type"] = typeStr.remove(Devicetype_lshw_Class_Prefix).trimmed();

            while (deviceType.size() > 0) {
                if (deviceType.last().indexOf(Devicetype_lshw_Class_Prefix) >= line.indexOf(Devicetype_lshw_Class_Prefix)) {
                    deviceType.pop_back();
                } else {
                    break;
                }
            }

            if (line.contains(Devicetype_Separator)) {
                QStringList strList = line.split(Devicetype_Separator);
                if (DeviceInfoMap.contains(strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix))) {
                    DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] += ", ";
                    DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] += strList.last().trimmed();
                } else {
                    DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
                }
            }
            deviceType.push_back(line);
            continue;
        }

        int index = line.indexOf(Devicetype_Separator);
        if (index > 0) {
            QString name = line.mid(0, index).trimmed().remove(Devicetype_lshw_Class_Prefix);
            if (name == "configuration" || name == "resources") {
                QChar splitChar = name == "configuration" ? '=' : ':';

                QStringList lst = line.mid(index + 1).trimmed().split(splitChar);
                if (lst.size() < 2) {
                    if (DeviceInfoMap.contains(name)) {
                        DeviceInfoMap[name] += ", ";
                        DeviceInfoMap[name] += line.mid(index + 1).trimmed();
                    } else {
                        DeviceInfoMap[name] = line.mid(index + 1).trimmed();
                    }

                } else {
                    for (int ind = 0; ind < lst.size() - 1; ++ind) {
                        QString tempName = lst[ind].split(" ").last();
                        int spaceIndex = lst[ind + 1].lastIndexOf(" ");
                        if (spaceIndex < 0) {
                            if (DeviceInfoMap.contains(tempName)) {
                                DeviceInfoMap[tempName] += ", ";
                                DeviceInfoMap[tempName] += lst[ind + 1];
                            } else {
                                DeviceInfoMap[tempName] = lst[ind + 1];
                            }
                        } else {
                            if (DeviceInfoMap.contains(tempName)) {
                                DeviceInfoMap[tempName] += ", ";
                                DeviceInfoMap[tempName] += lst[ind + 1].mid(0, spaceIndex);
                            } else {
                                DeviceInfoMap[tempName] = lst[ind + 1].mid(0, spaceIndex);
                            }
                        }
                    }
                }
            } else {
                if (DeviceInfoMap.contains(name)) {
                    DeviceInfoMap[name] += ", ";
                    DeviceInfoMap[name] += line.mid(index + 1).trimmed();
                } else {
                    DeviceInfoMap[name] = line.mid(index + 1).trimmed();
                }
            }
            continue;
        }
    }

    //last device
    {
        QString deviceTypeName;
        foreach (auto deviceType, deviceType) {
            if (deviceTypeName.isEmpty() == false) {
                deviceTypeName += Devicetype_Stitching_Symbol;
            }
            deviceTypeName += deviceType.trimmed().remove(Devicetype_lshw_Class_Prefix);
        }

        lshwDatabase_[deviceTypeName] = DeviceInfoMap;
        secondOrder.push_back(deviceTypeName);
    }

    m_toolDatabase["lshw"] = lshwDatabase_;
    secondOrder.removeDuplicates();
    m_toolDatabaseSecondOrder["lshw"] = secondOrder;

    return true;
}

bool DeviceInfoParser::executeProcess(const QString &cmd)
{
    QProcess process;
    process.start(cmd);
    bool res = process.waitForFinished(10000);
    m_standOutput = process.readAllStandardOutput();

    return res;
}

