#ifndef XML_H
#define XML_H

#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include <vector>
#include <map>

#include <bravo/reader.h>
#include <bravo/xml_element.h>
#include <bravo/string_utils.h>

using namespace bravo;

const char *bin_to_char64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int char64_to_bin[] = {
    //                            8  9  A  B
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    // 0
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    // 1
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0,62,    0, 0, 0,63,    // 2
   52,53,54,55,   56,57,58,59,   60,61, 0, 0,    0, 0, 0, 0,    // 3

    0, 0, 1, 2,    3, 4, 5, 6,    7, 8, 9,10,   11,12,13,14,    // 4
   15,16,17,18,   19,20,21,22,   23,24,25, 0,    0, 0, 0, 0,    // 5
    0,26,27,28,   29,30,31,32,   33,34,35,36,   37,38,39,40,    // 6
   41,42,43,44,   45,46,47,48,   49,50,51, 0,    0, 0, 0, 0,    // 7

    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,

    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0
};

std::vector<char> decode_base64(const char *src, size_t size)
{
    if (!src || !size)
        return std::vector<char>();
    
    size_t padding = 0;
    for(int r = (int)size - 1; r >= 0; --r)
    {
        if(src[r] != '=')
            break;
        
        ++padding;
    }
     
    std::vector<char> bin;
    size_t i = 0;
    
    size_t count = 0;
    
    if (size >= 4)
    {
        size -= 4;
        count = 4;
    }
    else
    {
        count = size;
        size = 0;
    }
    
    while(count)
    {
        switch(count)
        {            
            case 1:
                bin.push_back(0xFC & (char64_to_bin[(unsigned char)src[i]] << 2)); 
                ++i;
                break;  
                
            case 2:
                bin.push_back((0xFC & (char64_to_bin[(unsigned char)src[i]] << 2)) | (0x03 & (char64_to_bin[(unsigned char)src[i+1]] >> 4))); 
                ++i;
                
                if (src[i] == '=')
                    bin.push_back(0xF0 & (char64_to_bin[(unsigned char)src[i]] << 4)); 
                ++i;
                break;  
                
            case 3:
                bin.push_back((0xFC & (char64_to_bin[(unsigned char)src[i]] << 2)) | (0x03 & (char64_to_bin[(unsigned char)src[i+1]] >> 4))); 
                ++i;
                
                bin.push_back((0xF0 & (char64_to_bin[(unsigned char)src[i]] << 4)) | (0x0F & (char64_to_bin[(unsigned char)src[i+1]] >> 2))); 
                ++i;
                
                if (src[i] == '=')
                    bin.push_back(0xC0 & (char64_to_bin[(unsigned char)src[i]] << 6)); 
                i += 2;
                break;  
                
            case 4:
                bin.push_back((0xFC & (char64_to_bin[(unsigned char)src[i]] << 2)) | (0x03 & (char64_to_bin[(unsigned char)src[i+1]] >> 4))); 
                ++i;
                
                bin.push_back((0xF0 & (char64_to_bin[(unsigned char)src[i]] << 4)) | (0x0F & (char64_to_bin[(unsigned char)src[i+1]] >> 2))); 
                ++i;
                
                bin.push_back((0xC0 & (char64_to_bin[(unsigned char)src[i]] << 6)) | (0x3F & char64_to_bin[(unsigned char)src[i+1]])); 
                i += 2;
                break;
        }
        
        if (size >= 4)
        {
            size -= 4;
            count = 4;
        }
        else
        {
            count = size;
            size = 0;
        }
    }
    
    for(int k = 0; k < padding; ++k)
        bin.pop_back();
    
    return bin;
}

std::vector<char> decode_base64(const std::string &str)
{
    return decode_base64(str.c_str(), str.size());
}

