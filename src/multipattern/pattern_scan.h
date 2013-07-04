#ifndef UTILITY_PATTERNSCAN_H
#define UTILITY_PATTERNSCAN_H
#include <vector>
#include <string>
#include <map>
class PatternScan
{
public:
    PatternScan(uint8_t prefixSize = 3);
    ~PatternScan(){}
    void Load(const std::string& filename);
    void Init(const std::string& patternFileName);
    void Init(const std::vector<std::string>& patternList);
    /**
     * @brief  constructor, it may throw exceptions
     * @param  fileName the file name of pattern list
     * @param  prefixSize the prefix size, counted in UNICODE chars, not bytes.
     *         the default value is 3, the allowed values are 1, 2, 3 or 4.
     */
    PatternScan(const char* fileName, uint8_t prefixSize = 3);
    /**
     * @brief  scan a text to find all the matched pattern within it
     * @param  text a text to be scanned
     * @param  matched a map to save all the matched pattern
     * @param  threshold if specified a value, then the scan will stop immediately when the size of
     *         matched words exceeds this threshold.
     * @return the added terms
     */
    int Scan(const std::string& text, std::map<std::string, uint32_t>& matches,
             uint32_t threshold = 0xFFFFFFFF) const;
private:

    /**
     * @brief  get the unicode codepoint of the next UTF-8 character.  If there are invalid UTF-9
     *         characters detected, 0 will be returned.  Specially, if a unicode char with codepoint
     *         larger than 0xFFFF detected, then 0xFFFF is return instead.
     * @param  s the string to be processed.
     * @param  start the start index of the next char.  It will be updated to the start point
     *         of the next char duing this process.  Even if some error detected, the start will
     *         still be increased by at least 1.  So the loop can be promised end.
     * @return the codepoint of the unicode char.  uint16_t is used because we only interest in
     *         codepoints no larger than 0xFFFF.
     */
    static inline uint16_t GetNextUtf8Char(const std::string& s, size_t& start);

    static inline bool IsAscii(const std::string& s);
    static bool EnglishWordNotMatch(const std::string& text, const std::string& astr, size_t start);

    static const uint8_t sCharTable[256];
    static const uint8_t sCharMask[7];
    static const uint64_t sPatternMask[4];

    uint8_t mPrefixSize;
    std::vector<std::string> mPatternList; // The list of all the pattern words
    std::vector<std::string> mPatternSurfixList; // The list of pattern surfixes for check use
    std::vector<std::map<uint64_t, size_t> > mSmallPatternMaps; // The map for short patterns
    std::map<uint64_t, std::pair<size_t, size_t> > mPrefixMap; // The prefix map

    PatternScan(const PatternScan &);
    PatternScan& operator=(const PatternScan &);

};
#endif
