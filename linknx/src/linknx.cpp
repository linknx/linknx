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

extern "C"
{
#include "common.h"
}

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

int
main (int ac, char *ag[])
{
    int index;

    memset (&arg, 0, sizeof (arg));

    argp_parse (&argp, ac, ag, 0, &index, &arg);
    if (index < ac)
        die ("unexpected parameter");

    signal (SIGPIPE, SIG_IGN);
    pth_init ();

    if (arg.daemon)
    {
        int fd = open (arg.daemon, O_WRONLY | O_APPEND | O_CREAT);
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

    initLogging();

    FILE *pidf;
    if (arg.pidfile)
        if ((pidf = fopen (arg.pidfile, "w")) != NULL)
        {
            fprintf (pidf, "%d", getpid ());
            fclose (pidf);
        }

    RuleServer* rules = RuleServer::instance();
    ObjectController* objects = ObjectController::instance();
    Services* services = Services::instance();
    if (arg.configfile)
    {
        try
        {
            // Load a document
            ticpp::Document doc(arg.configfile);
            doc.LoadFile();

            ticpp::Element* pConfig = doc.FirstChildElement("config");

            ticpp::Element* pServices = pConfig->FirstChildElement("services", false);
            if (pServices != NULL)
                services->importXml(pServices);
            ticpp::Element* pObjects = pConfig->FirstChildElement("objects", false);
            if (pObjects != NULL)
                objects->importXml(pObjects);
            ticpp::Element* pRules = pConfig->FirstChildElement("rules", false);
            if (pRules != NULL)
                rules->importXml(pRules);
        }
        catch( ticpp::Exception& ex )
        {
            // If any function has an error, execution will enter here.
            // Report the error
            Logger::getInstance("main").errorStream() << "unable to load config: " << ex.m_details << endlog;
            die ("initialisation failed");
        }
        if (arg.writeconfig && arg.writeconfig[0] == 0)
            arg.writeconfig = arg.configfile;
    }
    else 
    {
        services->createDefault();
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

    signal (SIGINT, SIG_DFL);
    signal (SIGTERM, SIG_DFL);

    if (arg.pidfile)
        unlink (arg.pidfile);

    Services::reset();
    RuleServer::reset();
    ObjectController::reset();

    pth_exit (0);
    return 0;
}
