/*
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



#include "dfmglobal.h"
#include "filebatchprocess.h"

#include <regex>
#include <string>
#include <locale>
#include <bitset>
#include <iostream>
#include <type_traits>

#include <QDebug>
#include <QFileInfo>
#include <QByteArray>



std::once_flag FileBatchProcess::flag;



union EndianTest
{
    std::int16_t number;
    char array[2];
};

///###: here, We can jundge wheather the CPU is big-endian or little-endian in compile-time.
static constexpr EndianTest test{ 0x1001 };
inline constexpr bool isBigEndian()noexcept{ return (test.array[0] == 0x10);}
inline constexpr bool isLittleEndian()noexcept{ return (test.array[0] == 0x01); }



QSharedMap<DUrl, DUrl> FileBatchProcess::replaceText(const QList<DUrl>& originUrls, const QPair<QString, QString> &pair) const
{
    if(originUrls.isEmpty() == true){ //###: here, jundge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    QSharedMap<DUrl, DUrl> result{ new QMap<DUrl, DUrl>{}};

    for(auto url : originUrls){

        QFileInfo info{ url.toLocalFile() };

        ///###: symlink is also processed here.
        if(info.isFile()){
            QString fileName{ info.baseName() + QString{"."} + info.completeSuffix() };
            fileName.replace(pair.first, pair.second);

            QByteArray u8FileName{ fileName.toUtf8() };
            std::size_t sizeOfName{ u8FileName.size() };

            if( sizeOfName > MAX_FILE_NAME_CHAR_COUNT ){
                u8FileName.resize(MAX_FILE_NAME_CHAR_COUNT);
                u8FileName = FileBatchProcess::cutString(u8FileName);

            }

            DUrl changedUrl{ DUrl::fromLocalFile(info.path() + QString{"/"} + QString::fromUtf8(u8FileName)) };
            result->insert(url, changedUrl);
            continue;

        }

        if(info.isDir()){
            QString dirName{ info.baseName() };
            dirName.replace(pair.first, pair.second);

            QByteArray u8DirName{ dirName.toUtf8() };
            std::size_t sizeOfName{ u8DirName.size() };

            if(sizeOfName > MAX_FILE_NAME_CHAR_COUNT){
                u8DirName.resize(MAX_FILE_NAME_CHAR_COUNT);
                u8DirName = FileBatchProcess::cutString(u8DirName);
            }

            DUrl changedUrl{ DUrl::fromLocalFile(info.path() + QString{"/"} + QString::fromUtf8(u8DirName)) };
            result->insert(url, changedUrl);
            continue;
        }
    }

    return result;
}

QSharedMap<DUrl, DUrl> FileBatchProcess::addText(const QList<DUrl> &originUrls, const QPair<QString, DFileService::AddTextFlags>& pair) const
{
    if(originUrls.isEmpty()){  //###: here, jundge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    QSharedMap<DUrl, DUrl> result{ new QMap<DUrl, DUrl>{} };

    for(auto url : originUrls){
        QFileInfo info(url.toLocalFile());

        if(pair.second == DFileService::AddTextFlags::Before){ //###: insert string in the front of filename.

            if(info.isFile()){  //aim at files.

                QByteArray fileBaseName{ info.baseName().toUtf8() };
                QByteArray addedStr{ pair.first.toUtf8() };
                QByteArray suffixStr{ info.completeSuffix().toUtf8() };
                std::size_t sizeOfSuffix{ suffixStr.size() + 1 }; //plus 1 because dot(.)
                std::size_t sizeOfBaseName{ fileBaseName.size() };
                std::size_t sizeOfAddedStr{ pair.first.toUtf8().size() };


                if(sizeOfAddedStr >= MAX_FILE_NAME_CHAR_COUNT){
                    addedStr.resize(MAX_FILE_NAME_CHAR_COUNT - sizeOfSuffix);

                    QString fileName{ FileBatchProcess::cutString( addedStr ) };
                    fileName += QString{ "." };
                    fileName += QString::fromUtf8(suffixStr);

                    DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + fileName ) };
                    result->insert(url, beModifiedUrl);
                    continue;

                }else{

                    if( sizeOfAddedStr + sizeOfBaseName + sizeOfSuffix <= MAX_FILE_NAME_CHAR_COUNT){
                        QByteArray fileName{ addedStr + fileBaseName + QByteArray{"."} + suffixStr };

                        DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{ "/" } + QString::fromUtf8(fileName) ) };
                        result->insert(url, beModifiedUrl);
                        continue;

                    }else{ //###: >MAX_FILE_NAME_CHAR_COUNT

                        QByteArray fileName{ addedStr + fileBaseName };
                        fileName.resize(MAX_FILE_NAME_CHAR_COUNT - sizeOfSuffix);
                        fileName = FileBatchProcess::cutString(fileName);

                        fileName += QByteArray{"."};
                        fileName += suffixStr;

                        DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileName) ) };
                        result->insert(url, beModifiedUrl);
                        continue;
                    }

                }
            }

            if(info.isDir()){ //aim at dirs.

                QByteArray fileBaseName{ info.baseName().toUtf8() };
                QByteArray addedStr{ pair.first.toUtf8() };
                std::size_t sizeOfBaseName{ fileBaseName.size() };
                std::size_t sizeOfAddedStr{ addedStr.size() };

                if(sizeOfAddedStr >= MAX_FILE_NAME_CHAR_COUNT){
                    addedStr.resize(MAX_FILE_NAME_CHAR_COUNT);
                    addedStr = FileBatchProcess::cutString(addedStr);

                    QString fileName{ QString::fromUtf8(addedStr ) };
                    DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + fileName ) };
                    result->insert(url, beModifiedUrl);
                    continue;

                }else{

                    if( sizeOfAddedStr + sizeOfBaseName <= MAX_FILE_NAME_CHAR_COUNT){
                        QByteArray fileName{ addedStr + fileBaseName };

                        DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileName) ) };
                        result->insert(url, beModifiedUrl);
                        continue;

                    }else{ //###: >MAX_FILE_NAME_CHAR_COUNT
                        QByteArray fileName{ addedStr + fileBaseName };
                        fileName.resize(MAX_FILE_NAME_CHAR_COUNT);
                        fileName = FileBatchProcess::cutString(fileName);

                        DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() +  QString{"/"} + QString::fromUtf8(fileName) ) };
                        result->insert(url, beModifiedUrl);
                        continue;
                    }

                }


            }


        }else if(pair.second == DFileService::AddTextFlags::After){ //###: append string to filename.

            if(info.isFile()){
                QByteArray fileBaseName{ info.baseName().toUtf8() };
                QByteArray addedStr{ pair.first.toUtf8() };
                std::size_t sizeOfSuffix{ info.completeSuffix().toUtf8().size() + 1 }; //plus 1 because dot(.)
                std::size_t sizeOfBaseName{ fileBaseName.size() };
                std::size_t sizeOfAddedStr{ addedStr.size() };

                if(sizeOfAddedStr >= MAX_FILE_NAME_CHAR_COUNT){
                    QByteArray fileName{ fileBaseName + addedStr };
                    fileName.resize(MAX_FILE_NAME_CHAR_COUNT - sizeOfSuffix);
                    fileName = FileBatchProcess::cutString(fileName);

                    fileName += QByteArray{ "." };
                    fileName += info.completeSuffix().toUtf8();

                    DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileName) ) };
                    result->insert(url, beModifiedUrl);
                    continue;

                }else{

                    if( sizeOfBaseName + sizeOfAddedStr + sizeOfSuffix <= MAX_FILE_NAME_CHAR_COUNT){
                        QByteArray fileName{ fileBaseName + addedStr + QByteArray{"."} + info.completeSuffix().toUtf8() };

                        DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{ "/" } + QString::fromUtf8(fileName) ) };
                        result->insert(url, beModifiedUrl);
                        continue;

                    }else{ //###: >MAX_FILE_NAME_CHAR_COUNT

                        QByteArray fileName{ fileBaseName + addedStr};
                        fileName.resize(MAX_FILE_NAME_CHAR_COUNT - sizeOfSuffix);
                        fileName += QByteArray{ "." };
                        fileName += info.completeSuffix().toUtf8();

                        DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QByteArray{"/"} + QString::fromUtf8(fileName) ) };
                        result->insert(url, beModifiedUrl);
                        continue;
                    }

                }

            }

            if(info.isDir()){
                QByteArray fileBaseName{ info.baseName().toUtf8() };
                QByteArray addedStr{ pair.first.toUtf8() };
                std::size_t sizeOfBaseName{ fileBaseName.size() };
                std::size_t sizeOfAddedStr{ addedStr.size() };

                if(sizeOfAddedStr >= MAX_FILE_NAME_CHAR_COUNT){
                    addedStr.resize(MAX_FILE_NAME_CHAR_COUNT);
                    QByteArray fileName{ fileBaseName + addedStr };
                    fileName.resize(MAX_FILE_NAME_CHAR_COUNT);

                    fileName = FileBatchProcess::cutString(fileName);

                    DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileName) ) };
                    result->insert(url, beModifiedUrl);
                    continue;

                }else{

                    if( sizeOfBaseName + sizeOfAddedStr <= MAX_FILE_NAME_CHAR_COUNT){ //###: +1 stand for '/'
                        QByteArray fileName{ addedStr + fileBaseName };

                        DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileName) ) };
                        result->insert(url, beModifiedUrl);
                        continue;

                    }else{ //###: >MAX_FILE_NAME_CHAR_COUNT
                        QByteArray fileName{ addedStr + fileBaseName};
                        fileName.resize(MAX_FILE_NAME_CHAR_COUNT); //###: Why minus 1? because '/'

                        fileName == FileBatchProcess::cutString(fileName);

                        DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() +  QString{"/"} + QString::fromUtf8(fileName) ) };
                        result->insert(url, beModifiedUrl);
                        continue;

                    }

                }
            }

        }
    }


    return result;
}

QSharedMap<DUrl, DUrl> FileBatchProcess::customText(const QList<DUrl> &originUrls, const QPair<QString, QString> &pair) const
{
    if(originUrls.isEmpty() == true || pair.first.isEmpty() == true || pair.second.isEmpty() == true){  //###: here, jundge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    unsigned long long SNNumber{ std::stoull(pair.second.toStdString()) };
    unsigned long long index{ 0 };
    if(SNNumber == 0xffffffffffffffff){  //##: Maybe, this value will be equal to the max value of the type of unsigned long long
        index = SNNumber - originUrls.size();
    }

    index = SNNumber;
    QSharedMap<DUrl, DUrl> result{new QMap<DUrl, DUrl>{}};
    for(auto url : originUrls){
        QFileInfo info{url.toLocalFile()};

        if(info.isFile()){ //###:aim at file.

            QByteArray fileBaseName{ pair.first.toUtf8() };
            QByteArray indexStr{ QByteArray::fromStdString( std::to_string( index ) ) };
            std::size_t sizeOfSuffixStr{ info.completeSuffix().toUtf8().size() + 1 };
            std::size_t sizeOfBaseName{ pair.first.toUtf8().size() };
            std::size_t sizeOfIndexStr{ indexStr.size() };



            if( sizeOfBaseName >= MAX_FILE_NAME_CHAR_COUNT){
                std::size_t n{MAX_FILE_NAME_CHAR_COUNT - sizeOfIndexStr - sizeOfSuffixStr };
                fileBaseName.resize(n);

                QByteArray fileName{ FileBatchProcess::cutString(fileBaseName) };
                fileName += indexStr;
                fileName += QByteArray{"."};
                fileName += info.completeSuffix().toUtf8();

                DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileName) ) };
                result->insert(url, beModifiedUrl);

            }else{

                if(sizeOfBaseName + sizeOfIndexStr + sizeOfSuffixStr <= MAX_FILE_NAME_CHAR_COUNT){

                    QByteArray fileName{ fileBaseName + indexStr + QByteArray{"."} + info.completeSuffix().toUtf8() };

                    DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileName)) };
                    result->insert(url, beModifiedUrl);

                }else{

                    std::size_t n{ sizeOfIndexStr + sizeOfSuffixStr - MAX_FILE_NAME_CHAR_COUNT };
                    fileBaseName.resize(fileBaseName.size() - n);

                    QByteArray fileName{ FileBatchProcess::cutString(fileBaseName) };
                    fileName += indexStr;
                    fileName += QByteArray{"."};
                    fileName += info.completeSuffix().toUtf8();

                    DUrl beModifieddUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileName)) };
                    result->insert(url, beModifieddUrl);

                }


            }

            ++index;
            continue;
        }

        if(info.isDir()){  //###: aim at dir
            QByteArray indexStr{ QByteArray::fromStdString( std::to_string(index) ) };
            QByteArray basenameOfFile{ pair.first.toUtf8() };
            std::size_t sizeOfIndexStr{ indexStr.size() };
            std::size_t sizeOfBaseName{ basenameOfFile.size() };


            if(sizeOfBaseName >= MAX_FILE_NAME_CHAR_COUNT){

                basenameOfFile.resize(MAX_FILE_NAME_CHAR_COUNT - sizeOfIndexStr);
                QByteArray fileBaseName{ FileBatchProcess::cutString(basenameOfFile) };
                fileBaseName += indexStr;

                DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileBaseName) ) };
                result->insert(url, beModifiedUrl);

            }else{

                if(sizeOfBaseName + sizeOfIndexStr <= MAX_FILE_NAME_CHAR_COUNT){

                    QByteArray newFileBaseName{ basenameOfFile + indexStr };

                    DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(newFileBaseName)) };
                    result->insert(url, beModifiedUrl);

                }else{

                    std::size_t n{ sizeOfBaseName + sizeOfIndexStr - MAX_FILE_NAME_CHAR_COUNT };
                    basenameOfFile.resize(basenameOfFile.size() - n);
                    QByteArray fileBaseName{ FileBatchProcess::cutString(basenameOfFile) };
                    fileBaseName += indexStr;

                    DUrl beModifiedUrl{ DUrl::fromLocalFile( info.path() + QString{"/"} + QString::fromUtf8(fileBaseName) ) };
                    result->insert(url, beModifiedUrl);
                }
            }

            ++index;
            continue;
        }
    }

    return result;
}



////###: use the value of map to rename the file who name is the key of map.
QSharedPointer<QList<DUrl>> FileBatchProcess::batchProcessFile(const QSharedMap<DUrl, DUrl> &map)
{
    if(static_cast<bool>(map) == false){  //###: here, jundge whether there are fileUrls in map.
        return QSharedPointer<QList<DUrl>>{ nullptr };
    }


    QMap<DUrl, DUrl>::const_iterator beg = map->constBegin();
    QMap<DUrl, DUrl>::const_iterator end = map->constEnd();

    QSharedPointer<QList<DUrl>> cache{ new QList<DUrl>{} };

    for(; beg != end; ++beg){

        QString originName{ beg.key().toLocalFile() };
        QString changedName{ beg.value().toLocalFile() };

        if(originName == changedName){
            continue;
        }

       QString currentName{ beg.key().toLocalFile() };
       QString hopedName{ beg.value().toLocalFile() };

       ///###: just cache files that rename successfully.
       if( QFile::rename(currentName, hopedName) == true ){
           cache->push_back( DUrl::fromLocalFile(hopedName) );
       }
    }

    return cache;
}


QByteArray FileBatchProcess::cutString(const QByteArray &text)
{
    if(text.isEmpty() == true){
        return QByteArray{};
    }

    std::basic_string<char> u8Str{ text.toStdString() };
    std::basic_string<char>::const_reverse_iterator crbegin{ u8Str.crbegin() };
    std::basic_string<char>::const_reverse_iterator crend{ u8Str.crend() };

    std::size_t counter{ 0 };

    if(isLittleEndian() == true){
        std::bitset<8> bits{ static_cast<unsigned long>(*crbegin) };
        if(bits.test(7) == false){ //###: I jundge the last byte of u8Str. If it is 0xxx xxxx means it is a ASCII, so we return text directly.
            return text;
        }

        for(; crbegin != crend; ++crbegin, ++counter){  //###: 0xc0(11000000), I find the byte of flag of utf8 through 0xc0.
            if((static_cast<unsigned char>(*crbegin) & 0xc0) == 0xc0){
                ++counter;
                break;
            }
        }

       std::bitset<8> firstByteOfWard{ static_cast<unsigned long>(*crbegin) };
       std::string bitsStr{ firstByteOfWard.to_string() };

       std::basic_regex<char> regex{ "([1]+)([0]{1,1})" }; //###: here, Use regex to find how many bytes the last word in u8Str.
       std::match_results<std::basic_string<char>::const_iterator> result;

       if( std::regex_search(bitsStr, result, regex) == true ){
           std::size_t bitsNumber{ result.str(1).size() };
           if(bitsNumber == counter){ //###: I jundge that whether the last word of u8Str is complete.
               return text;

           }else{ //###: incomplete! I cut the last word of u8Str.
               std::basic_string<char> beCutedString{ u8Str.erase(u8Str.size()-counter, counter) };
               return QByteArray::fromStdString(beCutedString);
           }
       }

    }else{
        std::bitset<8> bits{ static_cast<unsigned long>(*crbegin) };
        if(bits.test(0) == false){
            return text;
        }

        for(; crbegin != crend; ++crbegin, ++counter){
            if((*crbegin & 0x03) == 0x03){
                ++counter;
                break;
            }
        }

        std::bitset<8> firstByteOfWord{ static_cast<unsigned long>(*crbegin) };
        std::string bitsStr{ firstByteOfWord.to_string() };

        std::basic_regex<char> regex{ "([0]{1,1})([1]+)" };
        std::match_results<std::basic_string<char>::const_iterator> result;

        if( std::regex_search(bitsStr, result, regex) == true ){

            std::size_t bitsNumber{ result.str(2).size() };
            if(bitsNumber == counter){
                return text;

            }else{

                std::basic_string<char> beCutedString{ u8Str.erase(u8Str.size()-counter, counter) };
                return QByteArray::fromStdString(beCutedString);
            }
        }
    }


}




