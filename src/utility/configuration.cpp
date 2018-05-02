/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Microsoft Corporation
 * 
 * -=- Robust Distributed System Nucleus (rDSN) -=- 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * Description:
 *     What is this file about?
 *
 * Revision history:
 *     xxxx-xx-xx, author, first version
 *     xxxx-xx-xx, author, fix bug about xxx
 */

# include <dsn/utility/configuration.h>
# include <dsn/utility/misc.h>
# include <cassert>
# include <errno.h>
# include <iostream>
# include <algorithm>

namespace dsn {


configuration::configuration()
{
    _warning = false;
}

configuration::~configuration()
{
}

bool configuration::load_include(const char* inc, const char* arguments)
{
    configuration conf;
    if (!conf.load(inc, arguments))
    {
        fprintf(stderr, "load included configuration file %s failed\n", inc);
        return false;
    }

    fprintf(stderr, "load included configuration file %s ...\n", inc);
    for (auto& sec : conf._configs)
    {
        for (auto& kv : sec.second)
        {
            set(kv.second->section.c_str(),
                kv.second->key.c_str(),
                kv.second->value.c_str(),
                kv.second->dsptr.c_str()
            );
        }
    }
    return true;
}

bool configuration::load(const char* file_name, const char* arguments, const char* overwrites)
{
    _file_name = std::string(file_name);

    FILE* fd = ::fopen(file_name, "rb");
    if (fd == nullptr)
    {
        std::string cdir;
        dsn::utils::filesystem::get_current_directory(cdir);
        fprintf(stderr, "ERROR: cannot open file %s in %s, err = %s\n", file_name, cdir.c_str(), strerror(errno));
        return false;
    }
    ::fseek(fd, 0, SEEK_END);
    int len = ftell(fd);
    if (len == -1 || len == 0)
    {
        fprintf(stderr, "ERROR: cannot get length of %s, err = %s\n", file_name, strerror(errno));
        ::fclose(fd);
        return false;
    }

    _file_data.resize(len + 1);
    ::fseek(fd, 0, SEEK_SET);
    auto sz = ::fread((char*)_file_data.c_str(), len, 1, fd);
    ::fclose(fd);
    if (sz != 1)
    {
        fprintf(stderr, "ERROR: cannot read correct data of %s, err = %s\n", file_name, strerror(errno));
        return false;
    }
    _file_data[len] = '\n';

    //-----------------------------------------------------------
    // STEP: replace data with specified arguments
    //
    if (arguments != nullptr)
    {
        std::string str_arguments(arguments);
        std::list<std::string> argkvs;
        utils::split_args(str_arguments.c_str(), argkvs, ';');
        for (auto& kv : argkvs)
        {
            std::list<std::string> vs;
            utils::split_args(kv.c_str(), vs, '=');
            if (vs.size() != 2)
            {
                fprintf(stderr, "ERROR: invalid configuration argument: '%s' in '%s'\n", kv.c_str(), arguments);
                return false;
            }

            std::string key = std::string("%") + *vs.begin() + std::string("%");
            std::string value = *vs.rbegin();

            _file_data = utils::replace_string(_file_data, key, value);

            fprintf(stderr, "config.replace: %s => %s\n", key.c_str(), value.c_str());
        }
    }
    
    //-----------------------------------------------------------
    // STEP: parse mapped file and build conf map (include @include)
    //
    std::map<std::string, conf*>* pSection = nullptr;
    char *p, *pLine = (char*)"", *pNextLine, *pEnd, *pSectionName = nullptr, *pEqual;
    int lineno = 0;

    // ATTENTION: arguments replace_string() may cause _file_data changed,
    // so set `p' and `pEnd' carefully.
    p = (char*)_file_data.c_str();
    pEnd = p + _file_data.size();

    while (p < pEnd) {
        //
        // get line
        //
        lineno++;
        while (*p == ' ' || *p == '\t' || *p == '\r')    p++;

        pLine = p;
        int shift = 0;
        while (*p != '\n' && p < pEnd)    
        {
            if (*p == '#' || *p == ';')
            {
                if (p != pLine && *(p-1) == '^')
                {
                    shift++;
                }
                else
                {
                    *p = '\0';
                }
            }

            if (shift > 0)
            {
                *(p-shift) = *p;
            }
            p++;
        }
        *(p-shift) = '\0';
        pNextLine = ++p;

        //
        // parse line
        //
        p = pLine;
        
        // skip comment line or empty line
        if (*p == '\0')    goto Next; 

        // collect include line and skip
        if (strstr(p, "@include") == p)
        {
            auto pinclude = (p + strlen("@include"));
            pinclude = utils::trim_string(pinclude);
            if (!load_include(pinclude, arguments))
                goto err;            
            goto Next;
        }

        pEqual = strchr(p, '=');
        if (nullptr == pEqual && *p != '[') {
            goto ConfReg;
        }
        if (nullptr != pEqual && *p == '[') 
            goto err;

        //
        //    conf
        //
        if (pEqual) 
        {
ConfReg:
            if (pSection == nullptr) {
                fprintf(stderr, "ERROR: configuration section not defined\n");
                goto err;
            }
            if (pEqual)    *pEqual = '\0';
            char* pKey = utils::trim_string(p);
            char* pValue = pEqual ? utils::trim_string(++pEqual) : nullptr;
            if (*pKey == '\0')    
                goto err;

            if (pSection->find((const char*)pKey) != pSection->end()) 
            {
                auto it = pSection->find((const char*)pKey);

                fprintf(stderr, "WARNING: overwrite option [%s] %s = (%s => %s) (line %u => %u)\n",
                    pSectionName,
                    pKey,
                    it->second->value.c_str(),
                    pValue,
                    it->second->line,
                    lineno
                );

                it->second->value = pValue ? pValue : "";
                it->second->line = lineno;
            }
            else
            {
                conf* cf = new conf;
                cf->section = (const char*)pSectionName;
                cf->key = pKey;                
                cf->line = lineno; 
                cf->present = true;

                if (pValue)
                {
                    cf->value = pValue;
                }
                else
                {
                    cf->value = "";
                }

                pSection->insert(std::make_pair(std::string(pKey), cf));
            }            
        }
        //
        //    section
        //
        else 
        {
            char* pRight = strchr(p, ']');
            if (nullptr == pRight)   
                goto err;
            *pRight = '\0';
            p++;
            pSectionName = utils::trim_string(p);
            if (*pSectionName == '\0')   
                goto err;

            if (has_section((const char*)pSectionName)) 
            {
                auto it = _configs.find(pSectionName);
                pSection = &it->second;
            }
            else
            {
                std::map<std::string, conf*> sm;
                auto it = _configs.insert(config_map::value_type(std::string(pSectionName), sm));
                assert(it.second);
                pSection = &it.first->second;
            }
        }

        //
        // iterate nextline
        //
Next:
        p = pNextLine;
    }

    //-----------------------------------------------------------
    // STEP: setup parameters from the special [config.args] section, e.g.,
    //
    // [config.args]
    // port = 12345
    //
    // [xyz]
    // key = %port%
    //
    // then [xyz].key will be assigned with 12345
    //
    if (has_section("config.args"))
    {
        std::vector<const char*> keys;
        get_all_keys("config.args", keys);

        for (auto& k : keys)
        {
            std::string key = std::string("%") + k + "%";
            auto value = get_string_value("config.args", k, "", "");

            for (auto& sec : _configs)
            {
                for (auto& kv : sec.second)
                {
                    if (kv.second->value.find(key) != std::string::npos)
                    {
                        kv.second->value = utils::replace_string(
                            kv.second->value, // %port%
                            key,  // %port%
                            value // 12345
                        );

                        fprintf(stderr, "config.config.args: [%s] %s = %s\n",
                            kv.second->section.c_str(),
                            kv.second->key.c_str(),
                            kv.second->value.c_str()
                        );
                    }
                }
            }
        }
    }
    
    //-----------------------------------------------------------
    // STEP: overwrite configs
    //
    if (overwrites != nullptr)
    {
        std::string str_overwrites(overwrites);
        std::list<std::string> argkvs;
        utils::split_args(overwrites, argkvs, ';');
        for (auto& kv : argkvs)
        {
            std::list<std::string> vs;
            utils::split_args(kv.c_str(), vs, '=');
            if (vs.size() != 2 && vs.size() != 1)
            {
                fprintf(stderr, "ERROR: invalid configuration overwrites: '%s' in '%s'\n", kv.c_str(), arguments);
                return false;
            }

            std::string section_and_key = *vs.begin();
            std::string value = vs.size() == 2 ? *vs.rbegin() : "";
            
            auto pos = section_and_key.find_last_of('.');
            if (pos == std::string::npos)
            {
                fprintf(stderr, "ERROR: invalid configuration overwrites: "
                    "'%s' does not represent section.key\n", 
                    section_and_key.c_str()
                );
                return false;
            }

            std::string section = section_and_key.substr(0, pos);
            std::string key = section_and_key.substr(pos + 1);
            
            set(section.c_str(), key.c_str(), value.c_str(), 
                "added by command line"
                );
        }
    }

    return true;
    
err:
    fprintf(stderr, "ERROR: unexpected configuration in %s(line %d): %s\n", file_name, lineno, pLine);
    return false;
}

void configuration::get_all_section_ptrs(std::vector<const char*>& sections)
{
    sections.clear();
    for (auto it = _configs.begin(); it != _configs.end(); ++it)
    {
        sections.push_back(it->first.c_str());
    }
}

void configuration::get_all_sections(std::vector<std::string>& sections)
{
    sections.clear();
    for (auto it = _configs.begin(); it != _configs.end(); ++it)
    {
        sections.push_back(it->first);
    }
}

void configuration::get_all_keys(const char* section, std::vector<const char*>& keys)
{
    std::multimap<int, const char*> ordered_keys;
    keys.clear();
    auto it = _configs.find(section);
    if (it != _configs.end())
    {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            ordered_keys.emplace(it2->second->line, it2->first.c_str());            
        }
    }

