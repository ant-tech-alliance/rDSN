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
 *     helper for json serialization
 *
 * Revision history:
 *     Dec., 2015, @Tianyi Wang, first version
 *     Jun., 2016, @Weijie Sun, add support for json decode
 */

#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <cctype>
#include <dsn/utility/autoref_ptr.h>
#include <dsn/cpp/auto_codes.h>
#include <dsn/utility/misc.h>
//#include <dsn/cpp/serialization_helper/dsn.layer2.types.h>

#define JsonSplitter "{}[]:,\""

#define JSON_ENCODE_ENTRY(out, prefix, T) out << "\""#T"\":"; ::dsn::json::json_forwarder<std::decay<decltype((prefix).T)>::type>::encode(out, (prefix).T)
#define JSON_ENCODE_ENTRIES2(out, prefix, T1, T2) JSON_ENCODE_ENTRY(out, prefix, T1); out << ","; JSON_ENCODE_ENTRY(out, prefix, T2)
#define JSON_ENCODE_ENTRIES3(out, prefix, T1, T2, T3) JSON_ENCODE_ENTRIES2(out, prefix, T1, T2); out << ","; JSON_ENCODE_ENTRY(out, prefix, T3)
#define JSON_ENCODE_ENTRIES4(out, prefix, T1, T2, T3, T4) JSON_ENCODE_ENTRIES3(out, prefix, T1, T2, T3); out << ","; JSON_ENCODE_ENTRY(out, prefix, T4)
#define JSON_ENCODE_ENTRIES5(out, prefix, T1, T2, T3, T4, T5) JSON_ENCODE_ENTRIES4(out, prefix, T1, T2, T3, T4); out << ","; JSON_ENCODE_ENTRY(out, prefix, T5)
#define JSON_ENCODE_ENTRIES6(out, prefix, T1, T2, T3, T4, T5, T6) JSON_ENCODE_ENTRIES5(out, prefix, T1, T2, T3, T4, T5); out << ","; JSON_ENCODE_ENTRY(out, prefix, T6)
#define JSON_ENCODE_ENTRIES7(out, prefix, T1, T2, T3, T4, T5, T6, T7) JSON_ENCODE_ENTRIES6(out, prefix, T1, T2, T3, T4, T5, T6); out << ","; JSON_ENCODE_ENTRY(out, prefix, T7)
#define JSON_ENCODE_ENTRIES8(out, prefix, T1, T2, T3, T4, T5, T6, T7, T8) JSON_ENCODE_ENTRIES7(out, prefix, T1, T2, T3, T4, T5, T6, T7); out << ","; JSON_ENCODE_ENTRY(out, prefix, T8)

#define JSON_DECODE_ENTRY(in, prefix, T) do {\
    in.expect_token("\""#T"\""); \
    in.expect_token(':'); \
    ::dsn::json::json_forwarder<std::decay<decltype((prefix).T)>::type>::decode(in, (prefix).T); \
} while (0)
#define JSON_DECODE_ENTRIES2(in, prefix, T1, T2) JSON_DECODE_ENTRY(in, prefix, T1); in.expect_token(','); JSON_DECODE_ENTRY(in, prefix, T2)
#define JSON_DECODE_ENTRIES3(in, prefix, T1, T2, T3) JSON_DECODE_ENTRIES2(in, prefix, T1, T2); in.expect_token(','); JSON_DECODE_ENTRY(in, prefix, T3)
#define JSON_DECODE_ENTRIES4(in, prefix, T1, T2, T3, T4) JSON_DECODE_ENTRIES3(in, prefix, T1, T2, T3); in.expect_token(','); JSON_DECODE_ENTRY(in, prefix, T4)
#define JSON_DECODE_ENTRIES5(in, prefix, T1, T2, T3, T4, T5) JSON_DECODE_ENTRIES4(in, prefix, T1, T2, T3, T4); in.expect_token(','); JSON_DECODE_ENTRY(in, prefix, T5)
#define JSON_DECODE_ENTRIES6(in, prefix, T1, T2, T3, T4, T5, T6) JSON_DECODE_ENTRIES5(in, prefix, T1, T2, T3, T4, T5); in.expect_token(','); JSON_DECODE_ENTRY(in, prefix, T6)
#define JSON_DECODE_ENTRIES7(in, prefix, T1, T2, T3, T4, T5, T6, T7) JSON_DECODE_ENTRIES6(in, prefix, T1, T2, T3, T4, T5, T6); in.expect_token(','); JSON_DECODE_ENTRY(in, prefix, T7)
#define JSON_DECODE_ENTRIES8(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8) JSON_DECODE_ENTRIES7(in, prefix, T1, T2, T3, T4, T5, T6, T7); in.expect_token(','); JSON_DECODE_ENTRY(in, prefix, T8)