std::string encode_base64(const char *bin, size_t size)
{
    std::stringstream ss;
    int i = 0;
    int num;
    size_t count = 0;
    
    if (size >= 3)
    {
        count = 3;
    }
    else
    {
        count = size;
    }
    
    if(size >= 3)
        size -= 3;
    else
        size = 0;
    
    while(count)
    {
        switch(count)
        {            
            case 1:
                num = 0x3F & (((unsigned char)bin[i]) >> 2);
                ss << bin_to_char64[num];            
                
                num = (0x30 & (((unsigned char)bin[i]) << 4));
                ss << bin_to_char64[num];  
                
                ss << "==";           
                break;
                
            case 2:
                num = 0x3F & (((unsigned char)bin[i]) >> 2);
                ss << bin_to_char64[num];            
                
                num = (0x30 & (((unsigned char)bin[i]) << 4)) | (0x0F & (((unsigned char)bin[i+1]) >> 4));
                ss << bin_to_char64[num];  
                ++i;
                
                num = (0x3C & (((unsigned char)bin[i]) << 2));
                ss << bin_to_char64[num];  
                ++i;
                
                ss << '=';  
                break;
                
            case 3:
                num = 0x3F & (((unsigned char)bin[i]) >> 2);
                ss << bin_to_char64[num];            
                
                num = (0x30 & (((unsigned char)bin[i]) << 4)) | (0x0F & (((unsigned char)bin[i+1]) >> 4));
                ss << bin_to_char64[num];  
                ++i;
                
                num = (0x3C & (((unsigned char)bin[i]) << 2)) | (0x03 & (((unsigned char)bin[i+1]) >> 6));
                ss << bin_to_char64[num];  
                ++i;
                
                num = 0x3F & ((unsigned char)bin[i]);
                ss << bin_to_char64[num];  
                ++i;
                break;                
            }
    
        if (size >= 3)
        {
            count = 3;
        }
        else
        {
            count = size;
        }
        
        if(size >= 3)
            size -= 3;
        else
            size = 0;
    }
    
    return ss.str();
}

std::string encode_base64(const std::string &str)
{
    return encode_base64(str.c_str(), str.size());
}

struct entity_maps_t
{
    entity_maps_t()
    {
        esc_all["<"] = "&lt;";
        esc_all[">"] = "&gt;";
        esc_all["&"] = "&amp;";
        esc_all["'"] = "&apos;";
        esc_all["\""] = "&quot;";     
        
        esc_lt_amp["<"] = "&lt;";
        esc_lt_amp["&"] = "&amp;";
        
        esc_quot["\""] = "&quot;";     
        
        esc_apos["'"] = "&apos;";        
        unesc_all["&lt;"] = "<";
        unesc_all["&gt;"] = ">";
        unesc_all["&amp;"] = "&";
        unesc_all["&apos;"] = "'";
        unesc_all["&quot;"] = "\"";     
        
        unesc_lt_amp["&lt;"] = "<";
        unesc_lt_amp["&amp;"] = "&";
        
        unesc_quot["&quot;"] = "\"";     
        
        unesc_apos["&apos;"] = "'";
    }
    
    std::map<std::string,std::string> esc_all;
    std::map<std::string,std::string> esc_lt_amp;
    std::map<std::string,std::string> esc_quot;
    std::map<std::string,std::string> esc_apos;

    std::map<std::string,std::string> unesc_all;
    std::map<std::string,std::string> unesc_lt_amp;
    std::map<std::string,std::string> unesc_quot;
    std::map<std::string,std::string> unesc_apos;
};

entity_maps_t entity_maps;

int cdata_end_count(const std::string &str)
{
    int count = 0;
    size_t start = 0;
    size_t pos = str.find("]]>", start);
    
    while(pos != std::string::npos)
    {
        ++count;
        start = pos + 3;
        pos = str.find("]]>", start);
    }
    
    return count;
}

struct entity_counts
{
    entity_counts() : lt(0), gt(0), apos(0), quot(0), amp(0) {}
    int lt;
    int gt;
    int apos;
    int quot;
    int amp;
};

void char_counts(const std::string &str, entity_counts &ec)
{
    int counts[256]={0};
    for(auto c : str)
    {
        counts[(unsigned char)c]++;
    }
    
    ec.lt      = counts['<'];
    ec.gt      = counts['>'];
    ec.apos    = counts['\''];
    ec.quot    = counts['"'];
    ec.amp     = counts['&'];
}

int alpha_num[] =
{
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // 00
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // 10
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // 20
    1,1,1,1,    1,1,1,1,    1,0,0,0,    0,0,0,0,    // 30
    
    0,1,1,1,    1,1,1,1,    1,1,1,1,    1,1,1,1,    // 40 A-Z
    1,1,1,1,    1,1,1,1,    1,1,1,0,    0,0,0,1,    // 50 _
    0,1,1,1,    1,1,1,1,    1,1,1,1,    1,1,1,1,    // 60 a-z
    1,1,1,1,    1,1,1,1,    1,1,1,0,    0,0,0,0,    // 70
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // 80
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // 90
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // A0
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // B0
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // C0
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // D0
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,    // E0
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0     // F0
};

#define S_MATCHING                  1
#define S_SEARCHING_FOR_WORD_START  2
#define S_SEARCHING_FOR_WORD_END    3

