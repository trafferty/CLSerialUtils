#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <iostream>
#include <string>  // string
#include <iomanip> // std::setw, put_time

#include <ctime>   // localtime
#include <chrono>  // chrono::system_clock
#include <sstream> // stringstream

class Logger
{
public:
    Logger(std::string name, bool debug, bool AnsiColor) :m_Name(name), m_Debug(debug), m_AnsiColor(AnsiColor) {};
    Logger(std::string name, bool debug) :m_Name(name), m_Debug(debug), m_AnsiColor(true) {};
    Logger(std::string name) :m_Name(name), m_Debug(false), m_AnsiColor(true) {};
    ~Logger(){};

    template <typename T> void LogError(const T& t)
    {
        //std::cout << TS_color << getTimeStamp() << Module_color << "(" << std::setw(16) << std::left << m_Name << ")" << LogLevel_color << " [ERROR] " << error_color << t << addPostModifiers() << std::endl;
        std::cout << addPreModifiers(Error) << t << addPostModifiers() << std::endl;
    }

    template <typename T> void LogWarn(const T& t)
    {
        std::cout << addPreModifiers(Warn) << t << addPostModifiers() << std::endl;
    }

    template <typename T> void LogInfo(const T& t)
    {
        std::cout << addPreModifiers(Info) << t << addPostModifiers() << std::endl;
    }

    template <typename T> void LogDebug(const T& t)
    {
        if (m_Debug)
        {
            std::cout << addPreModifiers(Debug) << t << addPostModifiers() << std::endl;
        }
    }

    template <typename First, typename... Rest> void LogError(const First& first, const Rest&... rest)
    {
        std::cout << addPreModifiers(Error) << first;
        Finish(rest...); // recursive call using pack expansion syntax
    }

    template <typename First, typename... Rest> void LogWarn(const First& first, const Rest&... rest)
    {
        std::cout << addPreModifiers(Warn) << first;
        Finish(rest...); // recursive call using pack expansion syntax
    }

    template <typename First, typename... Rest> void LogInfo(const First& first, const Rest&... rest)
    {
        std::cout << addPreModifiers(Info) << first;
        Finish(rest...); // recursive call using pack expansion syntax
    }

    template <typename First, typename... Rest> void LogDebug(const First& first, const Rest&... rest)
    {
        if (m_Debug)
        {
            std::cout << addPreModifiers(Debug) << first;
            Finish(rest...); // recursive call using pack expansion syntax
        }
    }

private:
    enum ErrorLevel {
        Debug,
        Info,
        Warn,
        Error
    };

    std::string m_Name;
    bool m_Debug = false;
    bool m_AnsiColor = false;

    const std::string error_color   ="\033[0;31m";  // red
    const std::string warn_color    ="\033[1;33m";  // yellow
    const std::string debug_color   ="\033[0;90m";  // dark grey
    const std::string info_color    ="\033[0;97m";  // white
    const std::string TS_color      ="\033[0;37m";  // light grey
    const std::string Module_color  ="\033[0;34m";  // blue
    const std::string LogLevel_color="\033[0;35m";  // magenta
    const std::string reset         ="\033[0m";

    template <typename T> void Finish(const T& t)
    {
        std::cout << t << std::endl;
    }

    template <typename First, typename... Rest> void Finish(const First& first, const Rest&... rest)
    {
        std::cout << first;
        Finish(rest...); // recursive call using pack expansion syntax
    }

    std::string addPreModifiers(ErrorLevel errorLevel)
    {
        std::stringstream ss;
        std::string errorLevelStr;
        std::string levelCodeStr;

        if (m_AnsiColor)
        {
            switch (errorLevel)
            {
                case Debug:
                    errorLevelStr = "DEBUG";
                    levelCodeStr = debug_color;
                    break;
                case Info:
                    errorLevelStr = "INFO ";
                    levelCodeStr = info_color;
                    break;
                case Warn:
                    errorLevelStr = "WARN ";
                    levelCodeStr = warn_color;
                    break;
                case Error:
                    errorLevelStr = "ERROR";
                    levelCodeStr = error_color;
                    break;
            };

            ss << TS_color << getTimeStamp() << Module_color;
            ss << " (" << std::setw(6) << std::left << m_Name << ")";
            ss << LogLevel_color << " [" << errorLevelStr << "] " << levelCodeStr;
        }
        else
        {
            ss << getTimeStamp() << "(" << std::setw(6) << std::left << m_Name << ")";
        }
        return ss.str();
    }

    std::string addPostModifiers()
    {
        std::stringstream ss;
        if (m_AnsiColor)
            ss << reset;
        else
            ss.clear();
        return ss.str();
    }

    std::string getTimeStamp()
    {
        std::stringstream ss;

        auto p = std::chrono::high_resolution_clock::now();
        //auto now_c = std::chrono::high_resolution_clock::to_time_t(p);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch());
        auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);
        std::time_t t = s.count();
        std::size_t fractional_seconds = ms.count() % 1000;

        //ss << std::put_time(std::localtime(&now_c), "%c"); ///gotta wait for c++14 support in gcc for this one...sigh
        char buf[20];
        std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&t));
        ss << buf;
        ss << ".";
        ss << std::right << std::setw(3) << std::setfill('0') << fractional_seconds;
        return ss.str();
    }
};

#endif
