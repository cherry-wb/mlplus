#include <boost/algorithm/string.hpp>
#include <fstream>
#include <vector>
#include "pattern_scan.h"
using namespace std;
using namespace boost;
const uint8_t PatternScan::sCharTable[256] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0
};
const uint8_t PatternScan::sCharMask[7] = {0, 0x7F, 0x1F, 0x0F, 0x07, 0x03, 0x01};
const uint64_t PatternScan::sPatternMask[4] = {0xFFFFull, 0xFFFFFFFFull, 0xFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull};

PatternScan::PatternScan(uint8_t prefixSize): mPrefixSize(prefixSize)
{
}

void PatternScan::Load(const std::string& filename)
{
    if(filename.empty()) return ;
    std::fstream ifs(filename.c_str());
    std::string line;
    std::vector<std::string> vecString;
    while(getline(ifs, line))
    {
        boost::trim(line);
        if(line.empty())
        {
            continue;
        }
        if(!boost::starts_with(line, "#"))
        {
            vector<string> keys;
            boost::split(keys, line, is_any_of("\t "));
            vecString.push_back(keys[0]);
        }
    }
    Init(vecString);
}
void PatternScan::Init(const vector<string>& patternList)
{
    mPatternList = patternList;
    mPatternList.push_back(""); // Put an empty string first so that the index of patterns are not 0.
    mPatternSurfixList.push_back("");
    sort(mPatternList.begin() , mPatternList.end());
    mPatternSurfixList.reserve(mPatternList.size());
    mSmallPatternMaps.resize(mPrefixSize - 1);
    for(size_t i = 1; i < mPatternList.size(); ++i)
    {
        const string& s = mPatternList[i];
        size_t start;
        int len;
        uint64_t pattern = 0;

        for(start = 0, len = 0; start < s.size() && len < mPrefixSize; ++len)
        {
            uint16_t unicode = GetNextUtf8Char(s, start);
            if(unicode == 0)
            {
                throw std::runtime_error("Invalid UTF-8 string detected in pattern file.");
            }
            pattern = (pattern << 16) | unicode;
        }
        if(len == 0)
        {
            mPatternSurfixList.push_back("");
        }
        else if(len < mPrefixSize)
        {
            mSmallPatternMaps[len - 1][pattern] = i;
            mPatternSurfixList.push_back("");
        }
        else
        {
            pair<size_t, size_t>& entry = mPrefixMap[pattern];
            if(entry.first == 0)
            {
                entry.first = i;
            }
            entry.second = i + 1;
            mPatternSurfixList.push_back(s.substr(start));
        }
    }
}
void PatternScan::Init(const string& fileName)
{
    ifstream ifs(fileName.c_str());
    if(!ifs)
    {
        throw std::runtime_error(string("Cannot open pattern list file: ") + fileName);
    }
    vector<string> patternList;
    string s;
    while(getline(ifs, s))
    {
        patternList.push_back(s);
    }
    Init(patternList);
}
PatternScan::PatternScan(const char* fileName, uint8_t prefixSize) : mPrefixSize(prefixSize)
{
    if(prefixSize < 1 || prefixSize > 4)
    {
        throw std::runtime_error("Invalid block size specified.");
    }
    Init(fileName);
}

inline bool PatternScan::IsAscii(const string& text)
{
    for(uint32_t i = 0; i < text.size(); ++i)
    {
        if((text[i] & 0x80) > 0) return false;
    }
    return true;
}

inline uint16_t PatternScan::GetNextUtf8Char(const std::string& s, size_t& start)
{
    if (start >= s.size())
    {
        return 0;
    }
    uint8_t ch = static_cast<uint8_t>(s[start]);
    uint8_t len = sCharTable[ch];
    if (len == 0)
    {
        ++start;
        return 0;
    }
    size_t end = start + len;
    if (end > s.size())
    {
        start = s.size();
        return 0;
    }
    uint32_t unicode = ch & sCharMask[len];
    for (++start; start < end; ++start)
    {
        ch = static_cast<uint8_t>(s[start]);
        if ((ch & 0xC0) ^ 0x80)
        {
            start = end;
            return 0;
        }
        unicode = (unicode << 6) | (ch & 0x3F);
    }
    if (unicode > 0xFFFF)
    {
        return 0xFFFF;
    }
    else
    {
        return unicode;
    }
}

bool PatternScan::EnglishWordNotMatch(const string& text, const string& astr, size_t start)
{
    bool isAscii = IsAscii(astr);
    int st = start - 1;
    bool startok = st >= 0 ? isspace(text[st]) : true;
    int ed = start + astr.size();
    bool endok = ed < (int)text.size() ? isspace(text[ed]) : true;
    return (isAscii && (!startok || !endok));
}

int PatternScan::Scan(const string& text, map<string, uint32_t>& matches, uint32_t threshold) const
{
    //matches.clear();
    int result = 0;
    if(mSmallPatternMaps.empty()) return 0;
    uint64_t pattern = 0;
    size_t start = 0;
    while(start < text.size())
    {
        pattern = ((pattern << 16) & sPatternMask[mPrefixSize - 1]) | GetNextUtf8Char(text, start);
        for(uint8_t len = 0; len < mPrefixSize - 1; ++len)
        {
            map<uint64_t, size_t>::const_iterator iter = mSmallPatternMaps[len].find(pattern & sPatternMask[len]);
            if(iter != mSmallPatternMaps[len].end())
            {
                int posSt = start - len;
                if(EnglishWordNotMatch(text, mPatternList[iter->second], posSt))
                {
                    continue;
                }
                ++matches[mPatternList[iter->second]];
                result++;
                if(matches.size() >= threshold)
                {
                    return result;
                }
            }
        }
        map<uint64_t, pair<size_t, size_t> >::const_iterator iter = mPrefixMap.find(pattern);
        if(iter != mPrefixMap.end())
        {
            for(size_t i = iter->second.first; i < iter->second.second; ++i)
            {

                if(text.compare(start, mPatternSurfixList[i].size(), mPatternSurfixList[i]) == 0)
                {
                    //for english
                    int posSt = start - mPrefixSize;
                    if(EnglishWordNotMatch(text, mPatternList[i], posSt))
                    {
                        continue;
                    }
                    ++matches[mPatternList[i]];
                    result++;
                    if(matches.size() >= threshold)
                    {
                        return result;
                    }
                }
            }
        }
    }
    return result;
}

