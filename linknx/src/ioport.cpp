/*
    LinKNX KNX home automation platform
    Copyright (C) 2007-2009 Jean-François Meessen <linknx@ouaye.net>
 
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

#include <iostream>
#include "ioport.h"

Logger& IOPort::logger_m(Logger::getInstance("IOPort"));
Logger& RxThread::logger_m(Logger::getInstance("RxThread"));
Logger& UdpIOPort::logger_m(Logger::getInstance("UdpIOPort"));

IOPortManager* IOPortManager::instance_m;

IOPortManager::IOPortManager()
{}

IOPortManager::~IOPortManager()
{
    IOPortMap_t::iterator it;
    for (it = portMap_m.begin(); it != portMap_m.end(); it++)
        delete (*it).second;
}

IOPortManager* IOPortManager::instance()
{
    if (instance_m == 0)
        instance_m = new IOPortManager();
    return instance_m;
}

IOPort* IOPortManager::getPort(const std::string& id)
{
    IOPortMap_t::iterator it = portMap_m.find(id);
    if (it == portMap_m.end())
    {
        std::stringstream msg;
        msg << "IOPortManager: IO Port ID not found: '" << id << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    return (*it).second;
}

void IOPortManager::addPort(IOPort* port)
{
    if (!portMap_m.insert(IOPortPair_t(port->getID(), port)).second)
        throw ticpp::Exception("IO Port ID already exists");
}

void IOPortManager::removePort(IOPort* port)
{
    IOPortMap_t::iterator it = portMap_m.find(port->getID());
    if (it != portMap_m.end())
    {
        delete it->second;
        portMap_m.erase(it);
    }
}

void IOPortManager::importXml(ticpp::Element* pConfig)
{
    ticpp::Iterator< ticpp::Element > child("ioport");
    for ( child = pConfig->FirstChildElement("ioport", false); child != child.end(); child++ )
    {
        std::string id = child->GetAttribute("id");
        bool del = child->GetAttribute("delete") == "true";
        IOPortMap_t::iterator it = portMap_m.find(id);
        if (it != portMap_m.end())
        {
            IOPort* port = it->second;

            if (del)
            {
                delete port;
                portMap_m.erase(it);
            }
            else
            {
                port->importXml(&(*child));
                portMap_m.insert(IOPortPair_t(id, port));
            }
        }
        else
        {
            if (del)
                throw ticpp::Exception("IO Port not found");
            IOPort* port = IOPort::create(&(*child));
            portMap_m.insert(IOPortPair_t(id, port));
        }
    }

}

void IOPortManager::exportXml(ticpp::Element* pConfig)
{
    IOPortMap_t::iterator it;
    for (it = portMap_m.begin(); it != portMap_m.end(); it++)
    {
        ticpp::Element pElem("ioport");
        (*it).second->exportXml(&pElem);
        pConfig->LinkEndChild(&pElem);
    }
}

/*
void IOPortManager::exportObjectValues(ticpp::Element* pObjects)
{
    ObjectIdMap_t::iterator it;
    for (it = objectIdMap_m.begin(); it != objectIdMap_m.end(); it++)
    {
        ticpp::Element pElem("object");
        pElem.SetAttribute("id", (*it).second->getID());
        pElem.SetAttribute("value", (*it).second->getValue());
        pObjects->LinkEndChild(&pElem);
    }
}
*/

IOPort::IOPort()
{}

IOPort::~IOPort()
{
}

IOPort* IOPort::create(const std::string& type)
{
    if (type == "" || type == "udp")
        return new UdpIOPort();
    else
        return 0;
}

IOPort* IOPort::create(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttribute("type");
    IOPort* obj = IOPort::create(type);
    if (obj == 0)
    {
        std::stringstream msg;
        msg << "IOPort type not supported: '" << type << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    obj->importXml(pConfig);
    return obj;
}

void IOPort::importXml(ticpp::Element* pConfig)
{
    id_m = pConfig->GetAttribute("id");
    url_m = pConfig->GetAttribute("url");
}

void IOPort::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("id", id_m);
    pConfig->SetAttribute("url", url_m);
}

