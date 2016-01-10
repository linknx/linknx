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

#include "services.h"
#include "ioport.h"

Services* Services::instance_m;

Services::Services() : xmlServer_m(0), persistentStorage_m(0)
{}

Services::~Services()
{
    stop();
    if (xmlServer_m)
        delete xmlServer_m;
    if (persistentStorage_m)
        delete persistentStorage_m;
    IOPortManager::reset();
}

Services* Services::instance()
{
    if (instance_m == 0)
        instance_m = new Services();
    return instance_m;
}

void Services::start()
{
    timers_m.startManager();
    knxConnection_m.startConnection();
}

void Services::stop()
{
    infoStream("Services") << "Stopping services" << endlog;
    timers_m.stopManager();
    knxConnection_m.stopConnection();
}

void Services::createDefault()
{
    if (xmlServer_m)
        delete xmlServer_m;
    xmlServer_m = new XmlInetServer(1028);
}

void Services::importXml(ticpp::Element* pConfig)
{
    ticpp::Element* pSmsGateway = pConfig->FirstChildElement("smsgateway", false);
    if (pSmsGateway)
        smsGateway_m.importXml(pSmsGateway);
    ticpp::Element* pEmailGateway = pConfig->FirstChildElement("emailserver", false);
    if (pEmailGateway)
        emailGateway_m.importXml(pEmailGateway);
    ticpp::Element* pXmlServer = pConfig->FirstChildElement("xmlserver", false);
    if (pXmlServer)
    {
        if (xmlServer_m)
            delete xmlServer_m;
        xmlServer_m = XmlServer::create(pXmlServer);
    }
    ticpp::Element* pKnxConnection = pConfig->FirstChildElement("knxconnection", false);
    if (pKnxConnection)
        knxConnection_m.importXml(pKnxConnection);
    ticpp::Element* pExceptionDays = pConfig->FirstChildElement("exceptiondays", false);
    if (pExceptionDays)
        exceptionDays_m.importXml(pExceptionDays);
    ticpp::Element* pLocationInfo = pConfig->FirstChildElement("location", false);
    if (pLocationInfo)
        locationInfo_m.importXml(pLocationInfo);
    ticpp::Element* pPersistence = pConfig->FirstChildElement("persistence", false);
    if (pPersistence)
    {
        if (persistentStorage_m)
            delete persistentStorage_m;
        persistentStorage_m = PersistentStorage::create(pPersistence);
    }
    ticpp::Element* pIOPorts = pConfig->FirstChildElement("ioports", false);
    if (pIOPorts)
        IOPortManager::instance()->importXml(pIOPorts);
}

void Services::exportXml(ticpp::Element* pConfig)
{
    ticpp::Element pSmsGateway("smsgateway");
    smsGateway_m.exportXml(&pSmsGateway);
    pConfig->LinkEndChild(&pSmsGateway);

    ticpp::Element pEmailGateway("emailserver");
    emailGateway_m.exportXml(&pEmailGateway);
    pConfig->LinkEndChild(&pEmailGateway);

    if (xmlServer_m)
    {
        ticpp::Element pXmlServer("xmlserver");
        xmlServer_m->exportXml(&pXmlServer);
        pConfig->LinkEndChild(&pXmlServer);
    }

    ticpp::Element pKnxConnection("knxconnection");
    knxConnection_m.exportXml(&pKnxConnection);
    pConfig->LinkEndChild(&pKnxConnection);

    ticpp::Element pExceptionDays("exceptiondays");
    exceptionDays_m.exportXml(&pExceptionDays);
    pConfig->LinkEndChild(&pExceptionDays);

    if (!locationInfo_m.isEmpty())
    {
        ticpp::Element pLocationInfo("location");
        locationInfo_m.exportXml(&pLocationInfo);
        pConfig->LinkEndChild(&pLocationInfo);
    }

    if (persistentStorage_m)
    {
        ticpp::Element pPersistence("persistence");
        persistentStorage_m->exportXml(&pPersistence);
        pConfig->LinkEndChild(&pPersistence);
    }

    ticpp::Element pIOPorts("ioports");
    IOPortManager::instance()->exportXml(&pIOPorts);
    pConfig->LinkEndChild(&pIOPorts);
}
