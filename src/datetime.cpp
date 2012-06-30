#include "datetime.h"
#include <cassert>
#include <sstream>
namespace mlplus
{
DateTime DateTime::parseString(const std::string& timeExp)
{
    size_t loc = 0;
    size_t ix = 0;
    uint64_t year, month, day, hour, minute, second, millisecond;
    ix = timeExp.find("-");
    if(ix == std::string::npos)
    {
        parseValue(timeExp, loc, year, timeExp.size());
        return DateTime(year, 1, 1, 0, 0, 0, 0);
    }
    else
    {
        parseValue(timeExp, loc, year, ix);
    }
    loc++;

    ix = timeExp.find("-", loc);
    if(ix == std::string::npos)
    {
        parseValue(timeExp, loc, month, timeExp.size() - loc);
        return DateTime(year, month, 1, 0, 0, 0, 0);
    }
    else
    {
        parseValue(timeExp, loc, month, ix - loc);
    }
    loc++;

    ix = timeExp.find(" ", loc);
    if(ix == std::string::npos)
    {
        parseValue(timeExp, loc, day, timeExp.size() - loc);
        return DateTime(year, month, day, 0, 0, 0, 0);
    }
    else
    {
        parseValue(timeExp, loc, day, ix - loc);
    }
    loc++;

    ix = timeExp.find(":", loc);
    if(ix == std::string::npos)
    {
        parseValue(timeExp, loc, hour, timeExp.size() - loc);
        return DateTime(year, month, day, hour, 0, 0, 0);
    }
    else
    {
        parseValue(timeExp, loc, hour, ix - loc);
    }
    loc++;

    ix = timeExp.find(":", loc);
    if(ix == std::string::npos)
    {
        parseValue(timeExp, loc, minute, timeExp.size() - loc);
        return DateTime(year, month, day, hour, minute, 0, 0);
    }
    else
    {
        parseValue(timeExp, loc, minute, ix - loc);
    }
    loc++;

    ix = timeExp.find(".", loc);
    if(ix == std::string::npos)
    {
        parseValue(timeExp, loc, second, timeExp.size() - loc);
        return DateTime(year, month, day, hour, minute, second, 0);
    }
    else
    {
        parseValue(timeExp, loc, second, ix - loc);
    }
    loc++;

    ix = timeExp.size();
    parseValue(timeExp, loc, millisecond, ix - loc);

    return DateTime(year, month, day, hour, minute, second, millisecond);
}

void DateTime::parseValue(const std::string& timeExp, size_t& loc, uint64_t& value, size_t num)
{
    value = 0;
    for(size_t i = 0; i < num; i++, loc++)
    {
        if(timeExp[loc] >= '0' && timeExp[loc] <= '9')
        {
            value = value * 10 + (timeExp[loc] - '0');
        }
    }
}

DateTime::DateTime(uint64_t ticks)
{
    assert(ticks <= MAX_MILLI_SEC_TOTAL);
    mTicks = ticks;
    calculateFields();
}

DateTime::DateTime(uint64_t year, uint64_t month, uint64_t day)
{
    initialize(year, month, day, 0, 0, 0, 0);
}

DateTime::DateTime(uint64_t year, uint64_t month, uint64_t day, uint64_t hour, uint64_t minute, uint64_t second)
{
    initialize(year, month, day, hour, minute, second, 0);
}

DateTime::DateTime(uint64_t year, uint64_t month, uint64_t day, uint64_t hour, uint64_t minute, uint64_t second, uint64_t millisecond)
{
    initialize(year, month, day, hour, minute, second, millisecond);
}


std::string DateTime::print() const
{
    std::stringstream ss;
    alignPrint(ss, mYear, 4) << "-";
    alignPrint(ss, mMonth, 2) << "-";
    alignPrint(ss, mDay, 2) << " ";
    alignPrint(ss, mHour, 2) << ":";
    alignPrint(ss, mMinute, 2) << ":";
    alignPrint(ss, mSecond, 2) << ".";
    alignPrint(ss, mMilliSecond, 3);
    return ss.str();
}

DateTime DateTime::addYear(int64_t deltaYear)
{
    assert(!multiplyOverflow(deltaYear, 12));
    int64_t deltaMonth = deltaYear * 12;
    return addMonth(deltaMonth);
}

DateTime DateTime::addMonth(int64_t deltaMonth)
{
    int64_t days = 0;
    int64_t year = deltaMonth / 12;
    int64_t remainder = deltaMonth % 12;
    uint32_t sumOfDay[13] = {0,31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int64_t remaindYear = year % 4;
    if(remainder < 0)
    {
        remaindYear += 4;
        for(size_t i = (mMonth + 10) % 12 + 1, j = remainder; j != 0; j++, i = (i + 10) % 12 + 1)
        {
            days -= sumOfDay[i];
        }
        if((mYear + remaindYear) % 4 == 0 && mMonth > 2 && mMonth + remainder <= 2)
        {
            days--;
        }
    }
    else if(remainder > 0)
    {
        for(size_t i = mMonth, j = remainder; j != 0; j--, i = i % 12 + 1)
        {
            days += sumOfDay[i];
        }
        if((mYear + remaindYear) % 4 == 0 && mMonth <= 2 && mMonth + remainder > 2)
        {
            days++;
        }
    }
    assert(!multiplyOverflow(year, 365));
    int64_t totalDays = year * 365;
    assert(!addOverflow(days, totalDays));
    days += totalDays;
    assert(!addOverflow(days, year / 4));
    if(mMonth <= 2) days += (mYear + year - 1) / 4 - (mYear - 1) / 4;
    else days += (mYear + year) / 4 - mYear / 4;
    return addDay(days);
}

DateTime DateTime::addDay(int64_t deltaDay)
{
    assert(!multiplyOverflow(deltaDay, 24));
    int64_t deltaHour = deltaDay * 24;
    return addHour(deltaHour);
}

DateTime DateTime::addHour(int64_t deltaHour)
{
    assert(!multiplyOverflow(deltaHour, 60));
    int64_t deltaMinute = deltaHour * 60;
    return addMinute(deltaMinute);
}

DateTime DateTime::addMinute(int64_t deltaMinute)
{
    assert(!multiplyOverflow(deltaMinute, 60));
    int64_t deltaSecond = deltaMinute * 60;
    return addSecond(deltaSecond);
}

DateTime DateTime::addSecond(int64_t deltaSecond)
{
    assert(!multiplyOverflow(deltaSecond, 1000));
    int64_t deltaMilliSecond = deltaSecond * 1000;
    return addMilliSecond(deltaMilliSecond);
}

DateTime DateTime::addMilliSecond(int64_t deltaMilliSecond)
{
    return addTicks(deltaMilliSecond);
}

void DateTime::initialize(uint64_t year, uint64_t month, uint64_t day, uint64_t hour, uint64_t minute, uint64_t second, uint64_t millisecond)
{
    assert(!yearOverflow(year));
    assert(!monthOverflow(month));
    assert(!dayOverflow(year, month, day));
    assert(!hourOverflow(hour));
    assert(!minuteOverflow(minute));
    assert(!secondOverflow(second));
    assert(!milliSecondOverflow(millisecond));
    mTicks = calculateTicks(year, month, day, hour, minute, second, millisecond);
    mYear = static_cast<uint16_t>(year);
    mMonth = static_cast<uint8_t>(month);
    mDay = static_cast<uint8_t>(day);
    mHour = static_cast<uint8_t>(hour);
    mMinute = static_cast<uint8_t>(minute);
    mSecond = static_cast<uint8_t>(second);
    mMilliSecond = millisecond;
}


uint64_t DateTime::calculateTicks(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond) const
{
    uint64_t ticks = 0;
    ticks += (static_cast<uint64_t>(year) * 365 + static_cast<uint64_t>(year + 3) / 4) * MILLI_SEC_PER_DAY;
    uint32_t sumOfDay[14] = {0,0,31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(0 == year % 4) sumOfDay[3] = 29;
    for(size_t i = 3; i < 13; ++i) sumOfDay[i] += sumOfDay[i - 1];
    ticks += sumOfDay[month] * 24 * 60 * 60 * 1000;
    ticks += static_cast<uint64_t>(day - 1) * MILLI_SEC_PER_DAY;
    ticks += static_cast<uint64_t>(hour) * MILLI_SEC_PER_HOUR;
    ticks += static_cast<uint64_t>(minute) * MILLI_SEC_PER_MINUTE;
    ticks += static_cast<uint64_t>(second) * MILLI_SEC_PER_SECOND;
    ticks += static_cast<uint64_t>(millisecond);
    return ticks;
}

void DateTime::calculateFields()
{
    uint64_t ticks = mTicks;
    mYear = extractYear(ticks);
    mMonth = extractMonth(mYear, ticks);
    mDay = extractDay(ticks);
    mHour = extractHour(ticks);
    mMinute = extractMinute(ticks);
    mSecond = extractSecond(ticks);
    mMilliSecond = extractMilliSecond(ticks);
}

uint16_t DateTime::extractYear(uint64_t& remainder) const
{
    uint64_t year = 0;
    uint64_t ticks = (365ull * 4 + 1) * MILLI_SEC_PER_DAY;
    year += (remainder / ticks) * 4;
    remainder %= ticks;
    uint64_t divide = 366ull * MILLI_SEC_PER_DAY;
    if(0 != remainder / divide)
    {
        year++;
        remainder -= divide;
        divide = 365ull * MILLI_SEC_PER_DAY;
        year += remainder / divide;
        remainder %= divide;
    }
    return static_cast<uint16_t>(year);
}

uint8_t DateTime::extractMonth(uint16_t year, uint64_t& remainder) const
{
    uint64_t month = 0;
    uint64_t remaindDays = remainder / (24ull * 60 * 60 * 1000);
    uint32_t sumOfDay[13] = {0,31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(0 == year % 4) sumOfDay[2] = 29;
    for(size_t i = 2; i < 13; ++i) sumOfDay[i] += sumOfDay[i - 1];
    for(size_t i = 1; i < 13; ++i)
    {
        if(remaindDays < sumOfDay[i])
        {
            month = i;
            break;
        }
    }
    remainder -= sumOfDay[month - 1] * MILLI_SEC_PER_DAY;
    return static_cast<uint8_t>(month);
}

DateTime DateTime::addTicks(int64_t deltaTicks)
{
    assert(!addTicksOverflow(deltaTicks));
    DateTime newDateTime(*this);
    newDateTime.mTicks += deltaTicks;
    newDateTime.calculateFields();
    assert(!yearOverflow(newDateTime.mYear));
    assert(!monthOverflow(newDateTime.mMonth));
    assert(!dayOverflow(newDateTime.mYear, newDateTime.mMonth, newDateTime.mDay));
    assert(!hourOverflow(newDateTime.mHour));
    assert(!minuteOverflow(newDateTime.mMinute));
    assert(!secondOverflow(newDateTime.mSecond));
    assert(!milliSecondOverflow(newDateTime.mMilliSecond));
    *this = newDateTime;
    return newDateTime;
}

}

