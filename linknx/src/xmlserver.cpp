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

#include <unistd.h>
#include "xmlserver.h"
#include <sys/un.h>
#include <netinet/in.h>
#include <iostream>
#include "ruleserver.h"
#include "objectcontroller.h"
#include "timermanager.h"
#include "services.h"

XmlServer::~XmlServer ()
{
    Stop ();
    std::list<ClientConnection*>::iterator it;
    for (it = connections_m.begin(); it != connections_m.end(); it++)
    {
        (*it)->RemoveServer();
        (*it)->StopDelete();
    }
    //  while (!connections_m.empty())
    //    pth_yield (0);

    close (fd_m);
}

bool
XmlServer::deregister (ClientConnection * con)
{
    connections_m.remove(con);
    return 1;
}

XmlServer* XmlServer::create(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttributeOrDefault("type", "inet");
    if (type == "inet")
    {
        int port = 0;
        pConfig->GetAttributeOrDefault("port", &port, 1028);
        return new XmlInetServer(port);
    }
    else if (type == "unix")
    {
        std::string path = pConfig->GetAttributeOrDefault("port", "/tmp/xmlserver.sock");
        return new XmlUnixServer(path.c_str());
    }
    else
    {
        std::stringstream msg;
        msg << "XmlServer: server type not supported: '" << type << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

XmlInetServer::XmlInetServer (int port)
{
    struct sockaddr_in addr;
    int reuse = 1;

    port_m = port;
    std::cout << "XmlInetServer " << port_m << std::endl;

    memset (&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons (port);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);

    fd_m = socket (AF_INET, SOCK_STREAM, 0);
    if (fd_m == -1)
        throw 1;

    setsockopt (fd_m, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse));

    if (bind (fd_m, (struct sockaddr *) &addr, sizeof (addr)) == -1)
        throw 1;

    if (listen (fd_m, 10) == -1)
        throw 1;

    Start ();
}

void XmlInetServer::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "inet");
    pConfig->SetAttribute("port", port_m);
}

XmlUnixServer::XmlUnixServer (const char *path)
{
    struct sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strncpy (addr.sun_path, path, sizeof (addr.sun_path));

    path_m = path;
    std::cout << "XmlUnixServer " << path_m << std::endl;

    fd_m = socket (AF_LOCAL, SOCK_STREAM, 0);
    if (fd_m == -1)
        throw 1;

    unlink (path);
    if (bind (fd_m, (struct sockaddr *) &addr, sizeof (addr)) == -1)
        throw 1;

    if (listen (fd_m, 10) == -1)
        throw 1;

    Start ();
}

void XmlUnixServer::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "unix");
    pConfig->SetAttribute("path", path_m);
}

void XmlServer::Run (pth_sem_t *stop1)
{
    pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
    while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
        int cfd;
        cfd = pth_accept_ev (fd_m, 0, 0, stop);
        if (cfd != -1)
        {
            ClientConnection *c = new ClientConnection (this, cfd);
            connections_m.push_back(c);
            c->Start ();
        }
    }
    pth_event_free (stop, PTH_FREE_THIS);
}

ClientConnection::ClientConnection (XmlServer *server, int fd)
{
    fd_m = fd;
    server_m = server;
}

ClientConnection::~ClientConnection ()
{
    if (server_m)
        server_m->deregister (this);
    close (fd_m);
}

