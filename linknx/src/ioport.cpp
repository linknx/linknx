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
#include <iomanip>
#include "ioport.h"
#include <fcntl.h>

Logger& IOPort::logger_m(Logger::getInstance("IOPort"));
Logger& RxThread::logger_m(Logger::getInstance("RxThread"));
Logger& UdpIOPort::logger_m(Logger::getInstance("UdpIOPort"));
Logger& TcpClientIOPort::logger_m(Logger::getInstance("TcpClientIOPort"));
Logger& SerialIOPort::logger_m(Logger::getInstance("SerialIOPort"));

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
    else if (type == "serial")
        return new SerialIOPort();
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
    if (isRxEnabled())
        rxThread_m.reset(new RxThread(this));
}

void IOPort::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("id", id_m);
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

void IOPort::addConnectListener(ConnectCondition *c)
{
    connectListenerList_m.push_back(c);
}

bool IOPort::removeConnectListener(ConnectCondition *c)
{
    connectListenerList_m.remove(c);
    return true;
}

void IOPort::onConnect()
{
    ConnectListenerList_t::iterator it;
    for (it = connectListenerList_m.begin(); it != connectListenerList_m.end(); it++)
    {
        (*it)->onConnect();
    }
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
    IOPort::exportXml(pConfig);
    pConfig->SetAttribute("type", "udp");
    pConfig->SetAttribute("host", host_m);
    pConfig->SetAttribute("port", port_m);
    if (rxport_m > 0)
        pConfig->SetAttribute("rxport", rxport_m);
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
    IOPort::exportXml(pConfig);
    pConfig->SetAttribute("type", "tcp");
    pConfig->SetAttribute("host", host_m);
    pConfig->SetAttribute("port", port_m);
    if (permanent_m)
        pConfig->SetAttribute("permanent", "true");
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
            onConnect();
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

SerialIOPort::SerialIOPort() : fd_m(-1)
{
    memset (&newtio_m, 0, sizeof (newtio_m));
}

SerialIOPort::~SerialIOPort()
{
    if (fd_m >= 0) {
        // restore old port settings
        tcsetattr(fd_m, TCSANOW, &oldtio_m);
        close(fd_m);
    }
    Logger::getInstance("SerialIOPort").debugStream() << "Deleting SerialIOPort " << endlog;
}

void SerialIOPort::importXml(ticpp::Element* pConfig)
{
    ErrorMessage err;
    int speed;
    struct termios newtio;
    std::string framing = pConfig->GetAttributeOrDefault("framing", "8N1");
    std::string flow = pConfig->GetAttributeOrDefault("flow", "none");
    std::string mode = pConfig->GetAttributeOrDefault("mode", "text");
    modeRaw_m = (mode == "raw");
    memset (&newtio, 0, sizeof (newtio));
    pConfig->GetAttribute("speed", &speed);
    newtio.c_cflag = CLOCAL | CREAD;
    newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;

    if (modeRaw_m)
    {
        timeout_m = RuleServer::parseDuration(pConfig->GetAttributeOrDefault("timeout", "0"), false, true) / 100;
        pConfig->GetAttributeOrDefault("msg-length", &msglength_m, 255);
        newtio.c_iflag = 0;
        newtio.c_lflag = 0;
        newtio.c_cc[VTIME] = timeout_m; // inter character timer (x100ms; 0=disabled)
        newtio.c_cc[VMIN]  = msglength_m; // block until timer expires or msglegth bytes are received
    }
    switch (framing[0]) {
        case '5':
            newtio.c_cflag |= CS5;
            break;
        case '6':
            newtio.c_cflag |= CS6;
            break;
        case '7':
            newtio.c_cflag |= CS7;
            break;
        case '8':
            newtio.c_cflag |= CS8;
            break;
        default:
            err << "Unsupported nb of data bits '" << framing[0] << "' for serial port";
            err.logAndThrow(logger_m);
    }
    switch (framing[1]) {
        case 'E':
            newtio.c_cflag |= PARENB;
            break;
        case 'O':
            newtio.c_cflag |= (PARENB | PARODD);
            break;
        case 'N':
            newtio.c_iflag |= IGNPAR;
            break;
        default:
            err << "Unsupported parity '" << framing[1] << "' for serial port";
            err.logAndThrow(logger_m);
    }

    if (framing[2] == '2')
        newtio.c_cflag |= CSTOPB;
    else if (framing[2] != '1') {
        err << "Unsupported nb of stop bits '" << framing[2] << "' for serial port";
        err.logAndThrow(logger_m);
    }

    if (flow == "xon-xoff")
        newtio.c_iflag |= (IXON | IXOFF);
    else if (flow == "rts-cts")
        newtio.c_cflag |= CRTSCTS;
    else if (flow != "none") {
        err << "Unsupported flow control '" << flow << "' for serial port";
        err.logAndThrow(logger_m);
    }

    switch (speed) {
        case 200:
            speed_m = B200;
            break;
        case 300:
            speed_m = B300;
            break;
        case 600:
            speed_m = B600;
            break;
        case 1200:
            speed_m = B1200;
            break;
        case 1800:
            speed_m = B1800;
            break;
        case 2400:
            speed_m = B2400;
            break;
        case 4800:
            speed_m = B4800;
            break;
        case 9600:
            speed_m = B9600;
            break;
        case 19200:
            speed_m = B19200;
            break;
        case 38400:
            speed_m = B38400;
            break;
        case 57600:
            speed_m = B57600;
            break;
        case 115200:
            speed_m = B115200;
            break;
        case 230400:
            speed_m = B230400;
            break;
        default:
            err << "Unsupported speed '" << speed << "' for serial port";
            err.logAndThrow(logger_m);
    }
    cfsetispeed(&newtio, speed_m);
    cfsetospeed(&newtio, speed_m);
    pConfig->GetAttribute("dev", &dev_m);
    newtio_m = newtio;

    IOPort::importXml(pConfig);

    fd_m = open(dev_m.c_str(), O_RDWR | O_NOCTTY );
    if (fd_m >= 0) {
        // Save previous port settings
        tcgetattr(fd_m, &oldtio_m);
        tcflush(fd_m, TCIFLUSH);
        tcsetattr(fd_m, TCSANOW, &newtio_m);
        logger_m.infoStream() << "SerialIOPort configured for device " << dev_m << endlog;
        onConnect();
    }
    else {
        logger_m.errorStream() << "Unable to open device '" << dev_m << "' for ioport " << getID() << endlog;
    }    
}

void SerialIOPort::exportXml(ticpp::Element* pConfig)
{
    int speed;
    IOPort::exportXml(pConfig);
    pConfig->SetAttribute("type", "serial");
    pConfig->SetAttribute("dev", dev_m);
    switch (speed_m) {
        case B200:
            speed = 200;
            break;
        case B300:
            speed = 300;
            break;
        case B600:
            speed = 600;
            break;
        case B1200:
            speed = 1200;
            break;
        case B1800:
            speed = 1800;
            break;
        case B2400:
            speed = 2400;
            break;
        case B4800:
            speed = 4800;
            break;
        case B9600:
            speed = 9600;
            break;
        case B19200:
            speed = 19200;
            break;
        case B38400:
            speed = 38400;
            break;
        case B57600:
            speed = 57600;
            break;
        case B115200:
            speed = 115200;
            break;
        case B230400:
            speed = 230400;
            break;
        default:
            speed = 9600;
            break;
    }
    pConfig->SetAttribute("speed", speed);

    std::string framing;
    switch (newtio_m.c_cflag & CSIZE) {
        case CS5:
            framing.push_back('5');
            break;
        case CS6:
            framing.push_back('6');
            break;
        case CS7:
            framing.push_back('7');
            break;
        default:
        case CS8:
            framing.push_back('8');
            break;
    }
    if (newtio_m.c_cflag & PARENB == 0)
        framing.push_back('N');
    else if (newtio_m.c_cflag & PARODD)
        framing.push_back('O');
    else
        framing.push_back('E');

    if (newtio_m.c_cflag & CSTOPB)
        framing.push_back('2');
    else
        framing.push_back('1');

    pConfig->SetAttribute("framing", framing);

    if (newtio_m.c_cflag & CRTSCTS)
        pConfig->SetAttribute("flow", "rts-cts");
    else if (newtio_m.c_iflag & IXON)
        pConfig->SetAttribute("flow", "xon-xoff");
    else
        pConfig->SetAttribute("flow", "none");

    if (modeRaw_m)
    {
        pConfig->SetAttribute("mode", "raw");
        if (timeout_m != 0)
            pConfig->SetAttribute("timeout", RuleServer::formatDuration(timeout_m*100, true));
        if (msglength_m != 255)
            pConfig->SetAttribute("msg-length", msglength_m);
    }
}

int SerialIOPort::send(const uint8_t* buf, int len)
{
    logger_m.infoStream() << "send(buf, len=" << len << "):"
        << buf << endlog;

    if (fd_m >= 0) {
        ssize_t nbytes = pth_write(fd_m, buf, len);
        if (nbytes == len) {
            return nbytes;
        }
        else {
            logger_m.errorStream() << "Unable to send to socket for ioport " << getID() << endlog;
        }
    }
    return -1;
}

int SerialIOPort::get(uint8_t* buf, int len, pth_event_t stop)
{
    logger_m.debugStream() << "get(buf, len=" << len << ")" << endlog;
    if (fd_m >= 0) {
        ssize_t i = pth_read_ev(fd_m, buf, len, stop);
//        logger_m.debugStream() << "Out of recvfrom " << i << " rl=" << rl << endlog;
        if (i > 0)
        {
            std::string msg(reinterpret_cast<const char*>(buf), i);
            if (logger_m.isDebugEnabled())
            {
                DbgStream dbg = logger_m.debugStream();
                dbg << "Received message on ioport " << getID() << ": ";
                if (modeRaw_m)
                {
                    dbg << std::hex << std::setfill ('0') << std::setw (2);
                    for (uint8_t *p = buf; p < buf+i; p++)
                        dbg << (int)*p << " ";
                    dbg << std::dec << endlog;
                }
                else
                    dbg << msg << endlog;
            }
            return i;
        }
    }
    return -1;
}

TxAction::TxAction(): varFlags_m(0), hex_m(false)
{}

TxAction::~TxAction()
{}

void TxAction::importXml(ticpp::Element* pConfig)
{
    int i=0;
    port_m = pConfig->GetAttribute("ioport");
    std::string data = pConfig->GetAttribute("data");
    if (!IOPortManager::instance()->getPort(port_m))
    {
        std::stringstream msg;
        msg << "TxAction: IO Port ID not found: '" << port_m << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }

    varFlags_m = 0;
    if (pConfig->GetAttributeOrDefault("hex", "false") != "false")
    {
        hex_m = true;
        while (i < data.length())
        {
            std::istringstream ss(data.substr(i, 2));
            ss.setf(std::ios::hex, std::ios::basefield);
            int value = 0;
            ss >> value;
            data_m.push_back(static_cast<char>(value));
            i += 2;
        }
        logger_m.infoStream() << "TxAction: Configured to send hex data to ioport " << port_m << endlog;
    }
    else
    {
        data_m = data;

        if (pConfig->GetAttribute("var") == "true")
        {
            varFlags_m = VarEnabled;
            if (parseVarString(data, true))
                varFlags_m |= VarData;
        }
        logger_m.infoStream() << "TxAction: Configured to send '" << data_m << "' to ioport " << port_m << endlog;
    }
}

void TxAction::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "ioport-tx");
    if (hex_m)
    {
        int i = 0;
        pConfig->SetAttribute("hex", "true");
        pConfig->SetAttribute("data", data_m);
        std::ostringstream ss;
        ss.setf(std::ios::hex, std::ios::basefield);
        ss.fill('0');
        while (i < data_m.length())
            ss << std::setw(2) << int(data_m[i++]);
        pConfig->SetAttribute("data", ss.str());
    }
    else
        pConfig->SetAttribute("data", data_m);
    pConfig->SetAttribute("ioport", port_m);
    if (varFlags_m & VarEnabled)
        pConfig->SetAttribute("var", "true");

    Action::exportXml(pConfig);
}

