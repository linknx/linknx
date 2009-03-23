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

#ifndef EMAILGATEWAY_H
#define EMAILGATEWAY_H

#include <string>
#include "config.h"
#include "logger.h"
#include "ticpp.h"

#ifdef HAVE_LIBESMTP
#include <auth-client.h>
#endif

class MessageBody
{
public:
    MessageBody(std::string& text);
    const char *getData(int *len);
private:
    std::string text_m;
    int status_m;
};

class EmailGateway
{
public:
    EmailGateway();
    ~EmailGateway();

    void importXml(ticpp::Element* pConfig);
    void exportXml(ticpp::Element* pConfig);

    void sendEmail(std::string &to, std::string &subject, std::string &text);

private:
    static const char *callback(void **buf, int *len, void *arg);

    enum EmailGatewayType
    {
        SMTP,
        Unknown
    };

    EmailGatewayType type_m;

    std::string host_m;
    std::string from_m;
    std::string login_m;
    std::string pass_m;

#ifdef HAVE_LIBESMTP
    static int authCallback(auth_client_request_t request, char **result, int fields, void *arg);
#endif

    static Logger& logger_m;
};

#endif