void ClientConnection::Run (pth_sem_t * stop1)
{
    pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
    while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
        if (readmessage (stop) == -1)
            break;
        std::string msgType;
        try
        {
            // Load a document
            ticpp::Document doc;
            //        std::cout << "PROCESSING MESSAGE:" << std::endl << msg_m << std::endl << "END OF MESSAGE" << std::endl;
            doc.LoadFromString(msg_m);

            ticpp::Element* pMsg = doc.FirstChildElement();
            msgType = pMsg->Value();
            if (msgType == "read")
            {
                ticpp::Element* pRead = pMsg->FirstChildElement();
                if (pRead->Value() == "object")
                {
                    std::string id = pRead->GetAttribute("id");
                    Object* obj = ObjectController::instance()->getObject(id);
                    std::stringstream msg;
                    msg << "<read status='success'>" << obj->getValue() << "</read>" << std::endl;
                    //            std::cout << "SENDING MESSAGE:" << std::endl << msg.str() << std::endl << "END OF MESSAGE" << std::endl;
                    sendmessage (msg.str(), stop);
                }
                else if (pRead->Value() == "config")
                {
                    ticpp::Element* pConfig = pRead->FirstChildElement(false);
                    if (pConfig == 0)
                    {
                        ticpp::Element objects("objects");
                        ObjectController::instance()->exportXml(&objects);
                        pRead->LinkEndChild(&objects);

                        ticpp::Element rules("rules");
                        RuleServer::instance()->exportXml(&rules);
                        pRead->LinkEndChild(&rules);

                        ticpp::Element services("services");
                        Services::instance()->exportXml(&services);
                        pRead->LinkEndChild(&services);
                    }
                    else if (pConfig->Value() == "objects")
                    {
                        ObjectController::instance()->exportXml(pConfig);
                    }
                    else if (pConfig->Value() == "rules")
                    {
                        RuleServer::instance()->exportXml(pConfig);
                    }
                    else if (pConfig->Value() == "services")
                    {
                        Services::instance()->exportXml(pConfig);
                    }
                    pMsg->SetAttribute("status", "success");
                    sendmessage (doc.GetAsString(), stop);
                }
                else
                    throw "Unknown read element";
            }
            else if (msgType == "write")
            {
                ticpp::Iterator< ticpp::Element > pWrite;
                for ( pWrite = pMsg->FirstChildElement(); pWrite != pWrite.end(); pWrite++ )
                {
                    if (pWrite->Value() == "object")
                    {
                        std::string id = pWrite->GetAttribute("id");
                        Object* obj = ObjectController::instance()->getObject(id);
                        obj->setValue(pWrite->GetAttribute("value"));
                    }
                    else if (pWrite->Value() == "config")
                    {
                        ticpp::Iterator< ticpp::Element > pConfigItem;
                        for ( pConfigItem = pWrite->FirstChildElement(); pConfigItem != pConfigItem.end(); pConfigItem++ )
                        {
                            if (pConfigItem->Value() == "objects")
                                ObjectController::instance()->importXml(&(*pConfigItem));
                            else if (pConfigItem->Value() == "rules")
                                RuleServer::instance()->importXml(&(*pConfigItem));
                            else if (pConfigItem->Value() == "services")
                                Services::instance()->importXml(&(*pConfigItem));
                            else
                                throw "Unknown config element";
                        }
                    }
                    else
                        throw "Unknown write element";
                }
                sendmessage ("<write status='success'/>\n", stop);
            }
            else
                throw "Unknown element";
        }
        catch( const char* ex )
        {
            sendreject (ex, msgType, stop);
        }
        catch( ticpp::Exception& ex )
        {
            sendreject (ex.m_details.c_str(), msgType, stop);
        }
    }
    pth_event_free (stop, PTH_FREE_THIS);
    StopDelete ();
}

int ClientConnection::sendreject (const char* msgstr, const std::string& type, pth_event_t stop)
{
    std::stringstream msg;
    if (type == "")
        msg << "<error>" << msgstr << "</error>" << std::endl;
    else
        msg << "<" << type << " status='error'>" << msgstr << "</" << type << ">" << std::endl;
    sendmessage (msg.str(), stop);
}

int ClientConnection::sendmessage (const std::string& msg, pth_event_t stop)
{
    return sendmessage(msg.length(), msg.c_str(), stop);
}

int ClientConnection::sendmessage (int size, const char * msg, pth_event_t stop)
{
    int i;
    int start = 0;

    start = 0;
    while (start < size)
    {
        i = pth_write_ev (fd_m, msg + start, size - start, stop);
        if (i <= 0)
            return -1;
        start += i;
    }
    pth_write_ev (fd_m, "\4", 1, stop);
    return 0;
}

int ClientConnection::readmessage (pth_event_t stop)
{
    char buf[256];
    int i;
    unsigned start;
    std::stringstream msg;

    while ((i = pth_read_ev (fd_m, &buf, 256, stop)) > 0)
    {
        std::string tstr(buf, i);
        msg << tstr;

        if (buf[i-1] == 4)
        {
            msg_m = msg.str();
            return 1;
        }
    }

    return -1;
}