    for (auto& k : ordered_keys)
    {
        keys.push_back(k.second);
    }
}

bool configuration::get_string_value_internal(const char* section, const char* key, const char* default_value, const char** ov, const char* dsptr)
{
    _lock.lock();

    std::map<std::string, conf*> *ps = nullptr;
    auto it = _configs.find(section);
    if (it != _configs.end())
    {
        ps = &it->second;
        auto it2 = it->second.find(key);
        if (it2 != it->second.end())
        {
            if (!it2->second->present)
            {
                if (it2->second->value != default_value)
                {
                    fprintf(stderr, "ERROR: configuration default value is different for '[%s] %s': %s <--> %s\n",
                        section, key, it2->second->value.c_str(), default_value);
                    ::abort();
                }
            }

            if (it2->second->dsptr.length() == 0)
                it2->second->dsptr = dsptr;

            *ov = it2->second->value.c_str();
            bool ret = it2->second->present ? true : false;

            _lock.unlock();
            return ret;
        }
    }
    
    if (ps == nullptr)
    {
        std::map<std::string, conf*> sm;
        auto it = _configs.insert(config_map::value_type(std::string(section), sm));
        assert(it.second);
        ps = &it.first->second;
    }

    conf* cf = new conf();
    cf->dsptr = dsptr;
    cf->key = key;
    cf->value = default_value;
    cf->line = 0;
    cf->present = false;
    cf->section = section;
    ps->insert(std::make_pair(cf->key, cf));

    *ov = cf->value.c_str();

    _lock.unlock();
    return false;
}

