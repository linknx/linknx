/*
    LinKNX KNX home automation platform
    Copyright (C) 2007-2008 Jean-François Meessen <linknx@ouaye.net>
 
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

#include "logger.h"

#ifdef HAVE_LOG4CPP

#include    <log4cpp/BasicConfigurator.hh>
#include    <log4cpp/PropertyConfigurator.hh>

void initLogging() {
    try{ 
        log4cpp::PropertyConfigurator::configure("logging.conf"); // throw (ConfigureFailure)
    }
    catch (log4cpp::ConfigureFailure ex) {
        log4cpp::BasicConfigurator::configure();
        errorStream("Logger") << "Unable to load logging.conf : " << ex.what() << endlog;        
    }
}

#else

Logger::LoggerMap_t Logger::loggerMap_m;

void initLogging() {
}

Logger& Logger::getInstance(const char* cat) {
    LoggerMap_t::iterator it = loggerMap_m.find(cat);
    Logger* logger;
    if (it != loggerMap_m.end())
        logger = it->second;
    else {
        logger = new Logger(cat);
        loggerMap_m.insert(LoggerPair_t(cat, logger));
    }
    return *(logger);
}

DummyStream DummyStream::dummy;

Logger::Logger(const char* cat) : cat_m(cat) {
}

ErrStream Logger::errorStream() {
#ifdef LOG_SHOW_ERROR
    return std::cerr << "[ERROR] " << cat_m << ": ";
#else
    return DummyStream::dummy;
#endif
}

WarnStream Logger::warnStream()  {
#ifdef LOG_SHOW_WARN
    return std::cerr << "[WARN ] " << cat_m << ": ";
#else
    return DummyStream::dummy;
#endif
}

LogStream Logger::infoStream()  {
#ifdef LOG_SHOW_INFO
    return std::cout << "[INFO ] " << cat_m << ": ";
#else
    return DummyStream::dummy;
#endif
}

DbgStream Logger::debugStream() {
#ifdef LOG_SHOW_DEBUG
    return std::cout << "[DEBUG] " << cat_m << ": ";
#else
    return DummyStream::dummy;
#endif
}

#endif

ErrStream errorStream(const char* cat) { return Logger::getInstance(cat).errorStream(); };
WarnStream warnStream(const char* cat) { return Logger::getInstance(cat).warnStream(); };
LogStream infoStream(const char* cat) { return Logger::getInstance(cat).infoStream(); };
DbgStream debugStream(const char* cat) { return Logger::getInstance(cat).debugStream(); };
