/****************************************************************************
**
** DocToText - Converts DOC, XLS, XLSB, PPT, RTF, ODF (ODT, ODS, ODP),
**             OOXML (DOCX, XLSX, PPTX), iWork (PAGES, NUMBERS, KEYNOTE),
**             ODFXML (FODP, FODS, FODT), PDF, EML and HTML documents to plain text.
**             Extracts metadata and annotations.
**
** Copyright (c) 2006-2013, SILVERCODERS(R)
** http://silvercoders.com
**
** Project homepage: http://silvercoders.com/en/products/doctotext
**
** This program may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING.GPL included in the
** packaging of this file.
**
** Please remember that any attempt to workaround the GNU General Public
** License using wrappers, pipes, client/server protocols, and so on
** is considered as license violation. If your program, published on license
** other than GNU General Public License version 2, calls some part of this
** code directly or indirectly, you have to buy commercial license.
** If you do not like our point of view, simply do not use the product.
**
** Licensees holding valid commercial license for this product
** may use this file in accordance with the license published by
** SILVERCODERS and appearing in the file COPYING.COM
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
*****************************************************************************/

#include <fstream>
#include "misc.h"
#include "doctotext_unzip.h"
#include <iostream>
#include "metadata.h"
#include "plain_text_extractor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>        // for strcat()
#include <limits.h>
#ifdef DEBUG
#include "tracing.h"
#endif
#include "version.h"
#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/types.h>
#include<string>
#include<cstring>

using namespace std;

static void version()
{
    printf("DocToText v%s\nConverts DOC, XLS, PPT, RTF, ODF (ODT, ODS, ODP), OOXML (DOCX, XLSX, PPTX) and HTML documents to plain text\nCopyright (c) 2006-20112 SILVERCODERS(R)\nhttp://silvercoders.com\n", VERSION);
}

