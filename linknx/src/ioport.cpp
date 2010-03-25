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
Logger& TcpClientIOPort::logger_m(Logger::getInstance("TcpClientIOPort"));

IOPortManager* IOPortManager::instance_m;

IOPortManager::IOPortManager()
{}

IOPortManager::~IOPortManager()
{
    IOPortMap_t::iterator it;
    for (it = portMap_m.begin(); it != portMap_m.end(); it++) {
        delete (*it).second;
    }
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
        return 0;
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

IOPort::IOPort()
{}

IOPort::~IOPort()
{
}

IOPort* IOPort::create(const std::string& type)
{
    if (type == "" || type == "udp")
        return new UdpIOPort();
    else if (type == "tcp")
        return new TcpClientIOPort();
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
    if (isRxEnabled())
        rxThread_m.reset(new RxThread(this));
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

RxThread::RxThread(IOPort *port) : port_m(port), isRunning_m(false), stop_m(0)
{}

RxThread::~RxThread()
{
    Stop();
}

void RxThread::addListener(IOPortListener *listener)
{
    if (listenerList_m.empty())
        Start();
    listenerList_m.push_back(listener);
}

bool RxThread::removeListener(IOPortListener *listener)
{
    listenerList_m.remove(listener);
    if (listenerList_m.empty())
        Stop();
    return true;
}

void RxThread::Run (pth_sem_t * stop1)
{
    stop_m = pth_event (PTH_EVENT_SEM, stop1);
    uint8_t buf[1024];
    int retval;
    logger_m.debugStream() << "Start IO Port loop." << endlog;
    while ((retval = port_m->get(buf, sizeof(buf), stop_m)) > 0)
    {
        ListenerList_t::iterator it;
        for (it = listenerList_m.begin(); it != listenerList_m.end(); it++)
        {
//            logger_m.debugStream() << "Calling onDataReceived on listener for " << port_m->getID() << endlog;
            (*it)->onDataReceived(buf, retval);
        }
    }
    logger_m.debugStream() << "Out of IO Port loop." << endlog;
    pth_event_free (stop_m, PTH_FREE_THIS);
    stop_m = 0;
}

UdpIOPort::UdpIOPort() : sockfd_m(-1), port_m(0), rxport_m(0)
{
    memset (&addr_m, 0, sizeof (addr_m));
}

UdpIOPort::~UdpIOPort()
{
    if (sockfd_m >= 0)
        close(sockfd_m);
    Logger::getInstance("UdpIOPort").debugStream() << "Deleting UdpIOPort " << endlog;
}

void UdpIOPort::importXml(ticpp::Element* pConfig)
{
    memset (&addr_m, 0, sizeof (addr_m));
    addr_m.sin_family = AF_INET;
    pConfig->GetAttribute("port", &port_m);
    addr_m.sin_port = htons(port_m);
    host_m = pConfig->GetAttribute("host");
    addr_m.sin_addr.s_addr = inet_addr(host_m.c_str());
    pConfig->GetAttributeOrDefault("rxport", &rxport_m, 0);
    IOPort::importXml(pConfig);

    sockfd_m = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_m >= 0 && rxport_m > 0) {
        struct sockaddr_in addr;
        bzero(&addr,sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(rxport_m);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(sockfd_m, (struct sockaddr *)&addr,sizeof(addr)) < 0) /* error */
        {
            logger_m.errorStream() << "Unable to bind socket for ioport " << getID() << endlog;
        }
    }
    else {
        logger_m.errorStream() << "Unable to create  socket for ioport " << getID() << endlog;
    }    
    
   
    logger_m.infoStream() << "UdpIOPort configured for host " << host_m << " and port " << port_m << endlog;
}

void UdpIOPort::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("host", host_m);
    pConfig->SetAttribute("port", port_m);
    if (rxport_m > 0)
        pConfig->SetAttribute("rxport", rxport_m);
    IOPort::exportXml(pConfig);
}

