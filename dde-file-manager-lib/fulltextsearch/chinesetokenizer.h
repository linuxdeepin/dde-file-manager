#ifndef CHINESETOKENIZER_H
#define CHINESETOKENIZER_H

#include <Tokenizer.h>

namespace Lucene {
class ChineseTokenizer: public Tokenizer
{
public:
    explicit ChineseTokenizer(const ReaderPtr &input);
    ChineseTokenizer(const AttributeSourcePtr &source, const ReaderPtr &input);
    ChineseTokenizer(const AttributeFactoryPtr &factory, const ReaderPtr &input);

    virtual ~ChineseTokenizer();

    LUCENE_CLASS(ChineseTokenizer);

protected:
    /// Max word length
    static const int32_t MAX_WORD_LEN;

    static const int32_t IO_BUFFER_SIZE;

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

#endif // CHINESETOKENIZER_H