void IOPort::addListener(IOPortListener *l)
{
    if (rxThread_m.get())
        rxThread_m->addListener(l);
}

bool IOPort::removeListener(IOPortListener *l)
{
    if (rxThread_m.get())
        return (rxThread_m->removeListener(l));
    else
        return false;
}

RxThread::RxThread(IOPort *port) : port_m(port), listener_m(0), stop_m(0), isRunning_m(false)
{}

RxThread::~RxThread()
{
}

void RxThread::addListener(IOPortListener *listener)
{
    if (listener_m)
        throw ticpp::Exception("KnxPort: TelegramListener already registered");
    listener_m = listener;
}

bool RxThread::removeListener(IOPortListener *listener)
{
    if (listener_m != listener)
        return false;
    listener_m = 0;
    return true;
}

void RxThread::Run (pth_sem_t * stop1)
{
    stop_m = pth_event (PTH_EVENT_SEM, stop1);
    bool retry = true;
    uint8_t buf[1024];
    int retval;
    while ((retval = port_m->get(buf, sizeof(buf))) > 0)
    {
    }
    logger_m.infoStream() << "Out of IO Port loop." << endlog;
    pth_event_free (stop_m, PTH_FREE_THIS);
    stop_m = 0;
}

UdpIOPort::UdpIOPort() : port_m(0)
{
    memset (&addr_m, 0, sizeof (addr_m));
}

UdpIOPort::~UdpIOPort()
{
}

void UdpIOPort::importXml(ticpp::Element* pConfig)
{
    memset (&addr_m, 0, sizeof (addr_m));
    addr_m.sin_family = AF_INET;
    pConfig->GetAttribute("port", &port_m);
    addr_m.sin_port = htons(port_m);
    host_m = pConfig->GetAttribute("host");
    addr_m.sin_addr.s_addr = inet_addr(host_m.c_str());
    IOPort::importXml(pConfig);
    logger_m.infoStream() << "UdpIOPort configured for host " << host_m << " and port " << port_m << endlog;
}

void UdpIOPort::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("host", host_m);
    pConfig->SetAttribute("port", port_m);
    IOPort::exportXml(pConfig);
}

void UdpIOPort::send(const uint8_t* buf, int len)
{
    logger_m.infoStream() << "send(buf, len=" << len << "):"
        << buf << endlog;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd >= 0) {
        size_t i = pth_sendto(sockfd, buf, len, 0,
               (const struct sockaddr *) &addr_m, sizeof (addr_m));
        if (i > 0) {
            
        }
        else {
            logger_m.errorStream() << "Unable to send to socket for ioport " << getID() << endlog;
        }
    }
    else {
        logger_m.errorStream() << "Unable to create  socket for ioport " << getID() << endlog;
    }    
}

int UdpIOPort::get(uint8_t* buf, int len)
{
    logger_m.infoStream() << "get(buf, len=" << len << "):"
        << buf << endlog;
    buf[0]='O';
    buf[1]='K';
    return 2;
}

TxAction::TxAction() : port_m(0)
{}

TxAction::~TxAction()
{}

void TxAction::importXml(ticpp::Element* pConfig)
{
    std::string portId;
    portId = pConfig->GetAttribute("ioport");
    port_m = IOPortManager::instance()->getPort(portId);

    data_m = pConfig->GetAttribute("data");

    logger_m.infoStream() << "TxAction: Configured to send '" << data_m << "' to ioport " << port_m->getID() << endlog;
}

void TxAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "tx");
    pConfig->SetAttribute("data", data_m);
    pConfig->SetAttribute("ioport", port_m->getID());

    Action::exportXml(pConfig);
}

void TxAction::Run (pth_sem_t * stop)
{
    if (port_m)
    {
        pth_sleep(delay_m);
        try
        {
            port_m->send(reinterpret_cast<const uint8_t*>(data_m.c_str()), data_m.length());
            logger_m.infoStream() << "Execute TxAction send '" << data_m << "' to ioport " << port_m->getID() << endlog;
        }
        catch( ticpp::Exception& ex )
        {
            logger_m.warnStream() << "Error in TxAction: " << ex.m_details << endlog;
        }
    }
}
