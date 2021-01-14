#ifndef DOCTOTEXT_DATA_STREAM_H
#define DOCTOTEXT_DATA_STREAM_H

#include <stdio.h>
#include <string>

namespace doctotext {
class DataStream
{
	public:
		virtual ~DataStream(){}
		virtual bool open() = 0;
		virtual bool close() = 0;
		virtual bool read(void* data, int element_size, size_t elements_num) = 0;
		virtual bool seek(int offset, int whence) = 0;
		virtual bool eof() = 0;
		virtual int getc() = 0;
		virtual bool unGetc(int ch) = 0;
		virtual size_t size() = 0;
		virtual size_t tell() = 0;
		virtual std::string name() = 0;
		virtual DataStream* clone() = 0;
};

class FileStream : public DataStream
{
	struct Implementation;
	Implementation* impl;
	public:
		FileStream(const std::string& file_name);
		~FileStream();
		bool open();
		bool close();
		bool read(void* data, int element_size, size_t elements_num);
		bool seek(int offset, int whence);
		bool eof();
		int getc();
		bool unGetc(int ch);
		size_t size();
		size_t tell();
		std::string name();
		DataStream* clone();
};

class BufferStream : public DataStream
{
	struct Implementation;
	Implementation* impl;
	public:
		BufferStream(const char* buffer, size_t size);
		~BufferStream();
		bool open();
		bool close();
		bool read(void* data, int element_size, size_t elements_num);
		bool seek(int offset, int whence);
		bool eof();
		int getc();
		bool unGetc(int ch);
		size_t size();
		size_t tell();
		std::string name();
		DataStream* clone();
};
}
#endif	//DOCTOTEXT_DATA_STREAM_H
