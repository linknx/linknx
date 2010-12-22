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

#include "persistentstorage.h"
#include <iostream>
#include <fstream>
#include <ctime>

#include <sys/types.h>
#include <dirent.h>

PersistentStorage* PersistentStorage::create(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttribute("type");
    if (type == "file")
    {
        std::string path = pConfig->GetAttributeOrDefault("path", "/var/lib/linknx/persist");
        std::string logPath = pConfig->GetAttribute("logpath");
        return new FilePersistentStorage(path, logPath);
    }
#ifdef HAVE_MYSQL
    else if (type == "mysql")
    {
        return new MysqlPersistentStorage(pConfig);
    }
#endif // HAVE_MYSQL
    else if (type == "")
    {
        return 0;
    }
    else
    {
        std::stringstream msg;
        msg << "PersistentStorage: storage type not supported: '" << type << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

Logger& FilePersistentStorage::logger_m(Logger::getInstance("FilePersistentStorage"));

FilePersistentStorage::FilePersistentStorage(std::string &path, std::string &logPath) : path_m(path), logPath_m(logPath)
{
    int  len = path_m.size();
    if (len > 0 && path_m[len-1] != '/')
        path_m.push_back('/');
    if (!opendir(path_m.c_str()))
    {
        std::stringstream msg;
        msg << "FilePersistentStorage: error opening path: '" << path_m << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    len = logPath_m.size();
    if (len == 0)
        logPath_m = path_m;
    if (len > 0 && logPath_m[len-1] != '/')
        logPath_m.push_back('/');
    if (!opendir(logPath_m.c_str()))
    {
        std::stringstream msg;
        msg << "FilePersistentStorage: error opening logpath: '" << logPath_m << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

void FilePersistentStorage::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "file");
    pConfig->SetAttribute("path", path_m);
    if (logPath_m != path_m)
        pConfig->SetAttribute("logpath", logPath_m);
}

void FilePersistentStorage::write(const std::string& id, const std::string& value)
{
    logger_m.infoStream() << "Writing '" << value << "' for object '" << id << "'" << endlog;
    std::string filename = path_m+id;
    std::ofstream fp_out(filename.c_str(), std::ios::out);
    fp_out << value;
    fp_out.close(); 
}

std::string FilePersistentStorage::read(const std::string& id, const std::string& defval)
{
    std::string value;
    std::string filename = path_m+id;
    std::ifstream fp_in(filename.c_str(), std::ios::in);
    std::getline(fp_in, value, static_cast<char>(-1));
    if (fp_in.fail())
        value = defval;
    fp_in.close();
    logger_m.infoStream() << "Reading '" << value << "' for object '" << id << "'" << endlog;
    return value;
}

void FilePersistentStorage::writelog(const std::string& id, const std::string& value)
{
    logger_m.infoStream() << "Writing log'" << value << "' for object '" << id << "'" << endlog;
    std::string filename = logPath_m+id+".log";
    std::ofstream fp_out(filename.c_str(), std::ios::app);

    time_t tim = time(0);
    struct tm * timeinfo = localtime(&tim);

    fp_out << timeinfo->tm_year+1900 << "-" << timeinfo->tm_mon+1 << "-" << timeinfo->tm_mday << " ";
    fp_out << timeinfo->tm_hour      << ":" << timeinfo->tm_min   << ":" << timeinfo->tm_sec;
    fp_out << " > " << value << "\n";
    fp_out.close(); 
}


#ifdef HAVE_MYSQL
Logger& MysqlPersistentStorage::logger_m(Logger::getInstance("MysqlPersistentStorage"));

MysqlPersistentStorage::MysqlPersistentStorage(ticpp::Element* pConfig)
{
    my_bool reconnect = 1;
    host_m = pConfig->GetAttribute("host");
    user_m = pConfig->GetAttribute("user");
    pass_m = pConfig->GetAttribute("pass");
    db_m = pConfig->GetAttribute("db");
    table_m = pConfig->GetAttribute("table");
    logtable_m = pConfig->GetAttribute("logtable");

    if(mysql_init(&con_m)==NULL)
    {
        throw ticpp::Exception("MysqlPersistentStorage: error initializing client");
    }
    mysql_options(&con_m, MYSQL_OPT_RECONNECT, &reconnect);

    if (!mysql_real_connect(&con_m, host_m.c_str(), user_m.c_str(), pass_m.c_str(), db_m.c_str(), 0,NULL,0)) 
    {
        std::stringstream msg;
        msg << "MysqlPersistentStorage: error connecting to '" << db_m << "' on host '" << host_m << "' with user '" << user_m << "', error was '" << mysql_error(&con_m) << "'" <<std::endl;
        throw ticpp::Exception(msg.str());
    }
}

MysqlPersistentStorage::~MysqlPersistentStorage()
{
    mysql_close(&con_m);
}

void MysqlPersistentStorage::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "mysql");
    pConfig->SetAttribute("host", host_m);
    pConfig->SetAttribute("user", user_m);
    pConfig->SetAttribute("pass", pass_m);
    pConfig->SetAttribute("db", db_m);
    pConfig->SetAttribute("table", table_m);
    pConfig->SetAttribute("logtable", logtable_m);
}

void MysqlPersistentStorage::write(const std::string& id, const std::string& value)
{
    if (table_m == "")
        return;
    logger_m.infoStream() << "Writing '" << value << "' for object '" << id << "'" << endlog;

    std::stringstream sql;
    sql << "INSERT INTO `" << table_m << "` (`object`, `value`) VALUES ('" << id << "', '" << value << "') ON DUPLICATE KEY UPDATE `value` = '" << value << "';";

    if (mysql_real_query(&con_m, sql.str().c_str(), sql.str().length()) != 0)
    {
        logger_m.errorStream() << "Error executing: '" << sql.str() << "' mySQL said: '" << mysql_error(&con_m) << "'" << endlog;
    }
}

std::string MysqlPersistentStorage::read(const std::string& id, const std::string& defval)
{
    std::string value;
    
    if (table_m == "")
    {
        value = defval;
    }
    else
    {
        std::stringstream sql;
        sql << "SELECT `value` FROM `" << table_m << "` WHERE `object`='" << id << "';";

        if (mysql_real_query(&con_m, sql.str().c_str(), sql.str().length()) == 0)
        {
            MYSQL_RES *result;
            MYSQL_ROW row;
            
            result = mysql_store_result(&con_m);
            
            if (mysql_num_rows(result) > 0)
            {
                row = mysql_fetch_row(result);
                value = row[0];
            }
            else
            {
                value = defval;
            }

            mysql_free_result(result);
        }
        else
        {
            value = defval;
            
            logger_m.errorStream() << "Error executing: '" << sql.str() << "' mySQL said: '" << mysql_error(&con_m) << "'" << endlog;
        }
    }

    logger_m.infoStream() << "Reading '" << value << "' for object '" << id << "'" << endlog;
    return value;
}

void MysqlPersistentStorage::writelog(const std::string& id, const std::string& value)
{
    if (logtable_m == "")
        return;
    logger_m.infoStream() << "Writing log '" << value << "' for object '" << id << "'" << endlog;

    std::stringstream sql;
    sql << "INSERT INTO `" << logtable_m << "` (ts, object, value) VALUES (NOW(), '" << id << "', '" << value << "');";

    if (mysql_real_query(&con_m, sql.str().c_str(), sql.str().length()) != 0)
    {
        logger_m.errorStream() << "Error executing: '" << sql.str() << "' mySQL said: '" << mysql_error(&con_m) << "'" << endlog;
    }
}
#endif // HAVE_MYSQL
