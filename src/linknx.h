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

#ifndef IOPORT_H
#define IOPORT_H

#include "config.h"
#include "logger.h"
#include "threads.h"
#include <string>
#include "ticpp.h"
#include "ruleserver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <regex.h>

/** Represents the whole program.
 * This class is aimed at becoming a singleton that hosts the other singletons.
 * But this requires further refactoring that cannot be done until the current
 * codebase has a stronger unit test coverage. So for now, use only static
 * methods. */
class Linknx
{
	public:
		static void LoadConfiguration(const std::string &file);
		static void ParseConfiguration(const std::string &configString);

	private:
		static void LoadConfiguration(ticpp::Document &doc, Logger &logger);
};

#endif