inline size_t find_whole_word(std::string &text, const std::string &find_string, size_t pos)
{
    size_t find_count = find_string.size();
    size_t text_count = text.size();
    
    if ((find_count == 0) || (text_count == 0) || (pos == std::string::npos) || (pos > (text_count - 1)))
    {
        return std::string::npos;
    }
    
    int prev_alpha_num = 0;
    int curr_alpha_num = 0;
    int boundary = 0;
    char char1 = ' ';
    int match_count = 0;
    size_t find_index = 0;
    size_t head = std::string::npos;
    
    int state = S_SEARCHING_FOR_WORD_START;
    for (size_t text_index = pos; text_index < text_count; text_index++)
    {
        char1 = text[text_index];
        curr_alpha_num = alpha_num[char1];
        boundary = curr_alpha_num - prev_alpha_num;
        prev_alpha_num = curr_alpha_num;
        
        switch (state)
        {
            case S_SEARCHING_FOR_WORD_START:
                if (boundary > 0)
                {
                    find_index = 0;
                    if (text[text_index] == find_string[find_index])
                    {
                        match_count = 1;
                        head = text_index;
                    }
                    else
                    {
                        match_count = 0;
                    }
                    
                    state = S_SEARCHING_FOR_WORD_END;
                    
                }
                break;
                
            case S_SEARCHING_FOR_WORD_END:
                
                if (boundary < 0)
                {
                    state = S_SEARCHING_FOR_WORD_START;
                    if (match_count == find_count)
                    {
                        return head;
                    }
                }
                else
                {
                    if (match_count > 0)
                    {
                        find_index++;
                        
                        if (text[text_index] == find_string[find_index])
                        {
                            match_count++;
                        }
                        else
                        {
                            match_count = 0;
                        }
                    }
                }
                break;
        }
    }
    
    if (match_count == find_count)
    {
        return head;
    }
    
    return std::string::npos;
}

inline int find_whole_word_positions(std::string &text, const std::string &find_string, std::vector<size_t> &pos)
{
    pos.clear();
    
    if (text.size() == 0)
    {
        return 0;
    }
    
    size_t start = 0;
    size_t f = 0;
    
    while (1)
    {
        f = find_whole_word(text, find_string, start);
        
        if (f == std::string::npos)
        {
            return 0;
        }
        
        pos.push_back(f);
        
        start = f + find_string.size();
        
        if (start > text.size() - 1)
        {
            return 0;
        }
    }
    
    return 0;
}


inline int find_positions1(std::string &text, const std::string &find_string, std::vector<size_t> &pos)
{
    pos.clear();
    
    if (text.size() == 0)
    {
        return 0;
    }
    
    size_t start = 0;
    size_t f = 0;
    
    while (1)
    {
        f = text.find(find_string, start);
        
        if (f == std::string::npos)
        {
            return 0;
        }
        
        pos.push_back(f);
        
        start = f + find_string.size();
        
        if (start > text.size() - 1)
        {
            return 0;
        }
    }
    
    return 0;
}


inline int replace_all1(std::string &text, const std::string &find_string, const std::string &replace_string)
{
    if (text.size() == 0)
    {
        return 0;
    }
    
    size_t start = 0;
    size_t f = 0;
    
    while (1)
    {
        f = text.find(find_string, start);
        
        if (f == std::string::npos)
        {
            return 0;
        }
        
        text.replace(f, find_string.size(), replace_string);
        
        start = f + replace_string.size();
        
        if (start > text.size() - 1)
        {
            return 0;
        }
    }
}

inline int replace_all_whole_words(std::string &text, const std::vector<std::string> &find_strings, const std::vector<std::string> &replace_strings)
{
    if (text.size() == 0)
    {
        return -1;
    }
    
    std::map<size_t, int> pos_to_string;
    
    for (int s = 0; s < (int)find_strings.size(); s++)
    {
        std::vector<size_t> pos;
        find_whole_word_positions(text, find_strings[s], pos);
        
        for (auto p : pos)
        {
            pos_to_string[p] = s;
        }
    }
    
    size_t delta = 0;
    
    for (auto pair1 : pos_to_string)
    {
        text.replace(pair1.first + delta, find_strings[pair1.second].size(), replace_strings[pair1.second]);
        delta += replace_strings[pair1.second].size() - find_strings[pair1.second].size();
    }
    
    return 0;
}