void TxAction::Run (pth_sem_t * stop)
{
    if (sleep(delay_m, stop))
        return;
    try
    {
        IOPort* port = IOPortManager::instance()->getPort(port_m);
        if (!port)
            throw ticpp::Exception("IO Port ID not found.");
        sendData(port);
    }
    catch( ticpp::Exception& ex )
    {
       logger_m.warnStream() << "Error in TxAction on port '" << port_m << "': " << ex.m_details << endlog;
    }
}

void TxAction::sendData(IOPort* port)
{
    std::string data = data_m;
    if (varFlags_m & VarData)
        parseVarString(data);
    if (hex_m)
        logger_m.infoStream() << "Execute TxAction send hex data to ioport " << port->getID() << endlog;
    else
        logger_m.infoStream() << "Execute TxAction send '" << data << "' to ioport " << port->getID() << endlog;
    const uint8_t* u8data = reinterpret_cast<const uint8_t*>(data.c_str());
    int len = data.length();
    int ret = port->send(u8data, len);
    while (ret < len) {
        if (ret <= 0)
            throw ticpp::Exception("Unable to send data.");
        len -= ret;
        u8data += ret;
        ret = port->send(u8data, len);
    }
}

RxCondition::RxCondition(ChangeListener* cl) : regexFlag_m(false), value_m(false), hex_m(false), pmatch_m(0), cl_m(cl)
{}