static void help()
{
    version();
    printf("\nUsage: doctotext [OPTION]... [FILE]\n\n");
    printf("FILE\t\tname of file to convert\n\n");
    printf("Options:\n");
    printf("--meta\textract metadata instead of text.\n");
    printf("--rtf\ttry to parse as RTF document first.\n");
    printf("--odf\ttry to parse as ODF/OOXML document first.\n");
    printf("--ooxml\ttry to parse as ODF/OOXML document first.\n");
    printf("--xls\ttry to parse as XLS document first.\n");
    printf("--xlsb\ttry to parse as XLSB document first.\n");
    printf("--iwork\ttry to parse as iWork document first.\n");
    printf("--ppt\ttry to parse as PPT document first.\n");
    printf("--doc\ttry to parse as DOC document first.\n");
    printf("--html\ttry to parse as HTML document first.\n");
    printf("--pdf\ttry to parse as PDF document first.\n");
    printf("--eml\ttry to parse as EML document first.\n");
    printf("--odfxml\ttry to parse as ODFXML (Open Document Flat XML) document first.\n");
    printf("--fix-xml\ttry to fix corrupted xml files (ODF, OOXML)\n");
    printf("--strip-xml\tstrip xml tags instead of parsing them (ODF, OOXML)\n");
    printf("--unzip-cmd=[COMMAND]\tuse COMMAND to unzip files from archives (ODF, OOXML)\n"
           "\tinstead of build-in decompression library.\n"
           "\t%%d in the command is substituted with destination directory path\n"
           "\t%%a in the command is substituted with name of archive file\n"
           "\t%%f in the command is substituted with name of file to extract\n"
           "\tExample: --unzip-cmd=\"unzip -d %%d %%a %%f\"\n"
          );
    printf("--verbose\tturn on verbose logging\n");
    printf("--log-file=[PATH]\twrite logs to specified file.\n");
}
//检查目录是否存在
//1:存在 0:不存在
int IsFolderExist(const char *path)
{
    DIR *dp;
    if ((dp = opendir(path)) == NULL) {
        return 0;
    }

    closedir(dp);
    return 1;
}
std::string getDefaultExtension(const std::string &fileName)
{
    return fileName.substr(fileName.find_last_of(".") + 1);
}
//检查文件(所有类型)是否存在
//-1:存在 0:不存在
int IsFileExist(const char *path)
{
    return !access(path, F_OK);
}
string get_document(string input)
{
    bool extract_metadata = false;
    string text = "";
    FormattingStyle options;
    options.table_style = TABLE_STYLE_TABLE_LOOK;
    options.list_style.setPrefix(" * ");
    options.url_style = URL_STYLE_UNDERSCORED;
    XmlParseMode mode = PARSE_XML;
    clock_t start, end;
    start = clock();           /*记录起始时间*/
//    printf("filename:%s\n",input.c_str());
    size_t last = input.find_last_of("/");
    std::string name = input.substr(last + 1);
    std::string dir = input.substr(0, last);
    std::string ext = getDefaultExtension(input);
    PlainTextExtractor::ParserType parser_type = PlainTextExtractor::PARSER_AUTO;
    try {
//        if (ext == "docx"|| ext == "dot" )
//            parser_type = PlainTextExtractor::PARSER_DOC;
//        else if (ext == "html" || ext == "htm" || ext == "xhtml" || ext == "xht")
//            parser_type = PlainTextExtractor::PARSER_HTML;
//        else if (ext == "xml")
//            parser_type = PlainTextExtractor::PARSER_ODFXML;
//        else if (ext == "txt" || ext == "md" || ext == "markdown")
//            extract_metadata = true;
//        else if (ext == "doc")
//            parser_type = PlainTextExtractor::PARSER_DOC;
//        else if (ext == "rtf")
//            parser_type = PlainTextExtractor::PARSER_RTF;
//        else if (ext == "xls" || ext == "xlsx")
//            parser_type = PlainTextExtractor::PARSER_XLS;
//        else if (ext == "ppt" || ext == "pptx")
//            parser_type = PlainTextExtractor::PARSER_PPT;
//        else if (ext == "pdf")
//            parser_type = PlainTextExtractor::PARSER_PDF;

        if (ext == "rtf")
            parser_type = PlainTextExtractor::PARSER_RTF;
        else if (ext == "odt" || ext == "ods" || ext == "odp" || ext == "odg" || ext == "docx" || ext == "xlsx" || ext == "pptx" || ext == "ppsx")
            parser_type = PlainTextExtractor::PARSER_ODF_OOXML;
        else if (ext == "xls")
            parser_type = PlainTextExtractor::PARSER_XLS;
        else if (ext == "xlsb")
            parser_type = PlainTextExtractor::PARSER_XLSB;
        else if (ext == "doc" || ext == "dot" || ext == "wps" )
            parser_type = PlainTextExtractor::PARSER_DOC;
        else if (ext == "ppt" || ext == "pps")
            parser_type = PlainTextExtractor::PARSER_PPT;
        else if (ext == "htm" || ext == "html")
            parser_type = PlainTextExtractor::PARSER_HTML;
        else if (ext == "pages" || ext == "key" || ext == "numbers")
            parser_type = PlainTextExtractor::PARSER_IWORK;
        else if (ext == "pdf")
            parser_type = PlainTextExtractor::PARSER_PDF;
        else if (ext == "txt" || ext == "text")
            parser_type = PlainTextExtractor::PARSER_TXT;
        else {
            std::cout << "Unsupported file extension: " << ext << std::endl;
            return "";
        }
        std::cout << "Conversion complete: " << input << std::endl;
    } catch (...) {
        std::cerr << "Error: " << input << std::endl;
    }
    //创建文件解析器
    PlainTextExtractor extractor(parser_type);

    //设置 xml 格式解析模式
    if (mode != PARSE_XML) {
        extractor.setXmlParseMode(mode);
    }

    //
    extractor.setFormattingStyle(options);

    if (extract_metadata) {
        //解析文件属性
        Metadata meta;
        if (!extractor.extractMetadata(input, meta)) {
            cerr << "Error processing file " << input << ".\n";
            return "";
        }
        cout << "Author: " << meta.author() << (meta.authorType() == Metadata::ESTIMATED ? " (estimated)" : "")
             << "\nCreation time: " << date_to_string(meta.creationDate()) << (meta.creationDateType() == Metadata::ESTIMATED ? " (estimated)" : "")
             << "\nLast modified by: " << meta.lastModifiedBy() << (meta.lastModifiedByType() == Metadata::ESTIMATED ? " (estimated)" : "")
             << "\nLast modification time: " << date_to_string(meta.lastModificationDate()) << (meta.lastModificationDateType() == Metadata::ESTIMATED ? " (estimated)" : "")
             << "\nPage count: " << meta.pageCount() << (meta.pageCountType() == Metadata::ESTIMATED ? " (estimated)" : "")
             << "\nWord count: " << meta.wordCount() << (meta.wordCountType() == Metadata::ESTIMATED ? " (estimated)" : "")
             << "\n";
    } else {
        //解析文件内容,
        /*
         * 原因是解析出来的内容和 tests目录下面 *.out 中的内容不相符，
         * 例如: 解析 tests/1.doc.out 最终结果的内容就是现在下面 printf 所打印输出的内容格式，
         * 从可以 tests/Makefile 中看到，在测试的过程中是通过 diff 命令比较，也就是通过比较 1.doc
         * 的解析结果内容和 1.doc.out 内容的是否相同来判断程序是否正常运行的
         */
        if (!extractor.processFile(input, text)) {
            cerr << "Error processing file " << input << ".\n";
            return "";
        }

        //打印文件内容
//        printf("%s\n", text.c_str());

        /*
         *  解析 link 信息
         *
         *  例如：文件 example.html 内容如下
         *  "text before link <a href="target">link</a> text after link"
         *  内容中包含一个 link， 我们使用 URL_STYLE_TEXT_ONLY 类型解析后的结果是
         *  "text before link link text after link".
         *  我们还获取到以下一些值：
         *  调用 getLinkUrl() 返回: "target".
         *  调用 getLinkText() 返回:"link".
         *  调用  getLinkTextSize() 返回: 4 (因为 "link" 是 4 个字节).
         *  调用 getLinkTextPosition() 返回: 17 (因为 "text before link " 是 17 个字节).
         */
        std::vector<Link> links;
        extractor.getParsedLinks(links);
        if (links.size() > 0) {
            printf("parsed links:\n");
            for (size_t i = 0; i < links.size(); ++i) {
                printf("%s @ index = %d length = %d\n", links[i].getLinkUrl(), links[i].getLinkTextPosition(), strlen(links[i].getLinkText()));
            }
        }

        //解析附件，这个现在只有 eml 类型的实现
        std::vector<Attachment> attachments;
        extractor.getAttachments(attachments);
        if (attachments.size() > 0) {
            printf("parsed attachments:\n");
            for (size_t i = 0; i < attachments.size(); ++i) {
                printf("\nname: %s\n", attachments[i].filename());
                std::map<std::string, Variant> variables = attachments[i].getFields();
                for (std::map<std::string, Variant>::iterator it = variables.begin(); it != variables.end(); ++it) {
                    if (it->first != "Content-ID") {
                        printf("field: %s, value: %s\n", it->first.c_str(), it->second.getString());
                    }
                }
            }
        }
    }
    end = clock();           /*记录结束时间*/
    {
        double seconds  = (double)(end - start) / CLOCKS_PER_SEC;
        fprintf(stderr, "Use time is: %.8f s\n", seconds);
    }
    return text.c_str();
}