inline int replace_all1(std::string &text, const std::map<std::string, std::string> &replace_map)
{
    if (text.size() == 0)
    {
        return -1;
    }
    
    std::map<size_t, int> pos_to_string;
    
    std::vector<std::map<std::string, std::string>::const_iterator> index_to_iterator;
    for (auto it = replace_map.begin(); it != replace_map.end(); it++)
    {
        
        std::vector<size_t> pos;
        find_positions(text, it->first, pos);
        
        for (auto p : pos)
        {
            pos_to_string[p] = (int)index_to_iterator.size();
        }
        
        index_to_iterator.push_back(it);
    }
    
    size_t delta = 0;
    
    for (auto pair1 : pos_to_string)
    {
        text.replace(pair1.first + delta, index_to_iterator[pair1.second]->first.size(), index_to_iterator[pair1.second]->second);
        delta += index_to_iterator[pair1.second]->second.size() - index_to_iterator[pair1.second]->first.size();
    }
    
    return 0;
}

inline int replace_all2(std::string &text, const std::vector<std::string> &find_strings, const std::vector<std::string> &replace_strings)
{
    if (text.size() == 0)
    {
        return -1;
    }
    
    if (find_strings.size() != replace_strings.size())
    {
        return -2;
    }
    
    int count = (int)find_strings.size();
    
    for (int i = 0; i < count; i++)
    {
        //        replace_all(text,find_strings[i], replace_strings[i]);
    }
    
    return 0;
}

std::string cdata_escape(const std::string &str)
{
    std::string r;
    size_t start = 0;
    size_t end = std::string::npos; 
    size_t len = std::string::npos; 
    size_t pos = str.find("]]>", start);
    while(pos != std::string::npos)
    {
        end = pos + 2;
        len = end - start;
        r += std::string("<![CDATA[") + str.substr(start,len) + "]]>"; 
        start = end;
        pos = str.find("]]>", start);
    }

    if (start < str.size())
    {
        r += std::string("<![CDATA[") + str.substr(start) + "]]>"; 
    }    

    return r;
}

std::string _escape(const std::string &str)
{
    std::map<std::string, std::string> m;
    m["<"] = "&lt;";
    m[">"] = "&gt;";
    m["&"] = "&amp;";
    m["'"] = "&apos;";
    m["\""] = "&quot;";
    std::string s = str;
    replace_all(s,m);
    return s;
}

std::string _unescape(const std::string &str)
{
    std::map<std::string, std::string> m;
    m["&lt;"] = "<";
    m["&gt;"] = ">";
    m["&amp;"] = "&";
    m["&apos;"] = "'";
    m["&quot;"] = "\"";
    std::string s = str;
    replace_all(s,m);
    return s;
}

std::string escape_and_quote(const std::string &str)
{
    entity_counts ec;
    char_counts(str, ec);
    
    if (ec.quot > ec.apos)
    {
        if(ec.apos)
        {
            std::string s = str;            
            replace_all(s,entity_maps.esc_apos);
            return std::string("'") + s + "'";
        }
        else
        {
            return std::string("'") + str + "'";
        }
    }
    else
    {
        if(ec.quot)
        {
            std::string s = str;            
            replace_all(s,entity_maps.esc_quot);
            return std::string("\"") + s + "\"";
        }
        else
        {
            return std::string("\"") + str + "\"";
        }
    }
    
    return "";
}

std::string unescape_quote(const std::string &str)
{
    if (!str.size())
        return "";
    
    std::string s = str.substr(1, str.size()-2);
    
    if (str[0] == '"')
    {
        replace_all(s,entity_maps.unesc_all);
    }
    else
    {
        replace_all(s,entity_maps.unesc_all);
    }
    
    return s;
}

std::string escape_body(const std::string &str)
{
    entity_counts ec;
    char_counts(str, ec);
    
    int total1 = ec.amp*3 + ec.lt*2;
    
    // <![CDATA[
    if (total1 >= 12 )
    {
        int count = cdata_end_count(str);        
        int total2 = count*9 + 12;
        
        if(total1 >= total2)
        {
            return cdata_escape(str);
        }
    }
    
    std::string s = str;
    replace_all(s,entity_maps.esc_lt_amp);

    return s;
}

std::string unescape_body(const std::string &str)
{
    std::string s = str;
    replace_all(s,entity_maps.unesc_all);
    return s;
}

// DFA

#define S_NAME      1
#define S_PROP_NAME 2
#define S_PROP_EQ   3
#define S_PROP_VAL  4

