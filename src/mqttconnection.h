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

#ifndef MQTTCONNECTION_H
#define MQTTCONNECTION_H

#include "config.h"

#ifdef HAVE_MQTT

#include <string>
#include "logger.h"
#include "ticpp.h"
#include "knxconnection.h"
#include <mosquitto.h>

class MqttConnection : public TelegramListener
{
public:
    MqttConnection();
    virtual ~MqttConnection();

    virtual void importXml(ticpp::Element* pConfig);
    virtual void exportXml(ticpp::Element* pConfig);

    void startConnection();
    void stopConnection();
    bool isVoid() { return url_m.empty(); }

    // TelegramListener interface
    virtual void onWrite(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);
    virtual void onRead(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);
    virtual void onResponse(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);

private:
    void connect();
    void disconnect();
    void publishTelegram(const std::string& messageType, eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len);
    std::string formatGroupAddress(eibaddr_t addr);

    struct mosquitto* mosq_m;
    std::string url_m;
    int port_m;
    std::string username_m;
    std::string password_m;
    std::string prefix_m;
    bool connected_m;

    static Logger& logger_m;
};

#endif // HAVE_MQTT

#endif // MQTTCONNECTION_H