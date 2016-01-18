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

#ifndef SERVICES_H
#define SERVICES_H

#include "config.h"
#include <string>
#include "ticpp.h"
#include "timermanager.h"
#include "xmlserver.h"
#include "smsgateway.h"
#include "emailgateway.h"
#include "knxconnection.h"
#include "persistentstorage.h"
#include "suncalc.h"


class Services
{
public:
    static Services* instance();
    static void reset()
    {
        if (instance_m)
            delete instance_m;
        instance_m = 0;
    };

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    void start();
    void stop();
    KnxConnection* getKnxConnection() { return &knxConnection_m; };
    SmsGateway* getSmsGateway() { return &smsGateway_m; };
    EmailGateway* getEmailGateway() { return &emailGateway_m; };
    TimerManager* getTimerManager() { return &timers_m; };
    ExceptionDays* getExceptionDays() { return &exceptionDays_m; };
    PersistentStorage* getPersistentStorage() { return persistentStorage_m; };
    LocationInfo* getLocationInfo() { return &locationInfo_m; };
    void setConfigFile(const char* filename) { if (filename) configFile_m = filename; };
    std::string getConfigFile() { return configFile_m; };
    void createDefault();

private:
    Services();
    ~Services();

    static Services* instance_m;

    XmlServer *xmlServer_m;
    PersistentStorage *persistentStorage_m;
    TimerManager timers_m;
    SmsGateway smsGateway_m;
    EmailGateway emailGateway_m;
    KnxConnection knxConnection_m;
    ExceptionDays exceptionDays_m;
    LocationInfo locationInfo_m;
    
    std::string configFile_m;
};

#endif
