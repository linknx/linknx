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
    std::cout << "PersistentStorage: writing '" << value << "' for object '" << id << "'" << std::endl;
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
    std::cout << "PersistentStorage: reading '" << value << "' for object '" << id << "'" << std::endl;
    return value;
}

void FilePersistentStorage::writelog(const std::string& id, const std::string& value)
{
    std::cout << "PersistentStorage: writing log'" << value << "' for object '" << id << "'" << std::endl;
    std::string filename = logPath_m+id+".log";
    std::ofstream fp_out(filename.c_str(), std::ios::app);

    time_t tim = time(0);
    struct tm * timeinfo = localtime(&tim);

    fp_out << timeinfo->tm_year+1900 << "-" << timeinfo->tm_mon+1 << "-" << timeinfo->tm_mday << " ";
    fp_out << timeinfo->tm_hour      << ":" << timeinfo->tm_min   << ":" << timeinfo->tm_sec;
    fp_out << " > " << value << "\n";
    fp_out.close(); 
}
