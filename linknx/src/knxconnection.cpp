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

#include <iostream>
#include "objectcontroller.h"
#include "knxconnection.h"
extern "C"
{
#include "common.h"
}

Logger& KnxConnection::logger_m(Logger::getInstance("KnxConnection"));

KnxConnection::KnxConnection() : con_m(0), isRunning_m(false), stop_m(0), listener_m(0)
{}

KnxConnection::~KnxConnection()
{
    if (con_m)
        EIBClose(con_m);
}

void KnxConnection::importXml(ticpp::Element* pConfig)
{
    url_m = pConfig->GetAttribute("url");
    if (isRunning_m)
    {
        Stop();
        Start();
    }
}

void KnxConnection::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("url", url_m);
}

void KnxConnection::addTelegramListener(TelegramListener *listener)
{
    if (listener_m)
        throw ticpp::Exception("KnxConnection: TelegramListener already registered");
    listener_m = listener;
}

bool KnxConnection::removeTelegramListener(TelegramListener *listener)
{
    if (listener_m != listener)
        return false;
    listener_m = 0;
    return true;
}

void KnxConnection::write(eibaddr_t gad, uint8_t* buf, int len)
{
    if(gad == 0)
        return;
    logger_m.infoStream() << "write(gad=" << gad << ", buf, len=" << len << "):"
        << buf << endlog;
//    printf ("ObjectController::write(gad=%d, buf, len=%d):", gad, len);
//    printHex (len, buf);
//    printf ("\n");
    if (con_m)
    {
        len = EIBSendGroup (con_m, gad, len, buf);
        if (len == -1)
            die ("Request failed");
        printf ("Send request\n");
    }
}

void KnxConnection::Run (pth_sem_t * stop1)
{
    if (url_m == "")
        return;
    stop_m = pth_event (PTH_EVENT_SEM, stop1);
    bool retry = true;
    while (retry)
    {
        con_m = EIBSocketURL(url_m.c_str());
        if (con_m)
        {
            EIBSetEvent (con_m, stop_m);
            if (EIBOpen_GroupSocket (con_m, 0) != -1)
            {
                logger_m.infoStream() << "KnxConnection: Group socket opened. Waiting for messages." << endlog;
                int retval;
                while ((retval = checkInput()) > 0)
                {
                    /*        TODO: find another way to check if event occured
                              struct timeval tv;
                    	        tv.tv_sec = 1;
                      	      tv.tv_usec = 0;
                              pth_select_ev(0,0,0,0,&tv,stop);
                    */
                }
                if (retval == -1)
                    retry = false;
            }
            else
                logger_m.errorStream() << "Failed to open group socket." << endlog;

            if (con_m)
                EIBClose(con_m);
            con_m = 0;
        }
        else
            logger_m.errorStream() << "Failed to open knxConnection url." << endlog;
        if (retry)
        {
            struct timeval tv;
            tv.tv_sec = 60;
            tv.tv_usec = 0;
            pth_select_ev(0,0,0,0,&tv,stop_m);
            if (pth_event_status (stop_m) == PTH_STATUS_OCCURRED)
                retry = false;
        }
    }
    logger_m.infoStream() << "Out of KnxConnection loop." << endlog;
    pth_event_free (stop_m, PTH_FREE_THIS);
    stop_m = 0;
}

int KnxConnection::checkInput()
{
    int len;
    eibaddr_t dest;
    eibaddr_t src;
    uint8_t buf[200];
    if (!con_m)
        return 0;
    len = EIBGetGroup_Src (con_m, sizeof (buf), buf, &src, &dest);
    if (pth_event_status (stop_m) == PTH_STATUS_OCCURRED)
        return -1;
    if (len == -1)
        die ("Read failed");
    if (len < 2)
        die ("Invalid Packet");
    if (buf[0] & 0x3 || (buf[1] & 0xC0) == 0xC0)
    {
        logger_m.warnStream() << "Unknown APDU from "<< src << " to " << dest << endlog;
/*        printf ("Unknown APDU from ");
        printIndividual (src);
        printf (" to ");
        printGroup (dest);
        printf (": ");
        printHex (len, buf);
        printf ("\n");*/
    }
    else
    {
        switch (buf[1] & 0xC0)
        {
        case 0x00:
            printf ("Read");
            break;
        case 0x40:
            printf ("Response");
            break;
        case 0x80:
            printf ("Write");
            break;
        }
        printf (" from ");
        printIndividual (src);
        printf (" to ");
        printGroup (dest);
        if (buf[1] & 0xC0)
        {
            printf (": ");
            if (len == 2)
                printf ("%02X", buf[1] & 0x3F);
            else
                printHex (len - 2, buf + 2);
        }
        printf ("\n");
        if (listener_m)
        {
            switch (buf[1] & 0xC0)
            {
            case 0x00:
                listener_m->onRead(src, dest, buf, len);
                break;
            case 0x40:
                listener_m->onResponse(src, dest, buf, len);
                break;
            case 0x80:
                listener_m->onWrite(src, dest, buf, len);
                break;
            }
        }
    }
    return 1;
}