int getdir(const char *pathname)
{
    DIR *path = NULL;
    path = opendir(pathname);

    if (path == NULL) {
        perror("failed");
        exit(1);
    }
    struct dirent *ptr; //目录结构体---属性：目录类型 d_type,  目录名称d_name
    char buf[1024] = {0};
    while ((ptr = readdir(path)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        //如果是目录
        if (ptr->d_type == DT_DIR) {

            sprintf(buf, "%s/%s", pathname, ptr->d_name);
//            printf("目录:%s\n",buf);
            getdir(buf);
        }
        if (ptr->d_type == DT_REG) {
            sprintf(buf, "%s/%s", pathname, ptr->d_name); //把pathname和文件名拼接后放进缓冲字符数组
            printf("文件:%s\n", buf);
            string contents = get_document(string(buf));
            printf("========================================================\n%s\n", contents.c_str());
        }
    }
    return 0;
}
//
//int main(int argc, char *argv[])
//{
//    //开启这个选项需要在 Makefile 中加入 -DDEBUG 选项
//#ifdef DEBUG
//    doctotext_init_tracing("doctotext.trace");
//#endif

//    if (argc < 2) {
//        help();
//        return EXIT_FAILURE;
//    }
//    string filename = argv[1];
//    string contents = "";
//    if (IsFolderExist(filename.c_str())) {
//        getdir(filename.c_str());
//    } else {
//        contents = get_document(filename);
//        printf("========================================================\n%s\n", contents.c_str());
//    }

//    return EXIT_SUCCESS;
//}


//int main(int argc, char* argv[])
//{
//  //开启这个选项需要在 Makefile 中加入 -DDEBUG 选项
//  #ifdef DEBUG
//      doctotext_init_tracing("doctotext.trace");
//  #endif
//
//  if (argc < 2)
//  {
//      help();
//      return EXIT_FAILURE;
//  }
//
//  std::string arg;
//  std::string cmd;
//
//  bool extract_metadata = false;
//
//  //设置文件的类型，PARSER_AUTO 意思是格式是未知的
//  PlainTextExtractor::ParserType parser_type = PlainTextExtractor::PARSER_AUTO;
//
//  XmlParseMode mode = PARSE_XML;
//
//  FormattingStyle options;
//  options.table_style = TABLE_STYLE_TABLE_LOOK;
//  options.list_style.setPrefix(" * ");
//  options.url_style = URL_STYLE_UNDERSCORED;
//
//  //详细日志的开关
//  bool verbose = false;
//  //日志流
//  ofstream* log_stream = NULL;
//
//  for(int i = 1 ; i < argc ; i ++)
//  {
//      arg = argv[i-1];
//
//      if (arg.find("--meta", 0) != -1)
//      {
//          //文件属性信息
//          extract_metadata = true;
//      }
//
//      //判断是否指定了要解析文件的格式
//      if (arg.find("--rtf", 0) != -1)
//      {
//          parser_type = PlainTextExtractor::PARSER_RTF;
//      }
//
//      if (arg.find("--odfxml", 0) != -1)
//      {
//          //odf 格式文件
//          parser_type = PlainTextExtractor::PARSER_ODFXML;
//      }
//
//      if (arg.find("--odf", 0) != -1 || arg.find("ooxml", 0) != -1)
//      {
//          //office2007 格式文件
//          parser_type = PlainTextExtractor::PARSER_ODF_OOXML;
//      }
//
//      if (arg.find("--ppt", 0) != -1)
//      {
//          //office2003 powerpoint 格式文件
//          parser_type = PlainTextExtractor::PARSER_PPT;
//      }
//
//      if (arg.find("--doc", 0) != -1)
//      {
//          //office2003 word 格式文件
//          parser_type = PlainTextExtractor::PARSER_DOC;
//      }
//
//      if (arg.find("--xls", 0) != -1)
//      {
//          //office2003 excel 格式文件
//          parser_type = PlainTextExtractor::PARSER_XLS;
//      }
//
//      if (arg.find("--xlsb", 0) != -1)
//      {
//          //excel 二进制类型的工作簿
//          parser_type = PlainTextExtractor::PARSER_XLSB;
//      }
//
//      if (arg.find("--iwork", 0) != -1)
//      {
//          //苹果办公格式文件
//          parser_type = PlainTextExtractor::PARSER_IWORK;
//      }
//
//      if (arg.find("--html", 0) != -1)
//      {
//          //html 格式文件
//          parser_type = PlainTextExtractor::PARSER_HTML;
//      }
//
//      if (arg.find("--pdf", 0) != -1)
//      {
//          //pdf 格式文件
//          parser_type = PlainTextExtractor::PARSER_PDF;
//      }
//
//      if (arg.find("--eml", 0) != -1)
//      {
//          //邮件格式文件
//          parser_type = PlainTextExtractor::PARSER_EML;
//      }
//
//      if(arg.find("table-style=", 0) != -1)
//      {
//          if(arg.find("one-row", arg.find("table-style=", 0) + 11) != -1)
//          {
//              options.table_style = TABLE_STYLE_ONE_ROW;
//          }
//          if(arg.find("one-col", arg.find("table-style=", 0) + 11) != -1)
//          {
//              options.table_style = TABLE_STYLE_ONE_COL;
//          }
//          if(arg.find("table-look", arg.find("table-style=", 0) + 11) != -1)
//          {
//              options.table_style = TABLE_STYLE_TABLE_LOOK;
//          }
//      }
//
//      if(arg.find("url-style=", 0) != -1)
//      {
//          if(arg.find("text-only", arg.find("url-style=", 0) + 10) != -1)
//          {
//              options.url_style = URL_STYLE_TEXT_ONLY;
//          }
//          if(arg.find("extended", arg.find("url-style=", 0) + 10) != -1)
//          {
//              options.url_style = URL_STYLE_EXTENDED;
//          }
//          if(arg.find("underscored", arg.find("url-style=", 0) + 10) != -1)
//          {
//              options.url_style = URL_STYLE_UNDERSCORED;
//          }
//      }
//
//      if(arg.find("list-style-prefix=", 0) != -1)
//      {
//          options.list_style.setPrefix(arg.substr(arg.find("list-style-prefix=", 0) + 18));
//      }
//
//      if (arg.find("fix-xml", 0) != std::string::npos)
//      {
//          //是否尝试修复损坏的 xml 文件
//          mode = FIX_XML;
//      }
//
//      if (arg.find("strip-xml", 0) != std::string::npos)
//      {
//          mode = STRIP_XML;
//      }
//
//      //获取解压缩的命令
//      if (arg.find("unzip-cmd=", 0) != -1)
//      {
//          DocToTextUnzip::setUnzipCommand(arg.substr(arg.find("unzip-cmd=", 0) + 10));
//      }
//
//      //获取详细日志开关
//      if (arg.find("verbose", 0) != std::string::npos)
//      {
//          verbose = true;
//      }
//
//      //获取日志输出文件名称
//      if (arg.find("log-file=", 0) != std::string::npos)
//      {
//          log_stream = new ofstream(arg.substr(arg.find("log-file=", 0) + 9).c_str());
//      }
//  }
//
//  //创建文件解析器
//  PlainTextExtractor extractor(parser_type);
//
//  //设置详细日志开关
//  if (verbose)
//  {
//      extractor.setVerboseLogging(true);
//  }
//
//  //设置日志输出文件名称F
//  if (log_stream != NULL)
//  {
//      extractor.setLogStream(*log_stream);
//  }
//
//  //设置 xml 格式解析模式
//  if (mode != PARSE_XML)
//  {
//      extractor.setXmlParseMode(mode);
//  }
//
//  //
//  extractor.setFormattingStyle(options);
//
//  if (extract_metadata)
//  {
//      //解析文件属性
//      Metadata meta;
//      if (!extractor.extractMetadata(argv[argc - 1], meta))
//      {
//          (log_stream != NULL ? *log_stream : cerr) << "Error processing file " << argv[argc - 1] << ".\n";
//          return EXIT_FAILURE;
//      }
//      cout << "Author: " << meta.author() << (meta.authorType() == Metadata::ESTIMATED ? " (estimated)" : "")
//          << "\nCreation time: " << date_to_string(meta.creationDate()) << (meta.creationDateType() == Metadata::ESTIMATED ? " (estimated)" : "")
//          << "\nLast modified by: " << meta.lastModifiedBy() << (meta.lastModifiedByType() == Metadata::ESTIMATED ? " (estimated)" : "")
//          << "\nLast modification time: " << date_to_string(meta.lastModificationDate()) << (meta.lastModificationDateType() == Metadata::ESTIMATED ? " (estimated)" : "")
//          << "\nPage count: " << meta.pageCount() << (meta.pageCountType() == Metadata::ESTIMATED ? " (estimated)" : "")
//          << "\nWord count: " << meta.wordCount() << (meta.wordCountType() == Metadata::ESTIMATED ? " (estimated)" : "")
//          << "\n";
//  }
//  else
//  {
//      //解析文件内容,
//        #warning NOTCE: 注意下面的所有 printf 语句不要做任何修改，修改后会造成运行自动化测试报错: make test
//        /*
//         * 原因是解析出来的内容和 tests目录下面 *.out 中的内容不相符，
//         * 例如: 解析 tests/1.doc.out 最终结果的内容就是现在下面 printf 所打印输出的内容格式，
//         * 从可以 tests/Makefile 中看到，在测试的过程中是通过 diff 命令比较，也就是通过比较 1.doc
//         * 的解析结果内容和 1.doc.out 内容的是否相同来判断程序是否正常运行的
//         */
//
//      string text;
//      if (!extractor.processFile(argv[argc - 1], text))
//      {
//          (log_stream != NULL ? *log_stream : cerr) << "Error processing file " << argv[argc - 1] << ".\n";
//          return EXIT_FAILURE;
//      }
//
//      //打印文件内容
//      printf("%s\n", text.c_str());
//
//      /*
//       *  解析 link 信息
//       *
//       *  例如：文件 example.html 内容如下
//       *  "text before link <a href="target">link</a> text after link"
//       *  内容中包含一个 link， 我们使用 URL_STYLE_TEXT_ONLY 类型解析后的结果是
//       *  "text before link link text after link".
//       *  我们还获取到以下一些值：
//       *  调用 getLinkUrl() 返回: "target".
//       *  调用 getLinkText() 返回:"link".
//       *  调用  getLinkTextSize() 返回: 4 (因为 "link" 是 4 个字节).
//       *  调用 getLinkTextPosition() 返回: 17 (因为 "text before link " 是 17 个字节).
//       */
//      std::vector<Link> links;
//      extractor.getParsedLinks(links);
//      if (links.size() > 0)
//      {
//          printf("parsed links:\n");
//          for (size_t i = 0; i < links.size(); ++i)
//          {
//              printf("%s @ index = %d length = %d\n", links[i].getLinkUrl(), links[i].getLinkTextPosition(), strlen(links[i].getLinkText()));
//          }
//      }
//
//      //解析附件，这个现在只有 eml 类型的实现
//      std::vector<Attachment> attachments;
//      extractor.getAttachments(attachments);
//      if (attachments.size() > 0)
//      {
//          printf("parsed attachments:\n");
//          for (size_t i = 0; i < attachments.size(); ++i)
//          {
//              printf("\nname: %s\n", attachments[i].filename());
//              std::map<std::string, Variant> variables = attachments[i].getFields();
//              for (std::map<std::string, Variant>::iterator it = variables.begin(); it != variables.end(); ++it)
//              {
//                   //源码中很多的 warning TODO 是没有完成的功能需要后续进行完善
//                     #warning TODO:  If Content-ID is not present in the file, mimetic generates it... \
//                   and test/Makefile always goes wrong.Maybe we should skip this field?
//                  if (it->first != "Content-ID")
//                  {
//                      printf("field: %s, value: %s\n", it->first.c_str(), it->second.getString());
//                  }
//              }
//          }
//      }
//  }
//  if (log_stream != NULL)
//  {
//      delete log_stream;
//  }
//  return EXIT_SUCCESS;
//}
