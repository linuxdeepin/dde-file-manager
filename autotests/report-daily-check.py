#!/usr/bin/python 
#coding=utf-8 

# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
       
from xml.dom.minidom import parse 
import xml.dom.minidom 
import os
from os import path
from bs4 import BeautifulSoup
import re
import time
import sys

print("hello world, will scann the dde-file-manager ut build path and mark the details on daily report folder ")

#real_project_path = os.path.split(os.path.realpath(__file__))[0]
real_project_path = os.path.abspath(os.path.join(os.getcwd(), ".."))

daily_report_path = real_project_path + "/build-ut"
build_ut_report_path = real_project_path + "/build-ut/report"
build_ut_coverage_path = real_project_path + "/build-ut/html"

if(len(sys.argv) > 1):
    daily_report_path = sys.argv[1]
    build_ut_report_path = sys.argv[2] + "/report"
    build_ut_coverage_path = sys.argv[2] + "/html"

print(daily_report_path)
print(build_ut_report_path)
print(build_ut_coverage_path)

# UT 报告扫描
def scann_report_file(report_path):
    reportlist = []
    file = os.listdir(report_path)
    for f in file:
        real_url = path.join(report_path,f)
        reportlist.append(real_url)
        #print(real_url)
    return reportlist

class tests_info:
    def __init__(self):
        self.file = "" #file path
        self.title = "" #file path
        self.totalTests = ""
        self.failures = "" 
        self.errors = ""
        self.buglist=[]

def parse_report_from_file(file):
    # 使用minidom解析器打开XML文档 
    DOMTree = xml.dom.minidom.parse(file) 
    Data = DOMTree.documentElement
    test_info = tests_info()
    test_info.file = file
    test_info.title=os.path.basename(file)
    test_info.totalTests = Data.getAttribute("tests")
    test_info.failures = Data.getAttribute("failures")
    test_info.errors = Data.getAttribute("errors")
     
    testsuite_list = Data.getElementsByTagName("testsuite")
    for tstsuite in testsuite_list:
        testcase_list = tstsuite.getElementsByTagName("testcase")
        for tstcase in testcase_list:
            name_tst = tstcase.getAttribute("name")
            if("bug_" in name_tst):
                test_info.buglist.append(name_tst)
                
    print(test_info.file)
    print("has tests:"+ test_info.totalTests)
    print("has failures:"+ test_info.failures)
    print("has errors:"+ test_info.errors)
    if(len(test_info.buglist) <1 ):
        test_info.buglist.append("no bug list")
    print("has bug list coverage:"+ str(test_info.buglist))

    return test_info

def get_reoprt_info(report_path):
    total_tests=[]
    reportlist = scann_report_file(report_path)
    for report in reportlist:
        total_tests.append(parse_report_from_file(report))
    return total_tests


# 覆盖率扫描
def scan_coverage_file(coverage_path):
    coverage_file =[]
    final_name = []
    for root,dirs,files in os.walk(coverage_path):
        for file in files:
            if "cov_" in file and ".html" in file:
                cov_file = os.path.join(root,file)
                coverage_file.append(cov_file)
                #print(cov_file)
    return coverage_file


class coverage_info:
    def __init__(self):
        self.file = "" #file path
        self.title = "" #覆盖名
        self.covAreaName = "" #line覆盖、fuction覆盖
        self.covCodeNum = ""     # 被覆盖代码行数
        self.totalCodeNum = ""     # 代码总行数
        self.covValue = ""    # 覆盖率

def parse_coverage_from_file(file_path):
    file = open(file_path, "rb")
    html = file.read().decode("utf-8")
    soup = BeautifulSoup(html, 'html.parser')
    table_nodes = soup.find_all('table', cellpadding=1)

    covitem = coverage_info()
    covitem.file = file_path
    for table in table_nodes:
        tr_list = table.find_all('tr')
        for tr in tr_list:
            item =tr.find('td',{'class':'headerValue'})
            if(item != None and "covinfo_" in item.get_text()):
                covitem.title = item.get_text()

            item =tr.find('td',{'class':'headerCovTableEntryLo'})  #覆盖率
            itemMed =tr.find('td',{'class':'headerCovTableEntryMed'}) #覆盖率
            if(item == None and itemMed == None):
                continue
            if(item != None):
                covitem.covValue = item.get_text()

            if(itemMed != None):
                covitem.covValue = itemMed.get_text()

            td_list = tr.find_all("td")
            item =tr.find_all('td',{'class':'headerItem'})
            covitem.covAreaName = item[1].get_text()
            if("Lines:" not in covitem.covAreaName):
                continue

            item =tr.find_all('td',{'class':'headerCovTableEntry'})
            covitem.covCodeNum = item[0].get_text()
            covitem.totalCodeNum = item[1].get_text()
            
            print(covitem.file)
            print(covitem.title)
            print(covitem.covAreaName)
            print(covitem.covValue)
            print(covitem.covCodeNum +":"+covitem.totalCodeNum)
            break
        break

    file.close()
    return covitem

def get_coverage_info(coverage_path):
    total_coverage=[]
    coverage_list = scan_coverage_file(coverage_path)
    for report in coverage_list:
        total_coverage.append(parse_coverage_from_file(report))
    return total_coverage

def produce_report_file(filename):
    total_tests = get_reoprt_info(build_ut_report_path)
    total_coverage = get_coverage_info(build_ut_coverage_path)
    report_file = daily_report_path+"/"+filename
    splitmark=' , '
    print(report_file)
    file = open(report_file, 'w')
    for test in total_tests:
        file.write(str(test.title)+splitmark)
        file.write(str("totalTests:" + test.totalTests)+splitmark)
        file.write(str("failures:" + test.failures)+splitmark)
        file.write(str("errors:" + test.errors)+splitmark)
        file.write("buglist:" + "|".join(test.buglist)+splitmark)
        file.write(str(test.file)+'\n')
    for test in total_coverage:
        file.write(str(test.title)+splitmark)
        file.write(str(test.covAreaName)+splitmark)
        file.write(str(test.covCodeNum +" : "+test.totalCodeNum)+splitmark)
        file.write(str(test.covValue)+splitmark)
        file.write(str(test.file)+'\n')
    file.close()


produce_report_file(time.strftime("%Y-%m-%d-%H:%M:%S")+"_ut_details.csv")