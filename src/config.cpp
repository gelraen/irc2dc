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
#include "config.h"
#include "defs.h"
#include <fstream>
#include <string>
#include <sstream>

using namespace std;


Config::Config()
	: IRCConfig(), DCConfig()
{
}

Config::Config(const IRCConfig& c1,const DCConfig& c2)
	: IRCConfig(c1), DCConfig(c2)
{
}


Config::~Config()
{
}

struct confvar
{
	string name;
	char type; // 's','i'
	int Config::* np;
	string Config::* sp;
};

const confvar varlist[]={
 {"irc_server",		's',	NULL,	&Config::m_irc_server},
 {"irc_port",		'i',	&Config::m_irc_port,	NULL},
 {"irc_nick",		's',	NULL,	&Config::m_irc_nick},
 {"irc_username",	's',	NULL,	&Config::m_irc_username},
 {"irc_password",	's',	NULL,	&Config::m_irc_password},
 {"irc_realname",	's',	NULL,	&Config::m_irc_realname},
 {"irc_channel",	's',	NULL,	&Config::m_irc_channel},
 {"dc_server",		's',	NULL,	&Config::m_dc_server},
 {"dc_port",		'i',	&Config::m_dc_port,	NULL},
 {"dc_nick",		's',	NULL,	&Config::m_dc_nick},
 {"dc_pass",		's',	NULL,	&Config::m_dc_pass},
 {"dc_description",	's',	NULL,	&Config::m_dc_description},
 {"dc_speed",		's',	NULL,	&Config::m_dc_speed},
 {"dc_speed_val",	'i',	&Config::m_dc_speed_val,	NULL},
 {"dc_email",		's',	NULL,	&Config::m_dc_email},
 {"dc_share_size",	's',	NULL,	&Config::m_dc_share_size},
 {"logfile",		's',	NULL,	&Config::m_sLogFile},
 {"",				'\0',	NULL,	NULL} // terminator
};

/*!
    \fn Config::ReadFromFile(const string& sConfFile)
 */
bool Config::ReadFromFile(const string& sConfFile)
{
	string str;
	ifstream conf(sConfFile.c_str());
	
	if (conf.bad()) return false;
	
	while (!conf.eof())
	{
		getline(conf,str);
		
		string::size_type pos=0;
		str=trim(str);
		if ((pos=str.find(';'))!=string::npos)
		{
			str.erase(pos); // erase all after ';'
		}
		if (str == ""||(pos=str.find("="))==string::npos)
			continue;
		vars[trim(str.substr(0, pos))] =
				trim(trim(trim(str.substr(pos + 1)), '\''), '\"');
	};
	
	int i;
	istringstream s;
	for(i=0;varlist[i].type!='\0';i++)
	{
		if (vars.find(varlist[i].name)==vars.end()) continue;
		switch (varlist[i].type)
		{
			case 's':
				(this->*(varlist[i].sp))=vars[varlist[i].name];
				break;
			case 'i':
				s.str(vars[varlist[i].name]);
				s >> (this->*(varlist[i].np));
				break;
		}
	}

	return true;
}


/*!
    \fn Config::getLogFile()
 */
const string& Config::getLogFile()
{
	return m_sLogFile;
}
