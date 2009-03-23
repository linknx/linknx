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

#ifndef KNXCONNECTION_H
#define KNXCONNECTION_H

#include "config.h"
#include "logger.h"
#include "threads.h"
#include <string>
#include "ticpp.h"
#include "eibclient.h"


class TelegramListener
{
public:
    virtual ~TelegramListener() {};
    virtual void onWrite(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len) = 0;
    virtual void onRead(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len) = 0;
    virtual void onResponse(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len) = 0;
};

class KnxConnection : public Thread
{
public:
    KnxConnection();
    virtual ~KnxConnection();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    void startConnection() { isRunning_m = true; Start(); };
    void stopConnection() { isRunning_m = false; Stop(); };

    void addTelegramListener(TelegramListener *listener);
    bool removeTelegramListener(TelegramListener *listener);
    void write(eibaddr_t gad, uint8_t* buf, int len);
    int checkInput();

private:
    EIBConnection *con_m;
    bool isRunning_m;
    pth_event_t stop_m;
    std::string url_m;
    TelegramListener *listener_m;

    void Run (pth_sem_t * stop);
    static Logger& logger_m;
};

#endif