#define JSON_ENTRIES_GET_MACRO(ph1,ph2,ph3,ph4,ph5,ph6,ph7,ph8, NAME, ...) NAME
//workaround due to the way VC handles "..."
#define JSON_ENTRIES_GET_MACRO_(tuple) JSON_ENTRIES_GET_MACRO tuple

#define JSON_ENCODE_ENTRIES(out, prefix, ...)\
    out<<"{";\
    JSON_ENTRIES_GET_MACRO_((__VA_ARGS__, \
        JSON_ENCODE_ENTRIES8, \
        JSON_ENCODE_ENTRIES7, \
        JSON_ENCODE_ENTRIES6, \
        JSON_ENCODE_ENTRIES5, \
        JSON_ENCODE_ENTRIES4, \
        JSON_ENCODE_ENTRIES3, \
        JSON_ENCODE_ENTRIES2, \
        JSON_ENCODE_ENTRY)) (out, prefix, __VA_ARGS__); \
    out<<"}"

#define JSON_DECODE_ENTRIES(in, prefix, ...)\
    in.expect_token('{');\
    JSON_ENTRIES_GET_MACRO_((__VA_ARGS__,\
        JSON_DECODE_ENTRIES8, \
        JSON_DECODE_ENTRIES7, \
        JSON_DECODE_ENTRIES6, \
        JSON_DECODE_ENTRIES5, \
        JSON_DECODE_ENTRIES4, \
        JSON_DECODE_ENTRIES3, \
        JSON_DECODE_ENTRIES2, \
        JSON_DECODE_ENTRY)) (in, prefix, __VA_ARGS__);\
    in.expect_token('}')

#define DEFINE_JSON_SERIALIZATION(...) \
void encode_json_state(std::stringstream& out) const \
{\
    JSON_ENCODE_ENTRIES(out, *this, __VA_ARGS__);\
}\
void decode_json_state(dsn::json::string_tokenizer& in) \
{\
    JSON_DECODE_ENTRIES(in, *this, __VA_ARGS__);\
}

#define DEFINE_JSON_GLOBAL_SERIALIZATION(T) \
    inline void marshall( ::dsn::binary_writer& writer, const T &value) \
    { \
        std::stringstream out; \
        value.encode_json_state(out);\
        writer.write(out.str()); \
    } \
    inline void unmarshall( ::dsn::binary_reader& reader, T &value) \
    { \
        std::string str; \
        reader.read(str); \
        ::dsn::json::string_tokenizer tokenizer(str); \
        value.decode_json_state(tokenizer); \
    }

#define DEFINE_JSON_GLOBAL_SERIALIZATION_WITHOUT_LENGTH(T) \
    inline void marshall( ::dsn::binary_writer& writer, const T &value) \
    { \
        std::stringstream out; \
        value.encode_json_state(out);\
        auto s = out.str(); \
        writer.write(s.c_str(), s.length()); \
    } \
    inline void unmarshall( ::dsn::binary_reader& reader, T &value) \
    { \
        auto bb = reader.get_remaining_buffer(); \
        std::string str(bb.data(), bb.length()); \
        ::dsn::json::string_tokenizer tokenizer(str); \
        value.decode_json_state(tokenizer); \
    }

#define ENUM_TYPE_SERIALIZATION(EnumType, InvalidEnum) \
inline void json_encode(std::stringstream& out, const EnumType& enum_variable)\
{\
    out << "\"" << enum_to_string(enum_variable) << "\"";\
}\
inline void json_decode(dsn::json::string_tokenizer& in, EnumType& enum_variable)\
{\
    std::string status_message;\
    dsn::json::json_decode(in, status_message);\
    enum_variable = enum_from_string(status_message.c_str(), InvalidEnum);\
}

