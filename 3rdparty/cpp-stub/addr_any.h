#ifndef __ADDR_ANY_H__
#define __ADDR_ANY_H__


//linux
#include <regex.h>
#include <cxxabi.h>
//c
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

//c++
#include <string>
#include <map>
//project
#include "elfio.hpp"



class AddrAny
{
public:
    AddrAny()
    {
        m_init = get_exe_pathname(m_fullname);
        m_baseaddr = 0;
    }
    AddrAny(std::string libname)
    {
        m_init = get_lib_pathname_and_baseaddr(libname, m_fullname, m_baseaddr);
    }

    int get_local_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)
    {
        return get_func_addr(SHT_SYMTAB, STB_LOCAL, func_name_regex_str, result);
    }
    int get_global_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)
    {
        return get_func_addr(SHT_SYMTAB, STB_GLOBAL, func_name_regex_str, result);
    }
    int get_weak_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)
    {
        return get_func_addr(SHT_SYMTAB, STB_WEAK, func_name_regex_str, result);
    }
    
    int get_global_func_addr_dynsym( std::string func_name_regex_str, std::map<std::string,void*>& result)
    {
        return get_func_addr(SHT_DYNSYM, STB_GLOBAL, func_name_regex_str, result);
    }
    int get_weak_func_addr_dynsym(std::string func_name_regex_str, std::map<std::string,void*>& result)
    {
        return get_func_addr(SHT_DYNSYM, STB_WEAK, func_name_regex_str, result);
    }
    
private:
    bool demangle(std::string& s, std::string& name) {
        int status;
        char* pname = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
        if (status != 0)
        {
            switch(status)
            {
                case -1: name = "memory allocation error"; break;
                case -2: name = "invalid name given"; break;
                case -3: name = "internal error: __cxa_demangle: invalid argument"; break;
                default: name = "unknown error occured"; break;
            }
            return false;
        }
        name = pname;
        free(pname);
        return true;
    }
    bool get_exe_pathname( std::string& name)
    {
        char                     line[512];
        FILE                    *fp;
        uintptr_t                base_addr;
        char                     perm[5];
        unsigned long            offset;
        int                      pathname_pos;
        char                    *pathname;
        size_t                   pathname_len;
        int                      match = 0;
        
        if(NULL == (fp = fopen("/proc/self/maps", "r")))
        {
            return false;
        }

        while(fgets(line, sizeof(line), fp))
        {
            if(sscanf(line, "%" PRIxPTR "-%*lx %4s %lx %*x:%*x %*d%n", &base_addr, perm, &offset, &pathname_pos) != 3) continue;

            if(0 != offset) continue;

            //get pathname
            while(isspace(line[pathname_pos]) && pathname_pos < (int)(sizeof(line) - 1))
                pathname_pos += 1;
            if(pathname_pos >= (int)(sizeof(line) - 1)) continue;
            pathname = line + pathname_pos;
            pathname_len = strlen(pathname);
            if(0 == pathname_len) continue;
            if(pathname[pathname_len - 1] == '\n')
            {
                pathname[pathname_len - 1] = '\0';
                pathname_len -= 1;
            }
            if(0 == pathname_len) continue;
            if('[' == pathname[0]) continue;

            name = pathname;
            match = 1;
            break;

        }
        fclose(fp);

        if(0 == match)
        {
            return false;
        }
        else
        {
            return true;
        }

    }

    bool get_lib_pathname_and_baseaddr(std::string pathname_regex_str, std::string& name, unsigned long& addr)
    {
        char                     line[512];
        FILE                    *fp;
        uintptr_t                base_addr;
        char                     perm[5];
        unsigned long            offset;
        int                      pathname_pos;
        char                    *pathname;
        size_t                   pathname_len;
        int                      match;
        regex_t   pathname_regex;

        regcomp(&pathname_regex, pathname_regex_str.c_str(), 0);

        if(NULL == (fp = fopen("/proc/self/maps", "r")))
        {
            return false;
        }

        while(fgets(line, sizeof(line), fp))
        {
            if(sscanf(line, "%" PRIxPTR "-%*lx %4s %lx %*x:%*x %*d%n", &base_addr, perm, &offset, &pathname_pos) != 3) continue;

            //check permission
            if(perm[0] != 'r') continue;
            if(perm[3] != 'p') continue; //do not touch the shared memory

            //check offset
            //
            //We are trying to find ELF header in memory.
            //It can only be found at the beginning of a mapped memory regions
            //whose offset is 0.
            if(0 != offset) continue;

            //get pathname
            while(isspace(line[pathname_pos]) && pathname_pos < (int)(sizeof(line) - 1))
                pathname_pos += 1;
            if(pathname_pos >= (int)(sizeof(line) - 1)) continue;
            pathname = line + pathname_pos;
            pathname_len = strlen(pathname);
            if(0 == pathname_len) continue;
            if(pathname[pathname_len - 1] == '\n')
            {
                pathname[pathname_len - 1] = '\0';
                pathname_len -= 1;
            }
            if(0 == pathname_len) continue;
            if('[' == pathname[0]) continue;

            //check pathname
            //if we need to hook this elf?
            match = 0;
            if(0 == regexec(&pathname_regex, pathname, 0, NULL, 0))
            {
                match = 1;
                name = pathname;
                addr = (unsigned long)base_addr;
                break;
            }
            if(0 == match) continue;

        }
        fclose(fp);
        if(0 == match)
        {
            return false;
        }
        else
        {
            return true;
        }

    }

    int get_func_addr(unsigned int ttype, unsigned int stype, std::string& func_name_regex_str, std::map<std::string,void*>& result)
    {
        // Create an elfio reader
        ELFIO::elfio reader;
        int count = 0;
        regex_t   pathname_regex;

        if(!m_init)
        {
            return -1;
        }

        regcomp(&pathname_regex, func_name_regex_str.c_str(), 0);
        // Load ELF data
        if(!reader.load(m_fullname.c_str()))
        {
            return -1;
        }
        
        ELFIO::Elf_Half sec_num = reader.sections.size();
        for(int i = 0; i < sec_num; ++i)
        {
            ELFIO::section* psec = reader.sections[i];
            // Check section type
            if(psec->get_type() == ttype)
            {
                const ELFIO::symbol_section_accessor symbols( reader, psec );
                for ( unsigned int j = 0; j < symbols.get_symbols_num(); ++j )
                {
                    std::string name;
                    std::string name_mangle;
                    ELFIO::Elf64_Addr value;
                    ELFIO::Elf_Xword size;
                    unsigned char bind;
                    unsigned char type;
                    ELFIO::Elf_Half section_index;
                    unsigned char other;
                    
                    // Read symbol properties
                    symbols.get_symbol( j, name, value, size, bind, type, section_index, other );
                    if(type == STT_FUNC && bind == stype)
                    {
                        bool ret = demangle(name,name_mangle);
                        if(ret == true)
                        {
                            if (0 == regexec(&pathname_regex, name_mangle.c_str(), 0, NULL, 0))
                            {
                                  result.insert ( std::pair<std::string,void *>(name_mangle,(void*)(value + m_baseaddr)));
                                  count++;
                            }
                        }
                        else
                        {
                            if (0 == regexec(&pathname_regex, name.c_str(), 0, NULL, 0))
                            {
                                  result.insert ( std::pair<std::string,void *>(name,(void*)(value + m_baseaddr)));
                                  count++;
                            }
                        }
                    }
                }
                break;
            }
        }
        
        return count;
    }
private:
    bool m_init;
    std::string m_name;
    std::string m_fullname;
    unsigned long m_baseaddr;

};
#endif
