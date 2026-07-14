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

#include "config.h"

#ifdef HAVE_MQTT

#include "mqttconnection.h"
#include "objectcontroller.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>

Logger& MqttConnection::logger_m(Logger::getInstance("MqttConnection"));

MqttConnection::MqttConnection() : mosq_m(0), port_m(1883), connected_m(false)
{
    mosquitto_lib_init();
}

MqttConnection::~MqttConnection()
{
    disconnect();
    mosquitto_lib_cleanup();
}

void MqttConnection::importXml(ticpp::Element* pConfig)
{
    try {
        url_m = pConfig->GetAttribute("url");
        std::string portStr = pConfig->GetAttribute("port");
        port_m = atoi(portStr.c_str());
        username_m = pConfig->GetAttribute("username");
        password_m = pConfig->GetAttribute("password");
        prefix_m = pConfig->GetAttribute("prefix");

        logger_m.infoStream() << "MQTT configuration loaded: url=" << url_m
                              << ", port=" << port_m
                              << ", prefix=" << prefix_m << endlog;
    }
    catch (ticpp::Exception& ex) {
        logger_m.errorStream() << "Error parsing MQTT configuration: " << ex.m_details << endlog;
        throw;
    }
}

void MqttConnection::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("url", url_m);
    pConfig->SetAttribute("port", port_m);
    pConfig->SetAttribute("username", username_m);
    pConfig->SetAttribute("password", password_m);
    pConfig->SetAttribute("prefix", prefix_m);
}

void MqttConnection::startConnection()
{
    if (!isVoid())
    {
        connect();
    }
}

void MqttConnection::stopConnection()
{
    disconnect();
}

void MqttConnection::connect()
{
    if (mosq_m)
    {
        logger_m.warnStream() << "MQTT connection already exists, disconnecting first" << endlog;
        disconnect();
    }

    // Create mosquitto client instance
    mosq_m = mosquitto_new(nullptr, true, nullptr);
    if (!mosq_m)
    {
        logger_m.errorStream() << "Failed to create MQTT client instance" << endlog;
        return;
    }

    // Set username and password
    if (!username_m.empty())
    {
        int ret = mosquitto_username_pw_set(mosq_m, username_m.c_str(), password_m.c_str());
        if (ret != MOSQ_ERR_SUCCESS)
        {
            logger_m.errorStream() << "Failed to set MQTT credentials: "
                                   << mosquitto_strerror(ret) << endlog;
            mosquitto_destroy(mosq_m);
            mosq_m = nullptr;
            return;
        }
    }

    // Connect to broker
    logger_m.infoStream() << "Connecting to MQTT broker at " << url_m << ":" << port_m << endlog;
    int ret = mosquitto_connect(mosq_m, url_m.c_str(), port_m, 60);
    if (ret != MOSQ_ERR_SUCCESS)
    {
        logger_m.errorStream() << "Failed to connect to MQTT broker: "
                               << mosquitto_strerror(ret) << endlog;
        mosquitto_destroy(mosq_m);
        mosq_m = nullptr;
        return;
    }

    // Start the network loop in a background thread
    ret = mosquitto_loop_start(mosq_m);
    if (ret != MOSQ_ERR_SUCCESS)
    {
        logger_m.errorStream() << "Failed to start MQTT loop: "
                               << mosquitto_strerror(ret) << endlog;
        mosquitto_destroy(mosq_m);
        mosq_m = nullptr;
        return;
    }

    connected_m = true;
    logger_m.infoStream() << "MQTT connection established" << endlog;
}

void MqttConnection::disconnect()
{
    if (mosq_m)
    {
        logger_m.infoStream() << "Disconnecting from MQTT broker" << endlog;
        mosquitto_loop_stop(mosq_m, true);
        mosquitto_disconnect(mosq_m);
        mosquitto_destroy(mosq_m);
        mosq_m = nullptr;
        connected_m = false;
    }
}

std::string MqttConnection::formatGroupAddress(eibaddr_t addr)
{
    // Use the same format as Object::WriteGroupAddr
    return Object::WriteGroupAddr(addr);
}

void MqttConnection::publishTelegram(const std::string& messageType, eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    if (!mosq_m || !connected_m)
    {
        logger_m.debugStream() << "MQTT not connected, skipping publish" << endlog;
        return;
    }

    // Format topic: prefix/gad
    std::string gad = formatGroupAddress(dest);
    std::string topic = prefix_m + "/" + gad;

    // Format payload as hex string of the raw telegram data
    std::ostringstream payload;
    payload << std::hex << std::setfill('0');
    for (int i = 0; i < len; i++)
    {
        if (i > 0)
            payload << " ";
        payload << std::setw(2) << static_cast<int>(buf[i]);
    }

    std::string payloadStr = payload.str();

    // Publish to MQTT
    int ret = mosquitto_publish(mosq_m, nullptr, topic.c_str(),
                                payloadStr.length(), payloadStr.c_str(),
                                0, false);

    if (ret != MOSQ_ERR_SUCCESS)
    {
        logger_m.errorStream() << "Failed to publish to MQTT topic " << topic
                               << ": " << mosquitto_strerror(ret) << endlog;
    }
    else
    {
        logger_m.debugStream() << "Published " << messageType << " to MQTT: topic="
                               << topic << ", payload=" << payloadStr << endlog;
    }
}

void MqttConnection::onWrite(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    logger_m.debugStream() << "KNX Write from " << Object::WriteAddr(src)
                          << " to " << formatGroupAddress(dest) << endlog;
    publishTelegram("Write", src, dest, buf, len);
}

void MqttConnection::onRead(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    logger_m.debugStream() << "KNX Read from " << Object::WriteAddr(src)
                          << " to " << formatGroupAddress(dest) << endlog;
    publishTelegram("Read", src, dest, buf, len);
}

void MqttConnection::onResponse(eibaddr_t src, eibaddr_t dest, const uint8_t* buf, int len)
{
    logger_m.debugStream() << "KNX Response from " << Object::WriteAddr(src)
                          << " to " << formatGroupAddress(dest) << endlog;
    publishTelegram("Response", src, dest, buf, len);
}

#endif // HAVE_MQTT