namespace dsn { namespace json {

class string_tokenizer
{
private:
    const char* buffer;
    unsigned pos;
    unsigned length;
public:
    static bool is_json_splitter(char token)
    {
        const char* s = JsonSplitter;
        for (int i=0; s[i]; ++i)
            if (s[i]==token)
                return true;
        return false;
    }
public:
    string_tokenizer(const char* b, unsigned offset, unsigned len): buffer(b), pos(offset), length(len)
    {
        dassert(pos<length, "");
    }
    string_tokenizer(const dsn::blob& source, unsigned from): string_tokenizer(source.data(), from, source.length()) {}
    string_tokenizer(const dsn::blob& source): string_tokenizer(source.data(), 0, source.length()) {}
    string_tokenizer(const std::string& source, unsigned from): string_tokenizer(source.c_str(), from, source.size()) {}
    string_tokenizer(const std::string& source): string_tokenizer(source.c_str(), 0, source.size()) {}

    void expect_token(const char* token)
    {
        while (pos<length && isblank(buffer[pos])) ++pos;
        int j=0;
        while (pos<length && token[j]!=0 && buffer[pos]==token[j])
            ++pos, ++j;
        if (token[j]!=0)
        {
            dassert(false, "invalid buffer:%s at pos %d", buffer, pos);
        }
    }
    void expect_token(char token)
    {
        while (pos<length && isblank(buffer[pos])) ++pos;
        if (pos<length && buffer[pos] == token)
            ++pos;
        else
        {
            dassert(false, "invalid buffer:%s at pos %d", buffer, pos);
        }
    }
    char peek_next() const
    {
        int i = pos;
        while (i<length && isblank(buffer[i])) ++i;
        return buffer[i];
    }
    void walk_until(char token)
    {
        while (pos<length && buffer[pos]!=token) ++pos;
        if (pos<length && buffer[pos]==token)
            return;
        else
        {
            dassert(false, "invalid buffer:%s at pos %d", buffer, pos);
        }
    }
    void walk_until_json_splitter()
    {
        while (pos<length && !is_json_splitter(buffer[pos])) ++pos;
        return;
    }
    unsigned tell() const { return pos; }
    void forward() { ++pos; }
    //assign substring [from, to) in buffer to output
    void assign(unsigned from, unsigned to, std::string& output)
    {
        output.assign(buffer+from, to-from);
    }
};

template<typename> class json_forwarder;

#define DSN_BASE_TYPE_JSON_STATE(TName) \
inline void json_encode(std::stringstream& out, const TName& t) \
{\
    out << t;\
}\
inline void json_decode(string_tokenizer& in, TName& t)\
{\
    int start_pos = in.tell();\
    in.walk_until_json_splitter();\
    std::string string_data;\
    in.assign(start_pos, in.tell(), string_data);\
    std::istringstream is(string_data);\
    is >> t;\
}

//DSN_BASE_TYPE_JSON_STATE(bool)
DSN_BASE_TYPE_JSON_STATE(float)
DSN_BASE_TYPE_JSON_STATE(double)
DSN_BASE_TYPE_JSON_STATE(int8_t)
DSN_BASE_TYPE_JSON_STATE(int16_t)
DSN_BASE_TYPE_JSON_STATE(int32_t)
DSN_BASE_TYPE_JSON_STATE(int64_t)
DSN_BASE_TYPE_JSON_STATE(uint8_t)
DSN_BASE_TYPE_JSON_STATE(uint16_t)
DSN_BASE_TYPE_JSON_STATE(uint32_t)
DSN_BASE_TYPE_JSON_STATE(uint64_t)

inline void json_encode(std::stringstream& out, const std::string& t)
{
    out << "\"" << t << "\"";
}

inline void json_encode(std::stringstream& out, const char* t)
{
    out << "\"" << t << "\"";
}

inline void json_decode(string_tokenizer& in, std::string& t)
{
    in.expect_token('\"');
    unsigned start_pos = in.tell();
    in.walk_until('\"');
    in.assign(start_pos, in.tell(), t);
    in.forward();
}

inline void json_encode(std::stringstream& out, const bool t)
{
    out << t;
}

inline void json_decode(string_tokenizer& in, bool& t)
{
    int start_pos = in.tell();
    in.walk_until_json_splitter();
    std::string string_data;
    in.assign(start_pos, in.tell(), string_data);
    std::istringstream is(string_data);
    is >> std::boolalpha >> t;
}

/*
ENUM_TYPE_SERIALIZATION(dsn::app_status::type, dsn::app_status::AS_INVALID)

inline void json_encode(std::stringstream& out, const dsn::gpid& pid)
{
    out << "\"" << pid.get_app_id() << "." << pid.get_partition_index() << "\"";
}
inline void json_decode(dsn::json::string_tokenizer& in, dsn::gpid& pid)
{
    std::string gpid_message;
    json_decode(in, gpid_message);
    dsn_global_partition_id c_gpid;
    sscanf(gpid_message.c_str(), "%d.%d", &c_gpid.u.app_id, &c_gpid.u.partition_index);
    pid = dsn::gpid(c_gpid);
}
*/
inline void json_encode(std::stringstream& out, const dsn::rpc_address& address)
{
    out << "\"" << address.to_string() << "\"";
}
inline void json_decode(dsn::json::string_tokenizer& in, dsn::rpc_address& address)
{
    std::string rpc_address_string;
    json_decode(in, rpc_address_string);
    address.from_string_ipv4(rpc_address_string.c_str());
}
/*
inline void json_encode(std::stringstream& out, const dsn::partition_configuration& config);
inline void json_decode(string_tokenizer& in, dsn::partition_configuration& config);
inline void json_encode(std::stringstream& out, const dsn::app_info& info);
inline void json_decode(string_tokenizer& in, dsn::app_info& info);
*/

template<typename T> inline void json_encode_iterable(std::stringstream& out, const T& t)
{
    out << "[";
    for (auto it = t.begin(); it != t.end(); ++it)
    {
        json_forwarder<typename std::decay < decltype(*it) > ::type>::encode(out, *it);
        if (std::next(it) != t.end())
        {
            out << ",";
        }
    }
    out << "]";
}

template<typename T> inline void json_encode_map(std::stringstream& out, const T& t)
{
    out << "{";
    for (auto it = t.begin(); it != t.end(); ++it)
    {
        json_forwarder<typename std::decay < decltype(it->first) > :: type>::encode(out, it->first);
        out << ":";
        json_forwarder<typename std::decay < decltype(it->second) > ::type>::encode(out, it->second);
        if (std::next(it) != t.end())
        {
            out << ",";
        }
    }
    out << "}";
}

template<typename TMap> inline void json_decode_map(string_tokenizer& in, TMap& t)
{
    t.clear();
    in.expect_token('{');
    while (in.peek_next() != '}')
    {
        if (!t.empty())
        {
            in.expect_token(',');
        }
        typename TMap::key_type key;
        typename TMap::mapped_type value;
        json_forwarder< decltype(key) >::decode(in, key);
        in.expect_token(':');
        json_forwarder< decltype(value) >::decode(in, value);

        if (!t.emplace(key, value).second)
        {
            throw in.tell();
        }
    }
    in.expect_token('}');
}

template<typename T> inline void json_encode(std::stringstream& out, const std::vector<T>& t)
{
    json_encode_iterable(out, t);
}

template<typename T> inline void json_decode(string_tokenizer& in, std::vector<T>& t)
{
    t.clear();
    in.expect_token('[');
    while (in.peek_next() != ']')
    {
        if (!t.empty())
        {
            in.expect_token(',');
        }
        T result;
        json_forwarder<T>::decode(in, result);
        t.emplace_back(std::move(result));
    }
    in.expect_token(']');
}

template<typename T> inline void json_encode(std::stringstream& out, const std::set<T>& t)
{
    json_encode_iterable(out, t);
}

template <typename T> inline void json_decode(string_tokenizer& in, std::set<T>& t)
{
    t.clear();
    in.expect_token('[');
    while (in.peek_next() != ']')
    {
        if (!t.empty())
        {
            in.expect_token(',');
        }
        T result;
        json_forwarder<T>::decode(in, result);
        if (!t.emplace(std::move(result)).second)
        {
            throw in.tell();
        }
    }
    in.expect_token(']');
}

template<typename T1, typename T2> inline void json_encode(std::stringstream& out, const std::unordered_map<T1, T2>& t)
{
    json_encode_map(out, t);
}

template<typename T1, typename T2> inline void json_decode(string_tokenizer& in, std::unordered_map<T1, T2>& t)
{
    json_decode_map(in, t);
}

template<typename T1, typename T2> inline void json_encode(std::stringstream& out, const std::map<T1, T2>& t)
{
    json_encode_map(out, t);
}

template<typename T1, typename T2> inline void json_decode(string_tokenizer& in, std::map<T1, T2>& t)
{
    json_decode_map(in, t);
}

template<typename T> inline void json_encode(std::stringstream& out, const dsn::ref_ptr<T>& t)
{
    json_encode(out, *t);
}

template<typename T> inline void json_decode(string_tokenizer& in, dsn::ref_ptr<T>& t)
{
    json_decode(in, *t);
}

template<typename T> inline void json_encode(std::stringstream& out, const std::shared_ptr<T>& t)
{
    json_encode(out, *t);
}

template<typename T> inline void json_decode(string_tokenizer& in, std::shared_ptr<T>& t)
{
    json_decode(in, *t);
}

template<typename T>
class json_forwarder {
private:
    //check if C has C.encode_json_state(sstream&) function
    template<typename C>
    static auto check_json_state(C*)
        -> typename std::is_same<decltype(std::declval<C>().encode_json_state(std::declval<std::stringstream&>())), void>::type;

