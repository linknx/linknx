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

#include "smsgateway.h"
#include <iostream>
#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif

Logger& SmsGateway::logger_m(Logger::getInstance("SmsGateway"));

SmsGateway::SmsGateway() : type_m(Unknown)
{}

SmsGateway::~SmsGateway()
{}

void SmsGateway::importXml(ticpp::Element* pConfig)
{
    std::string type = pConfig->GetAttribute("type");
    if (type == "clickatell")
    {
#ifdef HAVE_LIBCURL
        type_m = Clickatell;
        pConfig->GetAttribute("user", &user_m);
        pConfig->GetAttribute("pass", &pass_m);
        pConfig->GetAttribute("api_id", &data_m);
        from_m = pConfig->GetAttribute("from");
# else
        std::stringstream msg;
        msg << "SmsGateway: Gateway type 'clickatell' not supported, libcurl not available" << std::endl;
        throw ticpp::Exception(msg.str());
#endif
    }
    else if (type == "")
    {
        type_m = Unknown;
        user_m.clear();
        pass_m.clear();
        data_m.clear();
    }
    else
    {
        std::stringstream msg;
        msg << "SmsGateway: Bad gateway type: '" << type << "'" << std::endl;
        throw ticpp::Exception(msg.str());
    }
}

void SmsGateway::exportXml(ticpp::Element* pConfig)
{
    if (type_m == Clickatell)
    {
        pConfig->SetAttribute("type", "clickatell");
        pConfig->SetAttribute("user", user_m);
        pConfig->SetAttribute("pass", pass_m);
        pConfig->SetAttribute("api_id", data_m);
        if (!from_m.empty())
            pConfig->SetAttribute("from", from_m);
    }
}

void SmsGateway::sendSms(std::string &id, std::string &value)
{
    if (type_m == Clickatell)
    {
#ifdef HAVE_LIBCURL
        CURL *curl;
        CURLcode res;

        curl = curl_easy_init();
        if(curl)
        {
            char *escaped_value = curl_escape(value.c_str(), value.length());
            std::stringstream msg;
            msg << "http://api.clickatell.com/http/sendmsg?user=" << user_m
            << "&password=" << pass_m
            << "&api_id=" << data_m;
            if (!from_m.empty())
                msg << "&from=" << from_m;
            msg << "&to=" << id << "&text=" << escaped_value;
            std::string url = msg.str();
            curl_free(escaped_value);
            //        curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            res = curl_easy_perform(curl);

            logger_m.infoStream() << "curl_easy_perform returned: " << res << endlog;
            if (res != 0)
                logger_m.infoStream() << "msg=" << curl_easy_strerror(res) << endlog;

            curl_easy_cleanup(curl);
        }
        else
            logger_m.errorStream() << "Unable to execute SendSmsAction. Curl not available" << endlog;
#endif
    }
    else
        logger_m.errorStream() << "Unable to send SMS, gateway not set." << endlog;
}
