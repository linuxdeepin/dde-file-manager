/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHINESETOKENIZER_H
#define CHINESETOKENIZER_H

#include <Tokenizer.h>

/**
 * An tokenizer that tokenizes chinese
 * Only used for Lucene++
 */
namespace Lucene {
class ChineseTokenizer : public Tokenizer
{
public:
    explicit ChineseTokenizer(const ReaderPtr &input);
    ChineseTokenizer(const AttributeSourcePtr &source, const ReaderPtr &input);
    ChineseTokenizer(const AttributeFactoryPtr &factory, const ReaderPtr &input);

    virtual ~ChineseTokenizer();

    LUCENE_CLASS(ChineseTokenizer);

protected:
    /// Max word length
    static const int32_t kMaxWordLen;

    static const int32_t kIoBufferSize;

protected:
    /// word offset, used to imply which character(in) is parsed
    int32_t offset;

    /// the index used only for ioBuffer
    int32_t bufferIndex;

    /// data length
    int32_t dataLen;

    /// character buffer, store the characters which are used to compose the returned Token
    CharArray buffer;

    /// I/O buffer, used to store the content of the input (one of the members of Tokenizer)
    CharArray ioBuffer;

    TermAttributePtr termAtt;
    OffsetAttributePtr offsetAtt;

    int32_t length;
    int32_t start;

public:
    virtual void initialize();
    virtual bool incrementToken();
    virtual void end();
    virtual void reset();
    virtual void reset(const ReaderPtr &input);

protected:
    void push(wchar_t c);
    bool flush();
};
}

#endif   // CHINESETOKENIZER_H