int UdpIOPort::send(const uint8_t* buf, int len)
{
    logger_m.infoStream() << "send(buf, len=" << len << "):"
        << buf << endlog;

    if (sockfd_m >= 0) {
        ssize_t nbytes = pth_sendto(sockfd_m, buf, len, 0,
               (const struct sockaddr *) &addr_m, sizeof (addr_m));
        if (nbytes == len) {
            return nbytes;
        }
        else {
            logger_m.errorStream() << "Unable to send to socket for ioport " << getID() << endlog;
        }
    }
    return -1;
}

int UdpIOPort::get(uint8_t* buf, int len, pth_event_t stop)
{
    logger_m.debugStream() << "get(buf, len=" << len << "):"
        << buf << endlog;
    if (sockfd_m >= 0) {
        socklen_t rl;
        sockaddr_in r;
        rl = sizeof (r);
        memset (&r, 0, sizeof (r));
        ssize_t i = pth_recvfrom_ev(sockfd_m, buf, len, 0,
               (struct sockaddr *) &r, &rl, stop);
//        logger_m.debugStream() << "Out of recvfrom " << i << " rl=" << rl << endlog;
        if (i > 0 && rl == sizeof (r))
        {
            std::string msg(reinterpret_cast<const char*>(buf), i);
            logger_m.debugStream() << "Received '" << msg << "' on ioport " << getID() << endlog;
            return i;
        }
    }
    return -1;
}

TcpClientIOPort::TcpClientIOPort() : sockfd_m(-1), port_m(0)
{
    memset (&addr_m, 0, sizeof (addr_m));
}

TcpClientIOPort::~TcpClientIOPort()
{
    if (sockfd_m >= 0)
        close(sockfd_m);
    Logger::getInstance("TcpClientIOPort").debugStream() << "Deleting TcpClientIOPort " << endlog;
}

void TcpClientIOPort::importXml(ticpp::Element* pConfig)
{
    memset (&addr_m, 0, sizeof (addr_m));
    addr_m.sin_family = AF_INET;
    pConfig->GetAttribute("port", &port_m);
    addr_m.sin_port = htons(port_m);
    host_m = pConfig->GetAttribute("host");
    addr_m.sin_addr.s_addr = inet_addr(host_m.c_str());
    std::string perm = pConfig->GetAttribute("permanent");
    permanent_m = (perm == "true" || perm == "yes");
    IOPort::importXml(pConfig);

    logger_m.infoStream() << "TcpClientIOPort " << (permanent_m?"(permanent) ":"") << "configured for host " << host_m << " and port " << port_m << endlog;
}

void TcpClientIOPort::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("host", host_m);
    pConfig->SetAttribute("port", port_m);
    if (permanent_m)
        pConfig->SetAttribute("permanent", "true");
    IOPort::exportXml(pConfig);
}

int TcpClientIOPort::send(const uint8_t* buf, int len)
{
    logger_m.infoStream() << "send(buf, len=" << len << "):"
        << buf << endlog;

    connectToServer();
    if (sockfd_m >= 0) {
        ssize_t nbytes = pth_write(sockfd_m, buf, len);
        if (nbytes == len) {
            if (!permanent_m)
                disconnectFromServer();
            return nbytes;
        }
        else {
            logger_m.errorStream() << "Error while sending data for ioport " << getID() << endlog;
            disconnectFromServer();
        }
    }
    return -1;
}

int TcpClientIOPort::get(uint8_t* buf, int len, pth_event_t stop)
{
    logger_m.debugStream() << "get(buf, len=" << len << ")" << endlog;
    bool retry = true;
    while (retry) {
        connectToServer();
        if (sockfd_m >= 0) {
            ssize_t i = pth_read_ev(sockfd_m, buf, len, stop);
            logger_m.debugStream() << "Out of read " << i << endlog;
            if (i > 0)
            {
                std::string msg(reinterpret_cast<const char*>(buf), i);
                logger_m.debugStream() << "Received '" << msg << "' on ioport " << getID() << endlog;
                return i;
            }
            else {
                disconnectFromServer();
                if (pth_event_status (stop) == PTH_STATUS_OCCURRED)
                    retry = false;
            }
        }
        else {
            struct timeval tv;
            tv.tv_sec = 60;
            tv.tv_usec = 0;
            pth_select_ev(0,0,0,0,&tv,stop);
            if (pth_event_status (stop) == PTH_STATUS_OCCURRED)
                retry = false;
        }
    }
    logger_m.debugStream() << "Abort get() on ioport " << getID() << endlog;
    return -1;
}