/*
#define C_QM        10
#define C_EX_POINT  11
#define C_OPEN_SQ   12
#define C_C         13
#define C_D         14    
#define C_A         15
#define C_T         16
#define C_CLOSE_SQ  17
#define C_DASH      18
#define C_SINGLE_Q  19
#define UT          20 // utf8
*/

// Null state
#define NL 0        

// 1 Start state
#define ST (NL+1)

// 2 Text state
#define TX (ST+1)

// 2 Open bracket  < 
#define OB (TX+1)

// 3 Name  <a 
#define NM (OB+1)

// 4 Space before prop  <a_
#define SP (NM+1)

// 5 Property name  <a n 
#define PN (SP+1)

// 6 Space before equals  <a n_
#define SQ (PN+1)

// 7 Equals  <a n=      
#define EQ (PN+2)   

// 8 Double quote value  <a n="
#define VQ (EQ+1)   

// 9 Single quote value  <a n='
#define VS (VQ+1)   

// 10 End slash  <a / 
#define ED (VS+1)   

// 11 End name  < / a
#define EN (ED+1)   

// 12 Space before close bracket  < / a_
#define SC (ED+2)   

// 13 Exclamation point  <!
#define EX (SC+1)   

const char *state_names[34] = {
    "NL",
    "ST",
    "OB",
    "NM",
    "SP",
    "PN",
    "SQ",    
    "EQ",
    "VQ",
    "VS",    
    "ED",
    "EN",
    "SC",    
    "EX",    
    "C1",
    "C2",
    "C3",
    "C4",
    "C5",
    "C6",
    "C7",
    "C8",
    "C9",    
    "D1",
    "DQ",
    "DS",    
    "DB",
    "E1",
    "Q2",
    "Q1",    
    "M1",
    "M2",
    "M3",
    "M4"
};

// <![CDATA[ ]]>
#define C1 (EX+1)   // <![
#define C2 (C1+1)   // <![C
#define C3 (C1+2)   // <![CD
#define C4 (C1+3)   // <![CDA
#define C5 (C1+4)   // <![CDAT
#define C6 (C1+5)   // <![CDATA
#define C7 (C1+6)   // <![CDATA[
#define C8 (C1+7)   // ]
#define C9 (C1+8)   // ]]

// DTD elements
#define D1 (C9+1)   // <!D
#define DQ (D1+1)   // <!D"
#define DS (D1+2)   // <!D'
#define DB (D1+3)   // <!D[
#define E1 (D1+4)   // <!D[<
#define Q2 (D1+5)   // <!D[<"
#define Q1 (D1+6)   // <!D[<'

// <!-- --> Comment
#define M1 (Q1+1)   // <!-
#define M2 (M1+1)   // <!--
#define M3 (M1+2)   // -
#define M4 (M1+3)   // --

int states[][21] = {

    // Null State
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   // NL Null state
    
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U
    { 0, TX, TX, TX, TX, OB, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX },   // ST Start state
    
    // Start state, text
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U
    { 0, TX, TX, TX, TX, OB, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX, TX },   // TX Text abc def, body
    
    // Open bracket  < 
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, NM,  0,  0, OB,  0,  0, ED,  0,  0, OB, EX,  0, NM, NM, NM, NM,  0,  0,  0, NM},   // OB <
    
    // Name  <a 
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, NM, NM,  0, SP,  0, ST, ED,  0,  0,  0,  0,  0, NM, NM, NM, NM,  0,  0,  0, NM },   // NM < n
        
    // Space before prop  <n_
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, PN,  0,  0, SP,  0, ST, ED,  0,  0, SP,  0,  0, PN, PN, PN, PN,  0,  0,  0, PN },   // SP

    // Property name  <a n 
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, PN, PN,  0, SQ,  0,  0,  0, EQ,  0,  0,  0,  0, PN, PN, PN, PN,  0,  0,  0, PN },   // PN

    // Space before equals  <a n_
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0,  0,  0,  0, SQ,  0,  0,  0, EQ,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   // SQ

    // Equals  <a n=      
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0,  0,  0, VQ, EQ,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, VS,  0 },   // EQ

    // Double quote value  <a n="
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, VQ, VQ, SP, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ, VQ },   // VQ
    
    // Single quote value  <a n='
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, VS, SP, VS },   // VS
    
    // End slash  <a / 
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, EN,  0,  0, ED,  0, ST,  0,  0,  0,  0,  0,  0, EN, EN, EN, EN,  0,  0,  0, EN },   // ED
    
    // End name  < / a
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, EN, EN,  0, SC,  0, ST,  0,  0,  0,  0,  0,  0, EN, EN, EN, EN,  0,  0,  0, EN },   // EN

    // Space before close bracket  < / a_
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0,  0,  0,  0, SC,  0, ST,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   // SC
    
    // Exclamation point  <!
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, D1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, C1, D1, D1, D1, D1,  0, M1,  0, D1 },   // EX !
    
    // <![CDATA[ ]]>
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, C2,  0,  0,  0,  0,  0,  0,  0 },   // C1 ![
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, C3,  0,  0,  0,  0,  0,  0 },   // C2 ![C 
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, C4,  0,  0,  0,  0,  0 },   // C3 ![CD
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, C5,  0,  0,  0,  0 },   // C4 ![CDA
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, C6,  0,  0,  0,  0,  0 },   // C5 ![CDAT
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, C7,  0,  0,  0,  0,  0,  0,  0,  0 },   // C6 ![CDATA
    { 0, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C8, C7, C7, C7 },   // C7 ![CDATA[
    { 0, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C9, C7, C7, C7 },   // C8 ]
    { 0, C7, C7, C7, C7, C7, ST, C7, C7, C7, C7, C7, C7, C7, C7, C7, C7, C9, C7, C7, C7 },   // C9 ]]