const char* configuration::get_string_value(const char* section, const char* key, const char* default_value, const char* dsptr)
{
    const char* ov;
    if (!get_string_value_internal(section, key, default_value, &ov, dsptr))
    {
        if (_warning)
        {
            fprintf(stderr, "WARNING: configuration '[%s] %s' is not defined, default value is '%s'\n",
                section,
                key,
                default_value
                );
        }
    }
    return ov;
}

std::list<std::string> configuration::get_string_value_list(const char* section, const char* key, char splitter, const char* dsptr)
{
    const char* ov;
    if (!get_string_value_internal(section, key, "", &ov, dsptr))
    {
        if (_warning)
        {
            fprintf(stderr, "WARNING: configuration '[%s] %s' is not defined, default value is '%s'\n",
                section,
                key,
                ""
                );
        }
    }

    std::list<std::string> vs;
    utils::split_args(ov, vs, splitter);

    for (auto& v : vs)
    {
        v = std::string(utils::trim_string((char*)v.c_str()));
    }
    return vs;
}

void configuration::dump(std::ostream& os)
{
    _lock.lock();

    for (auto& s : _configs)
    {
        os << "[" << s.first << "]" << std::endl;

        std::multimap<int, conf*> ordered_entities;
        for (auto& kv : s.second)
        {
            ordered_entities.emplace(kv.second->line, kv.second);            
        }

        for (auto& kv : ordered_entities)
        {
            os << "; " << kv.second->dsptr << std::endl;
            os << kv.second->key << " = " << kv.second->value << std::endl << std::endl;
        }

        os << std::endl;
    }

    _lock.unlock();
}

void configuration::set(const char* section, const char* key, const char* value, const char* dsptr)
{
    std::map<std::string, conf*>* psection;

    _lock.lock();
    
    auto it = _configs.find(section);
    if (it != _configs.end())
    {
        psection = &it->second;
    }
    else
    {
        std::map<std::string, conf*> s;
        psection = &_configs.insert(config_map::value_type(section, s)).first->second;
    }

    auto it2 = psection->find(key);
    if (it2 == psection->end())
    {
        conf* cf = new conf();
        cf->dsptr = dsptr;
        cf->key = key;
        cf->value = value;
        cf->line = 0;
        cf->present = true;
        cf->section = section;
        psection->insert(std::make_pair(cf->key, cf));
    }
    else
    {        
        fprintf(stderr, "WARNING: overwrite [%s] %s = (%s => %s)\n",
            section,
            key, 
            it2->second->value.c_str(),
            value
            );

        it2->second->value = value;
        it2->second->present = true;
    }

    _lock.unlock();
}

void configuration::register_config_change_notification(config_file_change_notifier notifier)
{
    fprintf(stderr, "ERROR: method register_config_change_notification() not implemented\n");
    ::abort();
}

bool configuration::has_section(const char* section)
{
    auto it = _configs.find(section);
    bool r = (it != _configs.end());
    if (!r && _warning)
    {
        fprintf(stderr, "WARNING: configuration section '[%s]' is not defined, using default settings\n", section);
    }
    return r;
}

bool configuration::has_key(const char* section, const char* key)
{
    auto it = _configs.find(section);
    if (it != _configs.end())
    {
        auto it2 = it->second.find(key);
        return (it2 != it->second.end());
    }
    return false;
}

}