void TcpClientIOPort::connectToServer()
{
    if (sockfd_m < 0) {
        sockfd_m = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_m >= 0) {
            if (pth_connect(sockfd_m, (const struct sockaddr *) &addr_m, sizeof (addr_m)) < 0) {
                logger_m.errorStream() << "Unable to connect to server for ioport " << getID() << endlog;
                disconnectFromServer();
            }
        }
        else {
            logger_m.errorStream() << "Unable to create  socket for ioport " << getID() << endlog;
        }    
    }
}

void TcpClientIOPort::disconnectFromServer()
{
    if (close(sockfd_m) < 0) {
        logger_m.errorStream() << "Unable to close connection to server for ioport " << getID() << endlog;
    }
    sockfd_m = -1;
}

TxAction::TxAction()
{}

TxAction::~TxAction()
{}

void TxAction::importXml(ticpp::Element* pConfig)
{
    port_m = pConfig->GetAttribute("ioport");
    data_m = pConfig->GetAttribute("data");
    if (!IOPortManager::instance()->getPort(port_m))
    {
        std::stringstream msg;
        msg << "TxAction: IO Port ID not found: '" << port_m << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }

    logger_m.infoStream() << "TxAction: Configured to send '" << data_m << "' to ioport " << port_m << endlog;
}

void TxAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "ioport-tx");
    pConfig->SetAttribute("data", data_m);
    pConfig->SetAttribute("ioport", port_m);

    Action::exportXml(pConfig);
}

void TxAction::Run (pth_sem_t * stop)
{
    pth_sleep(delay_m);
    try
    {
        IOPort* port = IOPortManager::instance()->getPort(port_m);
        if (!port)
            throw ticpp::Exception("IO Port ID not found.");
        logger_m.infoStream() << "Execute TxAction send '" << data_m << "' to ioport " << port_m << endlog;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(data_m.c_str());
        int len = data_m.length();
        int ret = port->send(data, len);
        if (ret != len) {
            ret = port->send(data, len);
            if (ret != len)
                throw ticpp::Exception("Unable to send data.");
        }
        
    }
    catch( ticpp::Exception& ex )
    {
       logger_m.warnStream() << "Error in TxAction on port '" << port_m << "': " << ex.m_details << endlog;
    }
}

RxCondition::RxCondition(ChangeListener* cl) : value_m(false), cl_m(cl)
{}

RxCondition::~RxCondition()
{
    IOPort* port = IOPortManager::instance()->getPort(port_m);
    if (port)
        port->removeListener(this);
}

bool RxCondition::evaluate()
{
    return value_m;
}

void RxCondition::importXml(ticpp::Element* pConfig)
{
    port_m = pConfig->GetAttribute("ioport");
    exp_m = pConfig->GetAttribute("expected");
    IOPort* port = IOPortManager::instance()->getPort(port_m);
    if (!port)
    {
        std::stringstream msg;
        msg << "RxCondition: IO Port ID not found: '" << port_m << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
    port->addListener(this);
    logger_m.infoStream() << "RxCondition: configured to watch for '" << exp_m << "' on ioport " << port_m << endlog;
}

void RxCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "ioport-rx");
    pConfig->SetAttribute("expected", exp_m);
    pConfig->SetAttribute("ioport", port_m);
}

void RxCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "ioport-rx");
    pStatus->SetAttribute("ioport", port_m);
}

void RxCondition::onDataReceived(const uint8_t* buf, unsigned int len)
{
    if (len > exp_m.length())
        len = exp_m.length();
    std::string rx(reinterpret_cast<const char*>(buf), len); 
    if (cl_m && exp_m == rx)
    {
        logger_m.debugStream() << "RxCondition: expected message received: '" << exp_m << "'" << endlog;
        value_m = true;
        cl_m->onChange(0);
        value_m = false;
        cl_m->onChange(0);
    }        
}