RxCondition::~RxCondition()
{
    IOPort* port = IOPortManager::instance()->getPort(port_m);
    if (port)
        port->removeListener(this);
    if (regexFlag_m)
        regfree(&regex_m);
    if (pmatch_m)
        delete pmatch_m;
    std::vector<Object*>::iterator it;
    for (it=objects_m.begin(); it!=objects_m.end(); it++)
        if (*it)
            (*it)->decRefCount();
}

bool RxCondition::evaluate()
{
    return value_m;
}

void RxCondition::importXml(ticpp::Element* pConfig)
{
    if (!cl_m)
        throw ticpp::Exception("Rx condition on IO port is not supported in this context");
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
    regexFlag_m = pConfig->GetAttributeOrDefault("regex", "false") != "false";
    hex_m = pConfig->GetAttributeOrDefault("hex", "false") != "false";
    if (regexFlag_m)
    {
        if (regcomp(&regex_m, exp_m.c_str(), REG_EXTENDED) != 0) {
            std::stringstream msg;
            msg << "RxCondition: Invalid regular expression: '" << exp_m << "'" << std::endl;
            throw ticpp::Exception(msg.str());
        }
        size_t nmatch = regex_m.re_nsub+1;
        if (nmatch > 0)
        {
            pmatch_m = new regmatch_t[nmatch];
            objects_m.resize(nmatch);
        }
        for (int i=0; i<nmatch; i++)
        {
            std::string id;
            std::stringstream obj;
            obj << "object" << i;
            id = pConfig->GetAttributeOrDefault(obj.str(),"");
            if (id != "")
                objects_m[i] = ObjectController::instance()->getObject(id);
            else
                objects_m[i] = 0;
            logger_m.debugStream() << "subgroup: " << i << " => Object '" << id << "'" << endlog;
        }
    }
    
    
    logger_m.infoStream() << "RxCondition: configured to watch for '" << exp_m << "' on ioport " << port_m << endlog;
}

void RxCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "ioport-rx");
    pConfig->SetAttribute("expected", exp_m);
    if (hex_m)
        pConfig->SetAttribute("hex", "true" );
    if (regexFlag_m)
        pConfig->SetAttribute("regex", "true" );
    std::vector<Object*>::iterator it;
    int i=0;
    for (it=objects_m.begin(); it!=objects_m.end(); it++)
    {
        if (*it)
        {
            std::stringstream obj;
            obj << "object" << i;
            pConfig->SetAttribute(obj.str(), (*it)->getID());
        }
        i++;
    }

    pConfig->SetAttribute("ioport", port_m);
}

void RxCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "ioport-rx");
    pStatus->SetAttribute("ioport", port_m);
}

void RxCondition::onDataReceived(const uint8_t* buf, unsigned int len)
{
    std::string rx(reinterpret_cast<const char*>(buf), len);
    if (hex_m)
    {
        int i = 0;
        std::ostringstream ss;
        ss.setf(std::ios::hex, std::ios::basefield);
        ss.fill('0');
        while (i < len)
            ss << std::setw(2) << int(buf[i++]);
        rx = ss.str();
    }
    if (cl_m)
    {
        if (regexFlag_m)
        {
            int status;
            size_t nmatch = regex_m.re_nsub+1;
            status = regexec(&regex_m, rx.c_str(), nmatch, pmatch_m, 0);
            if (status == 0)
            {
                logger_m.debugStream() << "RxCondition: expected message received: '" << rx << "'" << regex_m.re_nsub << endlog;
                for (int i=0; i<nmatch; i++)
                {
                    Object* obj = objects_m[i];
                    regmatch_t match = pmatch_m[i];
                    logger_m.debugStream() << "subgroup: " << i << " '" << match.rm_so << ":" << match.rm_eo << "'" << endlog;
                    if ((match.rm_so != (size_t)-1) && (obj)) {
                        std::string newValue(rx.c_str(), match.rm_so, match.rm_eo - match.rm_so);
                        logger_m.debugStream() << "RxCondition: new value " << newValue << " found in regex for object " << obj->getID() << endlog;

                        if (hex_m && newValue.length() <= 8)
                        {
                            unsigned int value;
                            std::istringstream val(newValue);
                            val >> std::hex >> value;
                            std::stringstream out;
                            out << value;
                            newValue = out.str();
                        }
                        try
                        {
                            obj->setValue(newValue);
                        }
                        catch( ticpp::Exception& ex )
                        {
                            logger_m.errorStream() << "RxCondition: Error cannot set value '" << newValue << "' to object '" << obj->getID() << "'" << endlog;
                        }
                    }
                }
                value_m = true;
                cl_m->onChange(0);
                value_m = false;
                cl_m->onChange(0);
                
            }
        }
        else
        {
            rx.resize(exp_m.length());
            if (exp_m == rx)
            {
                logger_m.debugStream() << "RxCondition: expected message received: '" << exp_m << "'" << endlog;
                value_m = true;
                cl_m->onChange(0);
                value_m = false;
                cl_m->onChange(0);
            }
        }
    }
}