/*
// DTD elements
#define D1 (C9+1)   // <!D
#define DQ (D1+1)   // <!D"
#define DS (D1+2)   // <!D'
#define DB (D1+3)   // <!D[
#define E1 (D1+4)   // <!D[<
#define Q2 (D1+5)   // <!D[<"
#define Q1 (D1+6)   // <!D[<'
*/
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0, D1, D1, DQ, D1, D1, ST, D1, D1, D1, D1, D1, DB, D1, D1, D1, D1, D1, D1, DS, D1 },     // D1 <!D
    { 0, DQ, DQ, D1, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ, DQ },     // DQ <!D"
    { 0, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, DS, D1, DS },     // DS <!D'
    { 0, DB, DB, DB, DB, E1, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, D1, DB, DB, DB },     // DB <!D[
    { 0, E1, E1, Q2, E1, E1, DB, E1, E1, E1, E1, E1, E1, E1, E1, E1, E1, E1, E1, Q1, E1 },     // E1 <!D[<
    { 0, Q2, Q2, E1, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2 },     // Q2 <!D[<"
    { 0, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, E1, Q1 },     // Q1 <!D[<'
    
/*   
// <!-- --> Comment
#define M1 (Q1+1)   // <!-
#define M2 (M1+1)   // <!--
#define M3 (M1+2)   // -
#define M4 (M1+3)   // --
*/
   
    // <!-- --> Comment
    //    a   0   "       <   >   /   =   O   ?   !   [   C   D   A   T   ]   -   '   U         
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, M2,  0,  0 },   // M1 <!-
    { 0, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M3, M2, M2 },   // M2 <!--
    { 0, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M2, M4, M2, M2 },   // M3 -
    { 0,  0,  0,  0,  0,  0, ST,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },   // M4 -- 
};

// :   . added to number group  3A 2E

/*  
#define C_QM        10  3F
#define C_EX_POINT  11
#define C_OPEN_SQ   12
#define C_C         13
#define C_D         14    
#define C_A         15
#define C_T         16
#define C_CLOSE_SQ  17
#define C_DASH      18
#define C_SINGLE_Q  19
#define U           20 // utf8
*/

#define Z       0
#define AL      1
#define N       2
#define QU      3  // 22 "
#define WS      4  
#define OP      5  // 3C <
#define CL      6  // 3E >
#define SL      7  // 2F /
#define EE      8  // 3D =
#define O       9  // Other
#define QM      10 // 3F ?
#define EP      11 // 21 !
#define OS      12 // 5B [
#define C       13 // 43 C
#define D       14 // 44 D  
#define A       15 // 41 A
#define T       16 // 54 T
#define CS      17 // 5D ]
#define HY      18 // 2D -
#define QQ      19 // 27 '
#define U       20 // UTF-8

