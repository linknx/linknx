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

Logging* Logging::instance_m;

#ifdef HAVE_LOG4CPP

#include    <log4cpp/BasicConfigurator.hh>
#include    <log4cpp/PropertyConfigurator.hh>
#include    <log4cpp/OstreamAppender.hh>
#include    <log4cpp/FileAppender.hh>
#include    <log4cpp/RollingFileAppender.hh>
#include    <log4cpp/SimpleLayout.hh>
#include    <log4cpp/PatternLayout.hh>

void Logging::importXml(ticpp::Element* pConfig)
{
    if (!pConfig) {
        log4cpp::BasicConfigurator::configure();
    }
    else {
        try{ 
            conffile_m = pConfig->GetAttribute("config");
            if (conffile_m.length()) {
                log4cpp::PropertyConfigurator::configure(conffile_m); // throw (ConfigureFailure)
            }
            else {
                output_m = pConfig->GetAttribute("output");
                format_m = pConfig->GetAttribute("format");
                level_m  = pConfig->GetAttribute("level");
                log4cpp::Appender* app = NULL;
                if (output_m.length()) {
                    pConfig->GetAttributeOrDefault("maxfilesize", &maxSize_m, -1);
                    if (maxSize_m > 0) {
                        pConfig->GetAttributeOrDefault("maxfileindex", &maxIndex_m, 10);
                        app = new log4cpp::RollingFileAppender("RollingFileAppender", output_m, maxSize_m * 1024, maxIndex_m);
                    }
                    else
                        app = new log4cpp::FileAppender("FileAppender", output_m);
                }
                else
                    app = new log4cpp::OstreamAppender("ConsoleAppender", &std::cout);
                if (format_m == "basic")
                    app->setLayout(new log4cpp::BasicLayout());
                else if (format_m == "simple")
                    app->setLayout(new log4cpp::SimpleLayout());
                else {
                    log4cpp::PatternLayout* patternLayout = new log4cpp::PatternLayout();
                    if (format_m == "")
                        patternLayout->setConversionPattern("%d [%5p] %c: %m%n");
                    else
                        patternLayout->setConversionPattern(format_m);
                    app->setLayout(patternLayout);
                }
                log4cpp::Priority::Value prio = log4cpp::Priority::INFO;
                if (level_m != "")
                    prio = log4cpp::Priority::getPriorityValue(level_m);
                    
                log4cpp::Category& root = log4cpp::Category::getRoot();
                root.setPriority(prio);
                
                root.removeAllAppenders();
                root.addAppender(app);
            }
        }
        catch (log4cpp::ConfigureFailure ex) {
            throw ticpp::Exception(ex.what());
        }
        catch (std::invalid_argument ex) {
            throw ticpp::Exception(ex.what());
        }
    }
}

void Logging::exportXml(ticpp::Element* pConfig)
{
    if (conffile_m != "")
    {
        pConfig->SetAttribute("config", conffile_m);
    }
    else
    {
        if (output_m != "")
        {
            pConfig->SetAttribute("output", output_m);
            if (maxSize_m != -1)
            {
                pConfig->SetAttribute("maxfilesize", maxSize_m);
                pConfig->SetAttribute("maxfileindex", maxIndex_m);
            }
        }
        if (format_m != "")
            pConfig->SetAttribute("format", format_m);
        if (level_m != "")
            pConfig->SetAttribute("level", level_m);
    }
}

#else

#include    <ctime>

int Logger::level_m;
bool Logger::timestamp_m;
NullStreamBuf Logger::nullStreamBuf_m;
std::ostream Logger::nullStream_m(&Logger::nullStreamBuf_m);

void Logging::importXml(ticpp::Element* pConfig)
{
    if (!pConfig) {
        Logger::level_m = 20;
        Logger::timestamp_m = true;
    }
    else {
//        std::string output = pConfig->GetAttribute("output");
        format_m = pConfig->GetAttribute("format");
        level_m  = pConfig->GetAttribute("level");
        if (level_m == "" || level_m == "INFO")
            Logger::level_m = 20;
        else if (level_m == "NOTICE")
            Logger::level_m = 30;
        else if (level_m == "WARN")
            Logger::level_m = 40;
        else if (level_m == "ERROR")
            Logger::level_m = 50;
        else if (level_m == "DEBUG")
            Logger::level_m = 10;

        Logger::timestamp_m = (format_m != "simple");
    }
}

void Logging::exportXml(ticpp::Element* pConfig)
{
    if (format_m != "")
        pConfig->SetAttribute("format", format_m);
    if (level_m != "")
        pConfig->SetAttribute("level", level_m);
}

Logger::LoggerMap_t* Logger::getLoggerMap() {
    // This static local is used to avoid problems with
    // initialization order of static object present in
    // different compilation units
    static LoggerMap_t* loggerMap = new LoggerMap_t();
    return loggerMap;
}

Logger& Logger::getInstance(const char* cat) {
    LoggerMap_t* map = getLoggerMap();
    LoggerMap_t::iterator it = map->find(cat);
    Logger* logger;
    if (it != map->end())
        logger = it->second;
    else {
        logger = new Logger(cat);
        map->insert(LoggerPair_t(cat, logger));
    }
    return *(logger);
}

DummyStream DummyStream::dummy;

Logger::Logger(const char* cat) : cat_m(cat) {
}

std::ostream& Logger::addPrefix(std::ostream &s, const char* level) {
    if (timestamp_m) {
        time_t now;
        struct tm * timeinfo;
        char buffer [32];

        time ( &now );
        timeinfo = localtime ( &now );
        strftime (buffer,sizeof(buffer),"%Y-%m-%d %X ",timeinfo);
        s << buffer;
    }
    return s << level << cat_m << ": ";
}

ErrStream Logger::errorStream() {
#ifdef LOG_SHOW_ERROR
    if (level_m <= 50)
        return addPrefix(std::cerr, "[ERROR] ");
    else
        return nullStream_m;
#else
    return DummyStream::dummy;
#endif
}

WarnStream Logger::warnStream()  {
#ifdef LOG_SHOW_WARN
    if (level_m <= 40) 
        return addPrefix(std::cerr, "[ WARN] ");
    else
        return nullStream_m;
#else
    return DummyStream::dummy;
#endif
}

LogStream Logger::infoStream()  {
#ifdef LOG_SHOW_INFO
    if (level_m <= 20) 
        return addPrefix(std::cout, "[ INFO] ");
    else
        return nullStream_m;
#else
    return DummyStream::dummy;
#endif
}

DbgStream Logger::debugStream() {
#ifdef LOG_SHOW_DEBUG
    if (level_m <= 10) 
        return addPrefix(std::cout, "[DEBUG] ");
    else
        return nullStream_m;
#else
    return DummyStream::dummy;
#endif
}

bool Logger::isDebugEnabled() {
#ifdef LOG_SHOW_DEBUG
    return (level_m <= 10);
#else
    return false;
#endif
}

#endif

ErrStream errorStream(const char* cat) { return Logger::getInstance(cat).errorStream(); };
WarnStream warnStream(const char* cat) { return Logger::getInstance(cat).warnStream(); };
LogStream infoStream(const char* cat) { return Logger::getInstance(cat).infoStream(); };
DbgStream debugStream(const char* cat) { return Logger::getInstance(cat).debugStream(); };
bool isDebugEnabled(const char* cat) { return Logger::getInstance(cat).isDebugEnabled(); };
