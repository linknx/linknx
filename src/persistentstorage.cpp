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
#include "objectcontroller.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <typeinfo>

#include <sys/types.h>
#include <dirent.h>

PersistentStorage* PersistentStorage::create(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttribute("type");
    if (type == "file")
    {
        return new FilePersistentStorage(pConfig);
    }
#ifdef HAVE_MYSQL
    else if (type == "mysql")
    {
        return new MysqlPersistentStorage(pConfig);
    }
#endif // HAVE_MYSQL
#ifdef HAVE_INFLUXDB
    else if (type == "influxdb")
    {
        return new InfluxdbPersistentStorage(pConfig);
    }
#endif // HAVE_INFLUXDB
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

FilePersistentStorage::FilePersistentStorage(ticpp::Element* pConfig)
{
    path_m = pConfig->GetAttributeOrDefault("path", "/var/lib/linknx/persist");
    logPath_m = pConfig->GetAttribute("logPath");

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
    fp_out << std::setfill('0') << std::setw(2) 
    << timeinfo->tm_hour << ":" << std::setfill('0') << std::setw(2)
    << timeinfo->tm_min << ":" << std::setfill('0') << std::setw(2)
    << timeinfo->tm_sec;
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
    charset_m = pConfig->GetAttribute("charset");

    if(mysql_init(&con_m)==NULL)
    {
        throw ticpp::Exception("MysqlPersistentStorage: error initializing client");
    }
    mysql_options(&con_m, MYSQL_OPT_RECONNECT, &reconnect);

    if(!charset_m.empty())
    {
        mysql_options(&con_m, MYSQL_SET_CHARSET_NAME, charset_m.c_str());
    }

    if (!mysql_real_connect(&con_m, host_m.c_str(), user_m.c_str(), pass_m.c_str(), db_m.c_str(), 0,NULL,0)) 
    {
        std::stringstream msg;
        msg << "MysqlPersistentStorage: error connecting to '" << db_m << "' on host '" << host_m << "' with user '" << user_m << "', error was '" << mysql_error(&con_m) << "'" <<std::endl;
        throw ticpp::Exception(msg.str());
    }

    if(!charset_m.empty())
    {
        mysql_options(&con_m, MYSQL_SET_CHARSET_NAME, ("set names " + charset_m).c_str());
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
    if (charset_m != "")
        pConfig->SetAttribute("charset", charset_m);
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


#ifdef HAVE_INFLUXDB
Logger& InfluxdbPersistentStorage::logger_m(Logger::getInstance("InfluxdbPersistentStorage"));

InfluxdbPersistentStorage::InfluxdbPersistentStorage(ticpp::Element* pConfig)
{
    uri_m = pConfig->GetAttribute("uri");
    user_m = pConfig->GetAttribute("user");
    pass_m = pConfig->GetAttribute("pass");
    db_m = pConfig->GetAttribute("db");
    persist_db_m = pConfig->GetAttribute("persist_db");
}

InfluxdbPersistentStorage::~InfluxdbPersistentStorage()
{
}

static size_t _curl_write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

#undef INFLUXDB_EXTRA_DEBUG

bool InfluxdbPersistentStorage::curl_request(InfluxdbOperation_t oper, const std::string& db, const std::string& query, std::string& result)
{
    bool ret = false;
    CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if(curl)
	{
		std::stringstream url;
        url << uri_m << "/" << (oper == INFLUXDB_WRITE ? "write" : "query") << "?db=" << db;
        if (!user_m.empty() && !pass_m.empty())
        {
            url << "&u=" << user_m << "&p=" << pass_m;
        }
        const char *field_data = query.c_str();

#ifdef INFLUXDB_EXTRA_DEBUG
        logger_m.debugStream() << "requesting from " << url.str() <<  ": " << query << endlog;
#endif

		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, field_data);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "LinKNX");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _curl_write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

		res = curl_easy_perform(curl);
        std::cout << result << std::endl;

		if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code / 100 == 2)
                ret = true;
            else
                logger_m.warnStream() << "request to " << uri_m <<  " returned HTTP " << response_code << endlog;
        } else {
			logger_m.warnStream() << "request to " << uri_m <<  " failed: " << curl_easy_strerror(res) << endlog;
        }
#ifdef INFLUXDB_EXTRA_DEBUG
        logger_m.debugStream() << "response from db is: " << result << endlog;
#endif
		curl_easy_cleanup(curl);
	}
    return ret;
}

void InfluxdbPersistentStorage::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "influxdb");
    pConfig->SetAttribute("uri", uri_m);
    pConfig->SetAttribute("user", user_m);
    pConfig->SetAttribute("pass", pass_m);
    pConfig->SetAttribute("db", db_m);
    pConfig->SetAttribute("persist_db", persist_db_m);
}

void InfluxdbPersistentStorage::write(const std::string& id, const std::string& value)
{
    logger_m.infoStream() << "Persisting '" << value << "' for object '" << id << "'" << endlog;

    std::stringstream query_s(std::ios_base::out|std::ios_base::binary);;
    std::string resp;

    query_s << id << " " << "val=" << value << " 0";

    curl_request(INFLUXDB_WRITE, persist_db_m, query_s.str(), resp);
}

std::string InfluxdbPersistentStorage::read(const std::string& id, const std::string& defval)
{
    std::string value = defval, resp;
    std::stringstream query_s;
    query_s << "q=SELECT val FROM \"" << id << "\"";

    if (curl_request(INFLUXDB_QUERY, persist_db_m, query_s.str(), resp)) {
        Json::CharReaderBuilder builder;
        Json::CharReader * reader = builder.newCharReader();
        Json::Value root, val;
        std::string errors;

        if(!reader->parse(resp.c_str(), resp.c_str()+resp.size(), &root, &errors)) {
            logger_m.warnStream() << "couldn't parse JSON! Errors: " << errors << endlog;
        } else {
            val = root["results"][0]["series"][0]["values"][0][1];
            if (val != Json::nullValue) {
                value = val.asString();
                logger_m.debugStream() << "Read '" << value << "' for object '" << id << "' from InfluxDB response " << resp << endlog;
                return val.asString();
            } else {
                logger_m.warnStream() << "id '" << id << "' not found in persistence response " << resp << endlog;
            }
        }
    }
    
    return defval;
}

void InfluxdbPersistentStorage::writelog(const std::string& id, const std::string& value)
{
    logger_m.debugStream() << "Logging value '" << value << "' for object '" << id << "'" << endlog;

    std::string resp;
    std::stringstream query_s(std::ios_base::out|std::ios_base::binary);;

    query_s << id << " " << "val=" << value;

    curl_request(INFLUXDB_WRITE, db_m, query_s.str(), resp);
}
#endif // HAVE_INFLUXDB
