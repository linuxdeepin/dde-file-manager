/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include <ContribInc.h>
#include <TermAttribute.h>
#include <OffsetAttribute.h>
#include <Reader.h>
#include <CharFolder.h>
#include <MiscUtils.h>
#include <UnicodeUtils.h>

#include "chinesetokenizer.h"

namespace Lucene {

const int32_t ChineseTokenizer::kMaxWordLen = 255;
const int32_t ChineseTokenizer::kIoBufferSize = 1024;

ChineseTokenizer::ChineseTokenizer(const ReaderPtr &input)
    : Tokenizer(input)
{
}

ChineseTokenizer::ChineseTokenizer(const AttributeSourcePtr &source, const ReaderPtr &input)
    : Tokenizer(source, input)
{
}

ChineseTokenizer::ChineseTokenizer(const AttributeFactoryPtr &factory, const ReaderPtr &input)
    : Tokenizer(factory, input)
{
}

ChineseTokenizer::~ChineseTokenizer()
{
}

void ChineseTokenizer::initialize()
{
    offset = 0;
    bufferIndex = 0;
    dataLen = 0;
    buffer = CharArray::newInstance(kMaxWordLen);
    memset(buffer.get(), 0, kMaxWordLen);
    ioBuffer = CharArray::newInstance(kIoBufferSize);
    memset(ioBuffer.get(), 0, kIoBufferSize);
    length = 0;
    start = 0;

    termAtt = addAttribute<TermAttribute>();
    offsetAtt = addAttribute<OffsetAttribute>();
}

void ChineseTokenizer::push(wchar_t c)
{
    if (length == 0) {
        start = offset - 1;   // start of token
    }
    buffer[length++] = CharFolder::toLower(c);   // buffer it
}

bool ChineseTokenizer::flush()
{
    if (length > 0) {
        termAtt->setTermBuffer(buffer.get(), 0, length);
        offsetAtt->setOffset(correctOffset(start), correctOffset(start + length));
        return true;
    } else {
        return false;
    }
}

bool ChineseTokenizer::incrementToken()
{
    clearAttributes();

    length = 0;
    start = offset;

    bool last_is_en = false, last_is_num = false;
    while (true) {
        wchar_t c;
        ++offset;

        if (bufferIndex >= dataLen) {
            dataLen = input->read(ioBuffer.get(), 0, ioBuffer.size());
            bufferIndex = 0;
        }

        if (dataLen == -1) {
            --offset;
            return flush();
        } else {
            c = ioBuffer[bufferIndex++];
        }

        if (UnicodeUtil::isLower(c) || UnicodeUtil::isUpper(c)) {
            if (last_is_num) {
                --bufferIndex;
                --offset;
                return flush();
            }

            push(c);
            if (length == kMaxWordLen) {
                return flush();
            }
            last_is_en = true;
        } else if (UnicodeUtil::isDigit(c)) {
            if (last_is_en) {
                --bufferIndex;
                --offset;
                return flush();
            }

            push(c);
            if (length == kMaxWordLen) {
                return flush();
            }
            last_is_num = true;
        } else if (UnicodeUtil::isOther(c)) {
            if (length > 0) {
                --bufferIndex;
                --offset;
                return flush();
            }
            push(c);
            return flush();
        } else if (length > 0) {
            return flush();
        }
    }
}

void ChineseTokenizer::end()
{
    // set final offset
    int32_t finalOffset = correctOffset(offset);
    offsetAtt->setOffset(finalOffset, finalOffset);
}

void ChineseTokenizer::reset()
{
    Tokenizer::reset();
    offset = 0;
    bufferIndex = 0;
    dataLen = 0;
}

void ChineseTokenizer::reset(const ReaderPtr &input)
{
    Tokenizer::reset(input);
    reset();
}

}
