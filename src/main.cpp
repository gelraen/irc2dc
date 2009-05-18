/*-
 *   Copyright (C) 2008-2009 by Maxim Ignatenko
 *   gelraen.ua@gmail.com
 *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *    modification, are permitted provided that the following conditions   *
 *    are met:                                                             *
 *     * Redistributions of source code must retain the above copyright    *
 *       notice, this list of conditions and the following disclaimer.     *
 *     * Redistributions in binary form must reproduce the above copyright *
 *       notice, this list of conditions and the following disclaimer in   *
 *       the documentation and/or other materials provided with the        *
 *       distribution.                                                     *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR *
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      *
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, *
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY *
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   *
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE *
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  *
 *
 *  $Id$
 */

#include "defs.h"

#include <iostream>
#include <translator.h>
#include <config.h>
#include <dcclient.h>
#include <ircclient.h>
#include <sys/select.h>
#include <unistd.h>
#include "../config.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>

using namespace std;

void usage();
void version();
bool writepid(const string& pidfile);

int main(int argc,char *argv[])
{
	Config conf;
	Translator trans;
	IRCClient irc;
	DCClient dc;
	LogLevel=0xffffffff; // log all
	LogLevel&=(~LOG_RAWDATA); // except raw data
	
	string conffile=CONFFILE;
	string pidfile;
	bool daemonize=true;
	string logfile;
	
	if (argc==1)
	{
		usage();
		return 0;
	}
	
	char ch;
	while((ch = getopt(argc, argv, "dc:l:p:h")) != -1) {
		switch (ch) {
			case 'd':
				daemonize=false;
				break;
			case 'c':
				conffile=optarg;
				break;
			case 'l':
				logfile=optarg;
				break;
			case 'p':
				pidfile=optarg;
				break;
			case 'h':
			case '?':
			default:
				usage();
				return(0);
		}
	}

	if (!conf.ReadFromFile(conffile))
	{
		cerr << "Failed to load config. Exiting." << endl;
		return 1;
	}
	
	if (!logfile.empty())
	{
		conf.m_sLogFile=logfile;
	}
	
	if (!pidfile.empty())
	{
		conf.m_pidfile=pidfile;
	}

	if (daemonize)
	{
		if (daemon(0,0)==-1) // chdir to '/' and close fd's 0-2
		{
			perror("daemon");
			return 1;
		}

		// init logging here (maybe just open file at fd 2)
	}

	if (!conf.m_pidfile.empty()&&!writepid(conf.m_pidfile))
	{
		LOG(LOG_ERROR,"Unable to write pidfile");
	}

	if (!irc.setConfig(conf))
	{
		LOG(LOG_ERROR,"Incorrect config for IRC");
		return 1;
	}
	if (!dc.setConfig(conf))
	{
		LOG(LOG_ERROR,"Incorrect config for DC++");
		return 1;
	}

	string str;
	
	LOG(LOG_NOTICE,"Connecting to IRC... ");
	if (!irc.Connect())
	{
		LOG(LOG_ERROR,"ERROR");
		return 2;
	} else LOG(LOG_NOTICE,"OK");

	LOG(LOG_NOTICE,"Connecting to DC++... ");
	if (!dc.Connect())
	{
		LOG(LOG_ERROR,"ERROR");
		return 2;
	} else LOG(LOG_NOTICE,"OK");

	// now recreate config, cause DC hub may change our nickname
	conf=Config(irc.getConfig(),dc.getConfig());
	
	if (!trans.setConfig(conf))
	{
		LOG(LOG_ERROR,"Incorrect config for Translator");
		return 1;
	}

	fd_set rset;
	int max=0,t;

	for(;;)
	{
		// recreate rset
		FD_ZERO(&rset);
		
		t=irc.FdSet(rset);
		max=(t>max)?t:max;
		
		t=dc.FdSet(rset);
		max=(t>max)?t:max;
		
		select(max+1,&rset,NULL,NULL,NULL);
		
		while(irc.readCommand(str))
		{
			if (trans.IRCtoDC(str,str))
			{
				dc.writeCommand(str);
			}
		}
		if (!dc.isLoggedIn())
		{
			LOG(LOG_WARNING,"DC++ connection closed. Trying to reconnect...");
			dc.Connect();
		}
		
		while(dc.readCommand(str))
		{
			if (trans.DCtoIRC(str,str))
			{
				irc.writeCommand(str);
			}
		}
		if (!irc.isLoggedIn())
		{
			LOG(LOG_WARNING,"IRC connection closed. Trying to reconnect...");
			irc.Connect();
		}
	}

	return 0;
}

void usage()
{
	version();
	cout << endl;
	cout << "Options:" << endl;
	cout << "   -h      - show this help message" << endl;
	cout << "   -c file - specify path to config file" << endl;
	cout << "   -l file - override path to logfile specified in config" << endl;
	cout << "   -p file - override path to pidfile specified in config" << endl;
	cout << "   -d      - do not go in background, all logging goes to stderr" << endl;
	cout << endl;
}

void version()
{
	cout << PACKAGE << " " << VERSION << endl;
}

bool writepid(const string& pidfile)
{
	ofstream pid(pidfile.c_str());
	if (pid.bad()) return false;
	pid << getpid() << endl;
	return true;
}
