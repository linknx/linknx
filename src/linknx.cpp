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

// #include <argp.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdarg.h>
// #include <signal.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <cstring>
// #include <pthsem.h>
// #include "config.h"
// #include "ticpp.h"
// #include "eibclient.h"
// #include "objectcontroller.h"
// #include "ruleserver.h"
#include "services.h"
// #include "timermanager.h"
// #include "xmlserver.h"
// #include "smsgateway.h"
#include "linknx.h"

void Linknx::LoadConfiguration(const std::string &file)
{
	ticpp::Document doc;
	doc.LoadFile(file);
    Logger& logger = Logger::getInstance("main");
	LoadConfiguration(doc, logger);
	logger.infoStream() << "Config file loaded: " << file << endlog;
}

void Linknx::ParseConfiguration(const std::string &configString)
{
	ticpp::Document doc;
	doc.LoadFromString(configString);
    Logger& logger = Logger::getInstance("main");
	LoadConfiguration(doc, logger);
	logger.infoStream() << "Config string loaded." << endlog;
}

void Linknx::LoadConfiguration(ticpp::Document &doc, Logger &logger)
{
   	ticpp::Element* pConfig = NULL;
    Logging* logging = Logging::instance();
    RuleServer* rules = RuleServer::instance();
    ObjectController* objects = ObjectController::instance();
    Services* services = Services::instance();

	pConfig = doc.FirstChildElement("config");

	ticpp::Element* pLogging = pConfig->FirstChildElement("logging", false);
	logging->importXml(pLogging);
	logger.debugStream() << "Logging configured" << endlog;

	ticpp::Element* pServices = pConfig->FirstChildElement("services", false);
	if (pServices != NULL)
		services->importXml(pServices);
	logger.debugStream() << "Services loaded" << endlog;
	ticpp::Element* pObjects = pConfig->FirstChildElement("objects", false);
	if (pObjects != NULL)
		objects->importXml(pObjects);
	logger.debugStream() << "Objects loaded" << endlog;
	ticpp::Element* pRules = pConfig->FirstChildElement("rules", false);
	if (pRules != NULL)
		rules->importXml(pRules);
	logger.debugStream() << "Rules loaded" << endlog;
}
