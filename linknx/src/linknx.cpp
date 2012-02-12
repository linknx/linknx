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

#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <pthsem.h>
#include "config.h"
#include "ticpp.h"
#include "eibclient.h"
#include "objectcontroller.h"
#include "ruleserver.h"
#include "services.h"
#include "timermanager.h"
#include "xmlserver.h"
#include "smsgateway.h"

/** structure to store the arguments */
struct arguments
{
    /** path for config file */
    const char *configfile;
    /** path where to write config file */
    const char *writeconfig;
    /** path to pid file */
    const char *pidfile;
    /** path to trace log file */
    const char *daemon;
};
/** storage for the arguments*/
struct arguments arg;

/** version */
const char *argp_program_version = PACKAGE_STRING
#ifdef HAVE_LIBCURL
    "\n- Clickatell SMS gateway enabled"
#endif
#ifdef HAVE_LIBESMTP
    "\n- E-mail gateway enabled"
#ifdef HAVE_LIBESMTP_PTHREAD
    " (with pthread support)"
#endif
#endif
#ifdef HAVE_MYSQL
    "\n- MySQL support enabled"
#endif
#ifdef HAVE_LUA
    "\n- LUA scripting support enabled"
#endif
#ifdef HAVE_LOG4CPP
    "\n- Log4cpp logging enabled"
#endif
;

/** documentation */
static char doc[] =
    "LinKNX -- KNX home automation platform\n"
    "(C) 2007-2008 Jean-François Meessen <linknx@ouaye.net>\n";

/** documentation for arguments*/
static char args_doc[] = "";

/** option list */
static struct argp_option options[] =
    {
        {"config", 'c', "FILE", OPTION_ARG_OPTIONAL,
            "read configuration from file (default: /var/lib/linknx/linknx.xml)"
        },
        {"write", 'w', "FILE", OPTION_ARG_OPTIONAL,
            "write configuration to file (if no FILE specified, the config file is overwritten)"
        },
        {"pid-file", 'p', "FILE", 0, "write the PID of the process to FILE"},
        {"daemon", 'd', "FILE", OPTION_ARG_OPTIONAL,
         "start the program as daemon, the output will be written to FILE, if the argument present"},
        {0}
    };

/** parses and stores an option */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = (struct arguments *) state->input;
    switch (key)
    {
    case 'c':
        arguments->configfile = (char *) (arg ? arg : "/var/lib/linknx/linknx.xml");
        break;
    case 'w':
        arguments->writeconfig = (char *) (arg ? arg : "");
        break;
    case 'p':
        arguments->pidfile = arg;
        break;
    case 'd':
        arguments->daemon = (char *) (arg ? arg : "/dev/null");
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/** information for the argument parser*/
static struct argp argp = { options, parse_opt, args_doc, doc };

void die (const char *msg, ...)
{
    va_list ap;
    va_start (ap, msg);
    vprintf (msg, ap);
    va_end (ap);
    if (errno)
        printf (": %s\n", strerror (errno));
    else
        printf ("\n", strerror (errno));
    exit (1);
}

int
main (int ac, char *ag[])
{
    int index;

    memset (&arg, 0, sizeof (arg));

    argp_parse (&argp, ac, ag, 0, &index, &arg);
    if (index < ac)
        die ("unexpected parameter: %s\n", ag[index]);

    signal (SIGPIPE, SIG_IGN);
    pth_init ();

    if (arg.daemon)
    {
        int fd = open (arg.daemon, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd == -1)
            die ("Can not open file %s", arg.daemon);
        int i = fork ();
        if (i < 0)
            die ("fork failed");
        if (i > 0)
            exit (0);
        close (1);
        close (2);
        close (0);
        dup2 (fd, 1);
        dup2 (fd, 2);
        setsid ();
    }

    Logger& logger = Logger::getInstance("main");

    FILE *pidf;
    if (arg.pidfile)
        if ((pidf = fopen (arg.pidfile, "w")) != NULL)
        {
            fprintf (pidf, "%d", getpid ());
            fclose (pidf);
        }

    Logging* logging = Logging::instance();
    RuleServer* rules = RuleServer::instance();
    ObjectController* objects = ObjectController::instance();
    Services* services = Services::instance();
    if (arg.configfile)
    {
        ticpp::Document doc;
        ticpp::Element* pConfig = NULL;
        try
        {
            // Load a document
            doc.LoadFile(arg.configfile);

            pConfig = doc.FirstChildElement("config");

            ticpp::Element* pLogging = pConfig->FirstChildElement("logging", false);
            logging->importXml(pLogging);
            logger.debugStream() << "Logging configured" << endlog;
        }
        catch( ticpp::Exception& ex )
        {
            logging->defaultConfig();
            logger.errorStream() << "Unable to load config: " << ex.m_details << endlog;
            die ("Unable to load config");
        }

        try
        {
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
        catch( ticpp::Exception& ex )
        {
            logger.errorStream() << "Error in config: " << ex.m_details << endlog;
            die ("Error in config");
        }
        if (arg.writeconfig && arg.writeconfig[0] == 0)
            arg.writeconfig = arg.configfile;
        logger.infoStream() << "Config file loaded: " << arg.configfile << endlog;
    }
    else 
    {
        try
        {
            logging->defaultConfig();
            logger.infoStream() << "No config file, using default values" << endlog;
            services->createDefault();
        }
        catch( ticpp::Exception& ex )
        {
            logger.errorStream() << "Error while loading default configuration: " << ex.m_details << endlog;
            die ("Error in config");
        }
    }
    sigset_t t1;
    sigemptyset (&t1);
    sigaddset (&t1, SIGINT);
    sigaddset (&t1, SIGTERM);
    signal (SIGINT, SIG_IGN);
    signal (SIGTERM, SIG_IGN);

    services->setConfigFile(arg.writeconfig);
    services->getKnxConnection()->addTelegramListener(objects);
    services->start();
    int x;
    pth_sigwait (&t1, &x);

    logger.debugStream() << "Signal received, terminating" << endlog;

    signal (SIGINT, SIG_DFL);
    signal (SIGTERM, SIG_DFL);

    if (arg.pidfile)
        unlink (arg.pidfile);

    Services::reset();
    logger.debugStream() << "Services reset" << endlog;
    RuleServer::reset();
    logger.debugStream() << "RuleServer reset" << endlog;
    ObjectController::reset();
    logger.debugStream() << "ObjectController reset" << endlog;

    pth_exit (0);
    return 0;
}
