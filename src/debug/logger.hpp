// This file is part of Julius.
//
// Julius is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Julius is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Julius.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2017 Dalerank, dalerankn8@gmail.com

#ifndef __JULIUS_LOGGER_H_INCLUDED__
#define __JULIUS_LOGGER_H_INCLUDED__

#include <string>
#include "format.hpp"
#include "core/stringarray.hpp"

class LogWriter
{
public:
    enum Severity
    {
        debug=0,
        info,
        warn,
        error,
        fatal
    };

    LogWriter() {}
    static const char* severity(LogWriter::Severity s);
    virtual void write(const std::string&, bool newLine) {}
    virtual bool isActive() const
    {
        return false;
    }
};

class Logger
{
public:
    typedef enum {consolelog=0, filelog, count} Type;

#define DECL_LOGFUNC(severity,name) template<typename... Args> \
    static void name(const std::string& f, const Args & ... args) { _print(severity, fmt::format( f,args... )); } \
    static void name(const std::string& text) { _print( severity, text ); }

    DECL_LOGFUNC(LogWriter::info,  info)
    DECL_LOGFUNC(LogWriter::warn,  warning)
    DECL_LOGFUNC(LogWriter::debug, debug)
    DECL_LOGFUNC(LogWriter::error, error)
    DECL_LOGFUNC(LogWriter::fatal, fatal)
#undef DECL_LOGFUNC

    static void warningIf(bool warn, const std::string& text);
    static void update(const std::string& text, bool newline = false);

    static void addFilter(const std::string& text);
    static void addFilter(LogWriter::Severity s);
    static bool hasFilter(const std::string& text);
    static void removeFilter(const std::string& text);

    static void registerWriter(Type type, std::string param);
    static void registerWriter(const std::string& name, LogWriter& writer);

    ~Logger();
private:
    Logger();

    static void _print(LogWriter::Severity s, std::string text);

    static void write(LogWriter::Severity s, std::string message, bool newline = true);
    static void write(std::string message, bool newline=true);
};

#endif //__JULIUS_LOGGER_H_INCLUDED__