int xchar_types[] = {
//  0  1  2  3     4  5  6  7     8  9  A  B     C  D  E  F 
    Z, O, O, O,    O, O, O, O,    O, 4, 4, O,    O, 4, O, O, // O
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O, // 1
    4,EP,QU, O,    O, O, O,QQ,    O, O, O, O,    O,HY, 2,SL, // 2 
    2, 2, 2, 2,    2, 2, 2, 2,    2, 2, 2, O,   OP,EE,CL,QM, // 3

    O, A, 1, C,    D, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 1, // 4
    1, 1, 1, 1,    T, 1, 1, 1,    1, 1, 1,OS,    8,CS, O, 1, // 5
    O, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 1, // 6
    1, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, O,    O, O, O, O, // 7

    U, U, U, U,    U, U, U, U,    U, U, U, U,    U, U, U, U, // 8
    U, U, U, U,    U, U, U, U,    U, U, U, U,    U, U, U, U, // 9
    U, U, U, U,    U, U, U, U,    U, U, U, U,    U, U, U, U, // A
    U, U, U, U,    U, U, U, U,    U, U, U, U,    U, U, U, U, // B

    U, U, U, U,    U, U, U, U,    U, U, U, U,    U, U, U, U, // C
    U, U, U, U,    U, U, U, U,    U, U, U, U,    U, U, U, U, // D
    U, U, U, U,    U, U, U, U,    U, U, U, U,    U, U, U, U, // E
    U, U, U, U,    U, U, U, U,    U, U, U, U,    U, U, U, U, // F
};

int read_xml_element(reader &is, xml_element &bl, std::string &body);
int read_xml_element(reader &is, std::unique_ptr<xml_element> &e, std::unique_ptr<xml_element> &parent)
{
    e->parent= parent.get();
    e->closing = false;
    e->tag = "";
    e->attributes.clear();
    int state = 1;
    int next_state = 0;
    int t = 0;
    int current = 0;
    std::string item;
    std::string prop_name;
    char c = 0;
    std::string line;

    if (is.eof())
        return 0;

    bool found_open_bracket = false;

    while ((c = is.next_char()))
    {
        t = xchar_types[(unsigned char)c];
        next_state = states[state][t];

        if (next_state != state)
        {
            switch (state)
            {                                        
                case D1:
                    e->dtd = true;
                    break;
                    
                case M4:
                    e->comment = true;
                    break;
                
                case C6:
                line = "";
                break;

            case ST:
                if(next_state == OB)
                {
                    found_open_bracket = true;
                }
                break;

                case TX:
                    e->text = unescape_body(line);
                e->tag = "text";
                e->closing = true;
                    is.putback(c);
                break;
                
            case NM:
                //               cout << "name: " << line.substr(start, current - start) << "\n";
                e->tag = line;
                break;

            case EN:
                //                cout << "end name: " << line.substr(start, current - start) << "\n";
                e->tag = line;
                break;

            case PN:
                //                cout << "prop name: " << line.substr(start, current - start) << "\n";
                prop_name = line;
                break;

            case ED:
                //                cout << "prop name: " << line.substr(start, current - start) << "\n";
                e->closing = true; // = line.substr(start, current - start);
                break;

            case VQ:
            case VS:
                //                cout << "value: " << line.substr(start, current - start + 1) << "\n";
                line += c;
                    std::string ln = unescape_quote(line);
                    e->attributes[prop_name] = ln;
                break;
            }

            if(next_state == ST)
            {
                if (state == C9)
                {
                    e->cdata = true;
                    {
                        e->tag = "cdata";
                        e->text = line;
                        e->closing = true;

                        found_open_bracket = true;
                    }
                }
 
                break;
            }            

            if(e->text.size() && !e->cdata)
            {
                break;
            }
            
            if(next_state != C7 && next_state != C8 && next_state != C9)
            {            
                line = "";
            }
        }
        else
        {
            if (state == OB && c == '?')
                e->prolog = true;
        }
        
        line += c;
        state = next_state;
        ++current;
    }

    return (found_open_bracket || e->text.size()) ? 1 : 0;
}

int read_bracket(reader &is, std::string &line)
{
    if (is.eof())
    {
        line = "";
        return 0;
    }

    std::string result;

    while (!is.eof())
    {
        char c;
        c = is.next_char();
        if (c != '<')
            continue;
        result += c;
        break;
    }

    while (!is.eof())
    {
        char c;
        c = is.next_char();
        result += c;
        if (c == '>')
            break;
    }

    line = result;
    return (int)result.size();
}

void element_to_xml_(std::ostream &os, std::unique_ptr<xml_element> &bl, int margin = 0);
void to_xml_(std::ostream &os, xml_element &bl, int margin = 0);
void to_xml_close(std::ostream &os, std::unique_ptr<xml_element> &bl, int margin = 0);

void to_xml_(std::ostream &os, std::unique_ptr<xml_element> &e, int margin = 0)
{
    element_to_xml_(os, e, margin);

    for (auto &c : e->children_)
    {
        to_xml_(os, c, margin + 3);
    }

    if (e->text.size() || e->children_.size())
    {
        to_xml_close(os, e, margin);
    }
}