ConnectCondition::ConnectCondition(ChangeListener* cl) : value_m(false), cl_m(cl)
{}

ConnectCondition::~ConnectCondition()
{
    IOPort* port = IOPortManager::instance()->getPort(port_m);
    if (port)
        port->removeConnectListener(this);
}

bool ConnectCondition::evaluate()
{
    return value_m;
}

void ConnectCondition::importXml(ticpp::Element* pConfig)
{
    if (!cl_m)
        throw ticpp::Exception("Connect condition on IO port is not supported in this context");
    port_m = pConfig->GetAttribute("ioport");
    
    IOPort* port = IOPortManager::instance()->getPort(port_m);
    if (!port)
    {
        std::stringstream msg;
        msg << "ConnectCondition: IO Port ID not found: '" << port_m << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }

    if (!port->mustConnect())
    {
        std::stringstream msg;
        msg << "ConnectCondition: Only serial and TCP port can use connectCondition" << std::endl;
        throw ticpp::Exception(msg.str());
    }

    port->addConnectListener(this);
}

void ConnectCondition::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "ioport-connect");
    pConfig->SetAttribute("ioport", port_m);
}

void ConnectCondition::statusXml(ticpp::Element* pStatus)
{
    pStatus->SetAttribute("type", "ioport-connect");
    pStatus->SetAttribute("ioport", port_m);
}

void ConnectCondition::onConnect()
{
    logger_m.debugStream() << "ConnectCondition: IOPort '" << port_m << "' is connected" << endlog;
    value_m = true;
    cl_m->onChange(0);
    value_m = false;
    cl_m->onChange(0);
}
