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

#include "logger.hpp"

#include <cstdarg>
#include <cfloat>
#include <stdio.h>
#include <climits>
#include <cstring>
#include <iostream>
#include <stdint.h>
#include <fstream>
#include <map>
#include "format.hpp"
#include "core/time.h"

#ifdef JULIUS_PLATFORM_ANDROID
#include <android/log.h>
#include <SDL_system.h>
#endif

struct
{
    StringArray filters;
    std::map<std::string,LogWriter*> writers;
} ilog;

const char* LogWriter::severity(LogWriter::Severity s)
{
    switch (s)
    {
    case LogWriter::debug:
        return "[DEBUG]";
    case LogWriter::info:
        return "[INFO]";
    case LogWriter::warn:
        return "[WARN]";
    case LogWriter::error:
        return "[ERROR]";
    case LogWriter::fatal:
        return "[FATAL]";
    }
    return "[UNKNOWN]";
}

class FileLogWriter : public LogWriter
{
private:
    FILE* _logFile = nullptr;
public:
    FileLogWriter(const std::string& path)
    {
        _logFile = fopen(path.c_str(), "w");

        Time t = Time::current();
        if(_logFile)
            fmt::fprintf(_logFile, "Julius logfile created: %02d:%02d:%02d\n", t.hour, t.minutes, t.seconds);
    }

    ~FileLogWriter()
    {
        if(!_logFile)
            return;

        Time t = Time::current();
        fmt::fprintf(_logFile, "Julius logfile closed: %02d:%02d:%02d ", t.hour, t.minutes, t.seconds);
        fclose(_logFile);
    }

    virtual bool isActive() const
    {
        return _logFile != 0;
    }

    virtual void write( const std::string& str, bool )
    {
        // Don't write progress stuff into the logfile
        // Make sure only one thread is writing to the file at a time
        static int count = 0;
        if( _logFile )
        {
            fputs(str.c_str(), _logFile);
            fputs("\n", _logFile);

            count++;
            if( count % 10 == 0 )
            {
                fflush(_logFile);
            }
        }
    }
};

class ConsoleLogWriter : public LogWriter
{
public:
    virtual void write( const std::string& str, bool newline )
    {
#ifdef JULIUS_PLATFORM_ANDROID
        __android_log_print(ANDROID_LOG_DEBUG, JULIUS_PLATFORM_NAME, "%s", str.c_str() );
        if( newline )
            __android_log_print(ANDROID_LOG_DEBUG, JULIUS_PLATFORM_NAME, "\n" );
#else
        std::cout << str;
        if( newline ) std::cout << std::endl;
        else std::cout << std::flush;
#endif
    }

    virtual bool isActive() const
    {
        return true;
    }
};


void Logger::_print(LogWriter::Severity s,std::string str )
{
    write(s,str);
}

void Logger::write(LogWriter::Severity s, std::string message, bool newline)
{
    write(LogWriter::severity(s) + message, newline);
}

void Logger::write(std::string message, bool newline)
{
    // Check for filter pass
    for (int i=0; i < ilog.filters.size(); i++)
    {
        auto filter = ilog.filters[i];
        if (message.compare(0, filter.size(), filter) == 0)
            return;
    }

    for( auto& item : ilog.writers )
    {
        if (item.second)
        {
            item.second->write(message, newline);
        }
    }
}

void Logger::warningIf(bool warn, const std::string& text)
{
    if (warn) warning( text );
}
void Logger::update(const std::string& text, bool newline)
{
    write( text, newline );
}

void Logger::addFilter(const std::string& text)
{
    if (hasFilter(text))
        return;

    ilog.filters.addIfValid(text);
}

void Logger::addFilter(LogWriter::Severity s)
{
    ilog.filters.addIfValid(LogWriter::severity(s));
}

bool Logger::hasFilter(const std::string& text)
{
    for( auto& filter : ilog.filters)
    {
        if (filter == text) return true;
    }
    return false;
}

void Logger::removeFilter(const std::string& text)
{
    ilog.filters.remove(text);
}

void Logger::registerWriter(Logger::Type type, std::string param )
{
    switch( type )
    {
    case consolelog:
    {
        static ConsoleLogWriter wr;
        registerWriter( "__console", wr );
    }
    break;

    case filelog:
    {
        std::string fullname = param + "/stdout.txt";
        static FileLogWriter wr(fullname);
        registerWriter( "__log", wr);
    }
    break;

    case count:
        break;
    }
}

void Logger::registerWriter(const std::string& name, LogWriter& writer)
{
    if(writer.isActive())
    {
        ilog.writers[ name ] = &writer;
    }
}
