/*
    LinKNX KNX home automation platform
    Copyright (C) 2007 Jean-François Meessen <linknx@ouaye.net>
 
    Portions of code borrowed to EIBD (http://bcusdk.sourceforge.net/)
    Copyright (C) 2005-2006 Martin Kögler <mkoegler@auto.tuwien.ac.at>
 
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

#ifndef XMLSERVER_H
#define XMLSERVER_H

#include "config.h"
#include "threads.h"
#include <list>
#include <string>
#include "ticpp.h"
#include "objectcontroller.h"


class ClientConnection;

class XmlServer : protected Thread
{
public:
    virtual ~XmlServer();

    static XmlServer* create(ticpp::Element* pConfig);

    virtual void exportXml(ticpp::Element* pConfig) = 0;

    bool deregister (ClientConnection *con);
protected:
    int fd_m;
private:
    std::list<ClientConnection*> connections_m;

    void Run (pth_sem_t * stop);
};

class XmlInetServer : public XmlServer
{
public:
    XmlInetServer(int port);
    virtual ~XmlInetServer() {};

    virtual void exportXml(ticpp::Element* pConfig);
private:
    int port_m;
};

class XmlUnixServer : public XmlServer
{
public:
    XmlUnixServer(const char *path);
    virtual ~XmlUnixServer() {};

    virtual void exportXml(ticpp::Element* pConfig);
private:
    std::string path_m;
};

class ClientConnection : public Thread, public ChangeListener
{
public:
    ClientConnection (XmlServer *server, int fd);
    virtual ~ ClientConnection ();

    void RemoveServer() { server_m = 0; };

    int readmessage (pth_event_t stop);
    int sendmessage (int size, const char * msg, pth_event_t stop);
    int sendmessage (std::string msg, pth_event_t stop);
    int sendreject (const char* msgstr, const std::string& type, pth_event_t stop);

    virtual void onChange(Object* object);

    std::string msg_m;
    std::string msgbuf_m;
private:
    int fd_m;
    XmlServer *server_m;

    typedef std::list<Object*> NotifyList_t;
    NotifyList_t notifyList_m;

    void Run (pth_sem_t * stop);
};

#endif
