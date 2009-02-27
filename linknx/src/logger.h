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

void initLogging(ticpp::Element* pConfig=NULL);

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

class Logger
{
public:
    static Logger& getInstance(const char* cat);
    Logger(const char* cat);
    ErrStream errorStream();
    WarnStream warnStream();
    LogStream infoStream();
    DbgStream debugStream();
private:
    std::string cat_m;
    typedef std::pair<std::string ,Logger*> LoggerPair_t;
    typedef std::map<std::string ,Logger*> LoggerMap_t;
    static LoggerMap_t loggerMap_m;
};
#endif

ErrStream errorStream(const char* cat);
WarnStream warnStream(const char* cat);
LogStream infoStream(const char* cat);
DbgStream debugStream(const char* cat);


#endif /*LOGGER_H_*/