    template<typename>
    static std::false_type check_json_state(...);

    //check if C has C->json_state(sstream&) function
    template<typename C>
    static auto p_check_json_state(C*)
        -> typename std::is_same<decltype(std::declval<C>()->encode_json_state(std::declval<std::stringstream&>())),void>::type;

    template<typename>
    static std::false_type p_check_json_state(...);

    typedef decltype(check_json_state<T>(0)) has_json_state;
    typedef decltype(p_check_json_state<T>(0)) p_has_json_state;

    //internal serialization
    static void encode_inner(std::stringstream&out, const T& t, std::true_type, std::false_type)
    {
        t.encode_json_state(out);
    }
    static void encode_inner(std::stringstream&out, const T& t, std::false_type, std::true_type)
    {
        t->encode_json_state(out);
    }
    static void encode_inner(std::stringstream&out, const T& t, std::true_type, std::true_type)
    {
        t->encode_json_state(out);
    }
    static void encode_inner(std::stringstream&out, const T& t, std::false_type, std::false_type)
    {
        json_encode(out, t);
    }

    //internal deserialization
    static void decode_inner(string_tokenizer& in, T& t, std::true_type, std::false_type)
    {
        t.decode_json_state(in);
    }
    static void decode_inner(string_tokenizer& in, T& t, std::false_type, std::true_type)
    {
        t->decode_json_state(in);
    }
    static void decode_inner(string_tokenizer& in, T& t, std::true_type, std::true_type)
    {
        t->decode_json_state(in);
    }
    static void decode_inner(string_tokenizer& in, T& t, std::false_type, std::false_type)
    {
        json_decode(in, t);
    }
public:
    static void encode(std::stringstream& out, const T& t)
    {
        encode_inner(out, t, has_json_state{}, p_has_json_state{});
    }
    static dsn::blob encode(const T& t)
    {
        std::stringstream out;
        encode_inner(out, t, has_json_state{}, p_has_json_state{});
        std::string* result = new std::string(out.str());
        return dsn::blob( std::shared_ptr<char>( const_cast<char*>(result->c_str()), [result](char*){delete result;} ), result->length() );
    }
    static void decode(string_tokenizer& in, T& t)
    {
        decode_inner(in, t, has_json_state{}, p_has_json_state{});
    }
    static bool decode(const dsn::blob& bb, T& t)
    {
        dsn::json::string_tokenizer tokenizer(bb);
        try
        {
            decode(tokenizer, t);
            return true;
        }
        catch (...)
        {
            return false;
        }
        return true;
    }
};
/*
inline void json_encode(std::stringstream& out, const dsn::partition_configuration& config)
{
    JSON_ENCODE_ENTRIES(out, config, pid, ballot, max_replica_count, primary, secondaries, last_drops, last_committed_decree);
}
inline void json_decode(dsn::json::string_tokenizer& in, dsn::partition_configuration& config)
{
    JSON_DECODE_ENTRIES(in, config, pid, ballot, max_replica_count, primary, secondaries, last_drops, last_committed_decree);
}
inline void json_encode(std::stringstream& out, const dsn::app_info& info)
{
    JSON_ENCODE_ENTRIES(out, info, status, app_type, app_name, app_id, partition_count, envs, is_stateful, max_replica_count);
}
inline void json_decode(dsn::json::string_tokenizer& in, dsn::app_info& info)
{
    JSON_DECODE_ENTRIES(in, info, status, app_type, app_name, app_id, partition_count, envs, is_stateful, max_replica_count);
}
*/
}}