void save_to_xml(std::ostream &os, std::unique_ptr<xml_element> &e)
{
    unsigned char bom[3] = {0xEF,0xBB,0xBF};
    os.write((char*)&bom[0],3);
    for (auto &c : e->children_)
    {
        to_xml_(os, c);
    }
}

std::ostream &operator<<(std::ostream &os, xml_element &bl);

void element_to_xml_(std::ostream &os, std::unique_ptr<xml_element> &e, int margin /* = 0 */)
{
        std::string h;
        if (e->prolog)
            h = "?";
        std::string m = std::string(margin, ' ');

    if(e->cdata)
    {
        std::string m2 = std::string(margin + 3, ' ');
        os << "<![CDATA" << e->text << ">";
        
    }
    else if (!e->text.size())
    {
        std::string m2 = std::string(margin + 3, ' ');
        os << "<" << h << e->tag;

        if (e->prolog)
        {
            for (auto it = e->attributes.rbegin(); it != e->attributes.rend(); ++it)
            {
                os << " ";
                os << it->first << "=" << escape_and_quote(it->second);
            }
        }
        else
        {
            for (auto &p : e->attributes)
            {
                os << " ";
                os << p.first << "=" << escape_and_quote(p.second);
            }
        }

        os << h;
        if(!e->text.size() && !e->children_.size() && !e->prolog)
            os << "/>";
        else
            os << ">";
    }
    else
    {
        if (!e->cdata)
        {
            if(e->children_.size())
            {
              //  os << "\n";
                os << m << escape_body(e->text);
            }
            else
                os << escape_body(e->text);
        }
        else
        {
            if(e->children_.size())
            {
               // os << "\n";
                os << m << e->text;
            }
            else
                os << e->text;
        }
    }
}

void to_xml_close(std::ostream &os, std::unique_ptr<xml_element> &e, int margin /* = 0 */)
{
    if (!e->text.size())
    {
        std::string m = std::string(margin, ' ');
        if(e->children_.size())
        {
            os <<"</" << e->tag << ">";
        }
        else
        {
            os << "</" << e->tag << ">";
        }
    }
}

void load_from_xml(file_reader &f, std::unique_ptr<xml_element> &parent)
{
    std::unique_ptr<xml_element> e = std::make_unique<xml_element>();
    while (read_xml_element(f, e, parent))
    {
        if(e->tag == "code")
        {
        }
        if (e->closing && parent->tag == e->tag)
        {
            break;
        }
        
        if(e->comment || e->dtd)
        {
            e->clear();
        }
        else
        {
            if (!e->closing && !e->prolog)
            {
                load_from_xml(f, e);
            }

            parent->children_.push_back(std::move(e));
            e = std::make_unique<xml_element>();
        }
    }
}

void print_node(std::unique_ptr<xml_element> &e, int margin = 0)
{
    if (e->tag == "lang")
    {
        std::cout << "";
    }
    e->to_stream(std::cout, margin);
    std::cout.flush();
    std::cout << "\n";
    for (auto &c : e->children_)
    {
        print_node(c, margin + 3);
        std::cout.flush();
    }
}

class xml
{
public:
    xml(const std::string& fname = "")
        : filename(fname) 
    {}

    std::unique_ptr<xml_element> top;
    std::string filename;   
    int error;
};

#undef Z       
#undef AL      
#undef N       
#undef QU      
#undef WS        
#undef OP      
#undef CL      
#undef SL      
#undef EE      
#undef O       
#undef QM      
#undef EP      
#undef OS      
#undef C       
#undef D         
#undef A       
#undef T       
#undef CS      
#undef HY      
#undef QQ      
#undef U       
#undef S_MATCHING                
#undef S_SEARCHING_FOR_WORD_START
#undef S_SEARCHING_FOR_WORD_END  
#undef S_NAME     
#undef S_PROP_NAME
#undef S_PROP_EQ  
#undef S_PROP_VAL 
#undef NL
#undef ST
#undef OB
#undef NM
#undef SP
#undef PN
#undef SQ
#undef EQ
#undef VQ
#undef VS
#undef ED
#undef EN
#undef SC
#undef EX
#undef C1
#undef C2
#undef C3
#undef C4
#undef C5
#undef C6
#undef C7
#undef C8
#undef C9
#undef D1
#undef DQ
#undef DS
#undef DB
#undef E1
#undef Q2
#undef Q1
#undef M1
#undef M2
#undef M3
#undef M4
#endif // XML_H_


