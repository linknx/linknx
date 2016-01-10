/*
    LinKNX KNX home automation platform
    Copyright (C) 2007 Jean-François Meessen <linknx@ouaye.net>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef LOGGER_H_
#define LOGGER_H_

#include "config.h"
#include "ticpp.h"

class Logging
{
public:
    static Logging* instance()
    {
        if (instance_m == 0)
            instance_m = new Logging();
        return instance_m;
    };        
    static void reset()
    {
        if (instance_m)
            delete instance_m;
        instance_m = 0;
    };

    void importXml(ticpp::Element* pConfig);
    void exportXml(ticpp::Element* pConfig);

    void defaultConfig() { importXml(NULL); };

private:
    Logging() : maxSize_m(-1), maxIndex_m(0) {};
    
    std::string conffile_m;
    std::string output_m;
    std::string format_m;
    std::string level_m;
    int maxSize_m;
    int maxIndex_m;
    static Logging* instance_m;
};

#ifdef HAVE_LOG4CPP
#include    <log4cpp/Category.hh>

#define Logger log4cpp::Category
#define ErrStream log4cpp::CategoryStream
#define WarnStream log4cpp::CategoryStream
#define LogStream log4cpp::CategoryStream
#define DbgStream log4cpp::CategoryStream
#define endlog log4cpp::eol

#else

#include <iostream>
#include <map>

#define LOG_SHOW_ERROR 1
#define LOG_SHOW_WARN 1
#define LOG_SHOW_INFO 1
#undef  LOG_SHOW_DEBUG


#ifdef LOG_SHOW_ERROR
#define ErrStream std::ostream&
#else
#define ErrStream DummyStream&
#endif

#ifdef LOG_SHOW_WARN
#define WarnStream std::ostream&
#else
#define WarnStream DummyStream&
#endif

#ifdef LOG_SHOW_INFO
#define LogStream std::ostream&
#else
#define LogStream DummyStream&
#endif

#ifdef LOG_SHOW_DEBUG
#define DbgStream std::ostream&
#else
#define DbgStream DummyStream&
#endif
#define endlog std::endl

class DummyStream
{
public:
    template<class T>
    DummyStream& operator<<(T val) { return *this; };
    DummyStream& operator<< (std::ostream& ( *pf )(std::ostream&)) { return *this; };
    static DummyStream dummy;
};

class NullStreamBuf : public std::streambuf
{
protected:
    int_type overflow (int_type c) { return c; }
    int sync () { return 0; }
};

class Logger
{
public:
    static Logger& getInstance(const char* cat);
    Logger(const char* cat);
    ErrStream errorStream();
    WarnStream warnStream();
    LogStream infoStream();
    DbgStream debugStream();
    bool isDebugEnabled();
    friend class Logging;
private:
    std::string cat_m;
    typedef std::pair<std::string ,Logger*> LoggerPair_t;
    typedef std::map<std::string ,Logger*> LoggerMap_t;
    static LoggerMap_t* getLoggerMap();
    static int level_m; // 10=DEBUG, 20=INFO, 30=NOTICE, 40=WARN, 50=ERROR, 
    static bool timestamp_m;
    static std::ostream nullStream_m;
    static NullStreamBuf nullStreamBuf_m;

    std::ostream& addPrefix(std::ostream &s, const char* level);
};
#endif

ErrStream errorStream(const char* cat);
WarnStream warnStream(const char* cat);
LogStream infoStream(const char* cat);
DbgStream debugStream(const char* cat);
bool isDebugEnabled(const char* cat);

class ErrorMessage : public std::stringstream
{
public:
    void logAndThrow(Logger &logger) {
        std::string s = str();
        logger.errorStream() << s << endlog;
        throw ticpp::Exception(s);
    };
};

#endif /*LOGGER_H_*/
