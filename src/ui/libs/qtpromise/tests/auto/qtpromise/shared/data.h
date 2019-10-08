#ifndef QTPROMISE_TESTS_AUTO_SHARED_DATA_H
#define QTPROMISE_TESTS_AUTO_SHARED_DATA_H

// STL
#include <utility>

struct Logs {
    int ctor = 0;
    int copy = 0;
    int move = 0;
    int refs = 0;

    void reset() {
        ctor = 0;
        copy = 0;
        move = 0;
        refs = 0;
    }
};

struct Logger
{
    Logger() { logs().ctor++; logs().refs++; }
    Logger(const Logger&) { logs().copy++; logs().refs++; }
    Logger(Logger&&) { logs().move++; logs().refs++; }
    ~Logger() { logs().refs--; }

    Logger& operator=(const Logger&) { logs().copy++; return *this; }
    Logger& operator=(Logger&&) { logs().move++; return *this; }

public: // STATICS
    static Logs& logs() { static Logs logs; return logs; }
};

struct Data : public Logger
{
    Data(int v) : Logger(), m_value(v) {}
    int value() const { return m_value; }

    // MSVC 2013 doesn't support implicit generation of the move constructor and
    // operator, so we need to explicitly define these methods and thus the copy
    // constructor and operator also need to be explicitly defined (error C2280).
    // https://stackoverflow.com/a/26581337

    Data(const Data& other)
        : Logger(other)
        , m_value(other.m_value)
    { }

    Data(Data&& other) : Logger(std::forward<Data>(other))
    {
        std::swap(m_value, other.m_value);
    }

    Data& operator=(const Data& other)
    {
        Logger::operator=(other);
        m_value = other.m_value;
        return *this;
    }

    Data& operator=(Data&& other)
    {
        Logger::operator=(std::forward<Data>(other));
        std::swap(m_value, other.m_value);
        return *this;
    }

    bool operator==(const Data& other) const { return (m_value == other.m_value); }
    bool operator!=(const Data& other) const { return (m_value != other.m_value); }

private:
    int m_value;
};

#endif // QTPROMISE_TESTS_AUTO_SHARED_DATA_H
