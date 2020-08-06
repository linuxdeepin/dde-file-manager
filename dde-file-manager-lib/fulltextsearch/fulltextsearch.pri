# Copyright (C) 2019 ~ 2020 Uniontech Technology Co., Ltd.

#Author:     hujianzhong <hujianhzong@uniontech.com>

#Maintainer: hujianzhong <hujianhzong@uniontech.com>

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.

#This program is the full text search at dde-file-manager.
#-------------------------------------------------
#
# Project created by QtCreator 2020-05-12T16:00:00
#这个搜索引擎是利用C++语言，文件搜索速度相当快，是移植的开源代码doctotext的核心部分
#-------------------------------------------------
HEADERS += \ 
    $$PWD/fulltextsearch.h\
    $$PWD/doctotext_inc/attachment.h \
    $$PWD/doctotext_inc/doctotext_unzip.h \
    $$PWD/doctotext_inc/exception.h \
    $$PWD/doctotext_inc/formatting_style.h \
    $$PWD/doctotext_inc/link.h \
    $$PWD/doctotext_inc/metadata.h \
    $$PWD/doctotext_inc/misc.h \
    $$PWD/doctotext_inc/plain_text_extractor.h \
    $$PWD/doctotext_inc/tracing.h \
    $$PWD/doctotext_inc/variant.h \
    $$PWD/doctotext_inc/wv2_inc/wv2/dllmagic.h\
    $$PWD/doctotext_inc/wv2_inc/wv2/ustring.h\
    $$PWD/lucenePlusPlus/Lucene.h\
    $$PWD/lucenePlusPlus/ChineseAnalyzer.h\
    $$PWD/lucenePlusPlus/ChineseFilter.h\
    $$PWD/lucenePlusPlus/ChineseTokenizer.h\
    $$PWD/lucenePlusPlus/StandardAnalyzer.h\
    $$PWD/lucenePlusPlus/StandardFilter.h\
    $$PWD/lucenePlusPlus/StandardTokenizer.h\
    $$PWD/lucenePlusPlus/KeywordAnalyzer.h\
    $$PWD/lucenePlusPlus/KeywordTokenizer.h\
    $$PWD/lucenePlusPlus/LowerCaseFilter.h\
    $$PWD/lucenePlusPlus/LowerCaseTokenizer.h\
    $$PWD/lucenePlusPlus/PerFieldAnalyzerWrapper.h\
    $$PWD/lucenePlusPlus/PorterStemFilter.h\
    $$PWD/lucenePlusPlus/SimpleAnalyzer.h\
    $$PWD/lucenePlusPlus/StopAnalyzer.h\
    $$PWD/lucenePlusPlus/StopFilter.h\
    $$PWD/lucenePlusPlus/Token.h\
    $$PWD/lucenePlusPlus/TokenFilter.h\
    $$PWD/lucenePlusPlus/WhitespaceAnalyzer.h\
    $$PWD/lucenePlusPlus/WhitespaceTokenizer.h\
    $$PWD/lucenePlusPlus/DateField.h\
    $$PWD/lucenePlusPlus/DateTools.h\
    $$PWD/lucenePlusPlus/Document.h\
    $$PWD/lucenePlusPlus/Field.h\
    $$PWD/lucenePlusPlus/NumberTools.h\
    $$PWD/lucenePlusPlus/NumericField.h\
    $$PWD/lucenePlusPlus/IndexCommit.h\
    $$PWD/lucenePlusPlus/IndexDeletionPolicy.h\
    $$PWD/lucenePlusPlus/IndexReader.h\
    $$PWD/lucenePlusPlus/IndexWriter.h\
    $$PWD/lucenePlusPlus/KeepOnlyLastCommitDeletionPolicy.h\
    $$PWD/lucenePlusPlus/LogByteSizeMergePolicy.h\
    $$PWD/lucenePlusPlus/LogDocMergePolicy.h\
    $$PWD/lucenePlusPlus/LogMergePolicy.h\
    $$PWD/lucenePlusPlus/MergeScheduler.h\
    $$PWD/lucenePlusPlus/MultiReader.h\
    $$PWD/lucenePlusPlus/ParallelReader.h\
    $$PWD/lucenePlusPlus/Term.h\
    $$PWD/lucenePlusPlus/TermDocs.h\
    $$PWD/lucenePlusPlus/TermEnum.h\
    $$PWD/lucenePlusPlus/MultiFieldQueryParser.h\
    $$PWD/lucenePlusPlus/QueryParseError.h\
    $$PWD/lucenePlusPlus/QueryParser.h\
    $$PWD/lucenePlusPlus/BooleanClause.h\
    $$PWD/lucenePlusPlus/BooleanQuery.h\
    $$PWD/lucenePlusPlus/DocIdSet.h\
    $$PWD/lucenePlusPlus/DocIdSetIterator.h\
    $$PWD/lucenePlusPlus/Explanation.h\
    $$PWD/lucenePlusPlus/IndexSearcher.h\
    $$PWD/lucenePlusPlus/MatchAllDocsQuery.h\
    $$PWD/lucenePlusPlus/MultiPhraseQuery.h\
    $$PWD/lucenePlusPlus/MultiSearcher.h\
    $$PWD/lucenePlusPlus/MultiTermQuery.h\
    $$PWD/lucenePlusPlus/NumericRangeFilter.h\
    $$PWD/lucenePlusPlus/NumericRangeQuery.h\
    $$PWD/lucenePlusPlus/ParallelMultiSearcher.h\
    $$PWD/lucenePlusPlus/PhraseQuery.h\
    $$PWD/lucenePlusPlus/PrefixFilter.h\
    $$PWD/lucenePlusPlus/PrefixQuery.h\
    $$PWD/lucenePlusPlus/ScoreDoc.h\
    $$PWD/lucenePlusPlus/Scorer.h\
    $$PWD/lucenePlusPlus/Searcher.h\
    $$PWD/lucenePlusPlus/Sort.h\
    $$PWD/lucenePlusPlus/TermQuery.h\
    $$PWD/lucenePlusPlus/TermRangeFilter.h\
    $$PWD/lucenePlusPlus/TermRangeQuery.h\
    $$PWD/lucenePlusPlus/TopDocs.h\
    $$PWD/lucenePlusPlus/TopDocsCollector.h\
    $$PWD/lucenePlusPlus/TopFieldCollector.h\
    $$PWD/lucenePlusPlus/TopScoreDocCollector.h\
    $$PWD/lucenePlusPlus/Weight.h\
    $$PWD/lucenePlusPlus/WildcardQuery.h\
    $$PWD/lucenePlusPlus/SpanFirstQuery.h\
    $$PWD/lucenePlusPlus/SpanNearQuery.h\
    $$PWD/lucenePlusPlus/SpanNotQuery.h\
    $$PWD/lucenePlusPlus/SpanOrQuery.h\
    $$PWD/lucenePlusPlus/SpanQuery.h\
    $$PWD/lucenePlusPlus/FSDirectory.h\
    $$PWD/lucenePlusPlus/MMapDirectory.h\
    $$PWD/lucenePlusPlus/RAMDirectory.h\
    $$PWD/lucenePlusPlus/RAMFile.h\
    $$PWD/lucenePlusPlus/RAMInputStream.h\
    $$PWD/lucenePlusPlus/RAMOutputStream.h\
    $$PWD/lucenePlusPlus/SimpleFSDirectory.h\
    $$PWD/lucenePlusPlus/MiscUtils.h\
    $$PWD/lucenePlusPlus/StringUtils.h\
    $$PWD/lucenePlusPlus/BufferedReader.h\
    $$PWD/lucenePlusPlus/DocIdBitSet.h\
    $$PWD/lucenePlusPlus/FileReader.h\
    $$PWD/lucenePlusPlus/InfoStream.h\
    $$PWD/lucenePlusPlus/LuceneThread.h\
    $$PWD/lucenePlusPlus/OpenBitSet.h\
    $$PWD/lucenePlusPlus/OpenBitSetDISI.h\
    $$PWD/lucenePlusPlus/OpenBitSetIterator.h\
    $$PWD/lucenePlusPlus/StringReader.h\
    $$PWD/lucenePlusPlus/ThreadPool.h

SOURCES += \ 
    $$PWD/fulltextsearch.cpp

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/lucenePlusPlus
INCLUDEPATH += $$PWD/doctotext_inc
INCLUDEPATH += $$PWD/doctotext_inc/wv2_inc

DEPENDPATH += $$PWD/doctotext_inc


unix:!macx: LIBS += -L$$PWD/doctotext/ -ldoctotext
unix:!macx: LIBS += -L$$PWD/doctotext/ -llucene++
unix:!macx: LIBS += -L$$PWD/doctotext/ -llucene++-contrib

LIBS +=-lboost_thread
LIBS +=-lboost_system
DEFINES +=BOOST_USE_LIB
