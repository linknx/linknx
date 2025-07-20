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

#ifndef PERSISTENTSTORAGE_H
#define PERSISTENTSTORAGE_H

#include <string>
#include "config.h"
#include "logger.h"
#include "ticpp.h"
#include "objectcontroller.h"

#ifdef HAVE_MYSQL
#include <mysql/mysql.h>
#endif

#ifdef SUPPORT_INFLUXDB
#include <curl/curl.h>
#include <json/json.h>
#endif

class PersistentStorage
{
public:
    virtual ~PersistentStorage() {};

    static PersistentStorage* create(ticpp::Element* pConfig);

    virtual void exportXml(ticpp::Element* pConfig) = 0;

    virtual void write(const std::string& id, const std::string& value) = 0;
    virtual std::string read(const std::string& id, const std::string& defval="") = 0;
    virtual void writelog(const std::string &id, const ObjectValue &value) = 0;
};

class FilePersistentStorage : public PersistentStorage
{
public:
    FilePersistentStorage(ticpp::Element* pConfig);
    virtual ~FilePersistentStorage() {};

    virtual void exportXml(ticpp::Element* pConfig);

    virtual void write(const std::string& id, const std::string& value);
    virtual std::string read(const std::string& id, const std::string& defval="");
    virtual void writelog(const std::string& id, const ObjectValue &value);
private:
    std::string logPath_m;
    std::string path_m;
protected:
    static Logger& logger_m;
};

#ifdef HAVE_MYSQL
class MysqlPersistentStorage : public PersistentStorage
{
public:
    MysqlPersistentStorage(ticpp::Element* pConfig);
    virtual ~MysqlPersistentStorage();

    virtual void exportXml(ticpp::Element* pConfig);

    virtual void write(const std::string& id, const std::string& value);
    virtual std::string read(const std::string& id, const std::string& defval="");
    virtual void writelog(const std::string& id, const ObjectValue &value);
private:
    MYSQL con_m;
    std::string host_m;
    int port_m;
    std::string user_m;
    std::string pass_m;
    std::string logDb_m;
    std::string table_m;
    std::string logtable_m;
    std::string charset_m;

protected:
    static Logger& logger_m;
};
#endif // HAVE_MYSQL

#ifdef SUPPORT_INFLUXDB
enum InfluxdbOperation_t
{
    INFLUXDB_WRITE = 0,
    INFLUXDB_QUERY
};

class InfluxdbPersistentStorage : public PersistentStorage
{
public:
    InfluxdbPersistentStorage(ticpp::Element* pConfig);
    virtual ~InfluxdbPersistentStorage();
    virtual void exportXml(ticpp::Element* pConfig);
    virtual void write(const std::string& id, const std::string& value);
    virtual std::string read(const std::string& id, const std::string& defval="");
    virtual void writelog(const std::string& id, const ObjectValue &value);
private:
    bool curlRequest(InfluxdbOperation_t oper, const std::string& db, const std::string& query, std::string& result);
    bool createDB(const std::string &db, std::string &result);
    std::string uri_m;
    std::string user_m;
    std::string pass_m;
    std::string logDb_m;
    std::string persistenceDb_m;
protected:
    static Logger& logger_m;
};
#endif // SUPPORT_INFLUXDB
#endif
