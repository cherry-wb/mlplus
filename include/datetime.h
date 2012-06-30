#ifndef MLPLUS_DATA_TIME_H
#define MLPLUS_DATA_TIME_H
#include <string>
#include <iosfwd>
#include <iomanip>
namespace mlplus
{

class DateTime
{
public:
    static const uint64_t MAX_MILLI_SEC_TOTAL=(((((9999ull*365+2500)*24+23)*60+59)*60)+59)*1000+999;
    static const uint64_t MILLI_SEC_PER_DAY=24ull * 60 * 60 * 1000;
    static const uint64_t MILLI_SEC_PER_HOUR=60ull * 60 * 1000;
    static const uint64_t MILLI_SEC_PER_MINUTE=60ull * 1000;
    static const uint64_t MILLI_SEC_PER_SECOND=1000ull;

    /*only support parseString("2012-1-2 12:12:59.100") */
    static DateTime parseString(const std::string& timeExp);
    static void parseValue(const std::string& timeExp, size_t& loc, uint64_t& value, size_t num);
public:
    DateTime(uint64_t ticks);
    DateTime(uint64_t year, uint64_t month, uint64_t day);
    DateTime(uint64_t year, uint64_t month, uint64_t day, uint64_t hour, uint64_t minute, uint64_t second);
    DateTime(uint64_t year, uint64_t month, uint64_t day,
             uint64_t hour, uint64_t minute, uint64_t second, uint64_t MILLIsecond);
    inline uint64_t getTicks() const;
    inline uint16_t getYear() const;
    inline uint8_t getMonth() const;
    inline uint8_t getDay() const;
    inline uint8_t getHour() const;
    inline uint8_t getMinute() const;
    inline uint8_t getSecond() const;
    inline uint16_t getMilliSecond() const;
    std::string print() const;
    DateTime addYear(int64_t deltaYear);
    DateTime addMonth(int64_t deltaMonth);
    DateTime addDay(int64_t deltaDay);
    DateTime addHour(int64_t deltaHour);
    DateTime addMinute(int64_t deltaMinute);
    DateTime addSecond(int64_t deltaSecond);
    DateTime addMilliSecond(int64_t deltaMILLISecond);
private:
    void initialize(uint64_t year, uint64_t month, uint64_t day,
                    uint64_t hour, uint64_t minute, uint64_t second, uint64_t MILLIsecond);
    inline bool yearOverflow(uint64_t year) const;
    inline bool monthOverflow(uint64_t month) const;
    inline bool dayOverflow(uint64_t year, uint64_t month, uint64_t day) const;
    inline bool hourOverflow(uint64_t hour) const;
    inline bool minuteOverflow(uint64_t minute) const;
    inline bool secondOverflow(uint64_t second) const;
    inline bool milliSecondOverflow(uint64_t millisecond) const;
    inline bool multiplyOverflow(int64_t m1, int64_t m2);
    inline bool addOverflow(int64_t a1, int64_t a2);
    inline bool addTicksOverflow(int64_t ticks);
    uint16_t extractYear(uint64_t& remainder) const;
    uint8_t extractMonth(uint16_t year, uint64_t& remainder) const;
    inline uint8_t extractDay(uint64_t& remainder) const;
    inline uint8_t extractHour(uint64_t& remainder) const;
    inline uint8_t extractMinute(uint64_t& remainder) const;
    inline uint8_t extractSecond(uint64_t& remainder) const;
    inline uint16_t extractMilliSecond(uint64_t& remainder) const;
    inline std::ostream& alignPrint(std::ostream& ss, uint64_t data, size_t bitNum) const
    {
        return ss << std::setw(bitNum) << std::setfill('0') << data;
    }
    DateTime addTicks(int64_t deltaTicks);
    uint64_t calculateTicks(uint16_t year, uint8_t month, uint8_t day,
                            uint8_t hour, uint8_t minute, uint8_t second, uint16_t MILLIsecond) const;
    void calculateFields();
private:
    uint64_t mTicks;
    uint16_t mYear;
    uint8_t mMonth;
    uint8_t mDay;
    uint8_t mHour;
    uint8_t mMinute;
    uint8_t mSecond;
    uint16_t mMilliSecond;
};
inline uint64_t DateTime::getTicks() const
{
    return mTicks;
}
inline uint16_t DateTime::getYear() const
{
    return mYear;
}
inline uint8_t DateTime::getMonth() const
{
    return mMonth;
}
inline uint8_t DateTime::getDay() const
{
    return mDay;
}
inline uint8_t DateTime::getHour() const
{
    return mHour;
}
inline uint8_t DateTime::getMinute() const
{
    return mMinute;
}
inline uint8_t DateTime::getSecond() const
{
    return mSecond;
}
inline uint16_t DateTime::getMilliSecond() const
{
    return mMilliSecond;
}
inline bool DateTime::yearOverflow(uint64_t year) const
{
    return year > 9999;
}
inline bool DateTime::monthOverflow(uint64_t month) const
{
    return month == 0 || month >= 13;
}
inline bool DateTime::dayOverflow(uint64_t year, uint64_t month, uint64_t day) const
{
    if(day == 0) return true;
    uint8_t daysPerMon[13] = {0,31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(0 == year % 4) daysPerMon[2] = 29;
    if(day >  daysPerMon[month]) return true;
    return false;
}
inline bool DateTime::hourOverflow(uint64_t hour) const
{
    return hour >= 24;
}
inline bool DateTime::minuteOverflow(uint64_t minute) const
{
    return minute >= 60;
}
inline bool DateTime::secondOverflow(uint64_t second) const
{
    return second >= 60;
}
inline bool DateTime::milliSecondOverflow(uint64_t millisecond) const
{
    return millisecond >= 1000;
}
inline uint8_t DateTime::extractDay(uint64_t& remainder) const
{
    uint64_t days = remainder / MILLI_SEC_PER_DAY;
    remainder %= MILLI_SEC_PER_DAY;
    return static_cast<uint8_t>(days + 1);
}
inline uint8_t DateTime::extractHour(uint64_t& remainder) const
{
    uint64_t hours = remainder / MILLI_SEC_PER_HOUR;
    remainder %= MILLI_SEC_PER_HOUR;
    return static_cast<uint8_t>(hours);
}
inline uint8_t DateTime::extractMinute(uint64_t& remainder) const
{
    uint64_t minutes = remainder / MILLI_SEC_PER_MINUTE;
    remainder %= MILLI_SEC_PER_MINUTE;
    return static_cast<uint8_t>(minutes);
}
inline uint8_t DateTime::extractSecond(uint64_t& remainder) const
{
    uint64_t seconds = remainder / MILLI_SEC_PER_SECOND;
    remainder %= MILLI_SEC_PER_SECOND;
    return static_cast<uint8_t>(seconds);
}
inline uint16_t DateTime::extractMilliSecond(uint64_t& remainder) const
{
    uint64_t milliseconds = remainder;
    remainder = 0;
    return static_cast<uint16_t>(milliseconds);
}

inline bool DateTime::multiplyOverflow(int64_t m1, int64_t m2)
{
    if(m2 == 0) return false;
    int64_t tmp1 = std::numeric_limits<int64_t>::min() / m2;
    int64_t tmp2 = MAX_MILLI_SEC_TOTAL / m2;
    return std::min(tmp1, tmp2) > m1 || m1 > std::max(tmp1, tmp2);
}
inline bool DateTime::addOverflow(int64_t a1, int64_t a2)
{
    if(a2 > 0)
    {
        int64_t tmp = MAX_MILLI_SEC_TOTAL - a2;
        return a1 > tmp;
    }
    if(a2 < 0)
    {
        int64_t tmp = std::numeric_limits<int64_t>::min() - a2;
        return a1 < tmp;
    }
    return false;
}
inline bool DateTime::addTicksOverflow(int64_t ticks)
{
    if(ticks < 0)
    {
        return (uint64_t)abs(ticks) > mTicks;
    }
    return static_cast<uint64_t>(ticks) > (MAX_MILLI_SEC_TOTAL - mTicks);
}
}
#endif
