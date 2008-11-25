/*-
 *   Copyright (C) 2008 by gelraen
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
 *     * Neither the name of the gelraen nor the names of its contributors *
 *       may be used to endorse or promote products derived from this      *
 *       software without specific prior written permission.               *
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

#include <iostream>
#include <translator.h>
#include <config.h>
#include <dcclient.h>
#include <ircclient.h>
#include <sys/select.h>

using namespace std;

int main()
{
	Config conf;
	Translator trans;
	IRCClient irc;
	DCClient dc;
	
	// config options
	conf.m_irc_channel="#chat";
	conf.m_dc_server="dc";
	conf.m_irc_server="10.45.64.2";
	conf.m_irc_port=6667;
	
	if (!irc.setConfig(conf))
	{
		cerr << "Incorrect config for IRC" << endl;
		return 1;
	}
	if (!dc.setConfig(conf))
	{
		cerr << "Incorrect config for DC++" << endl;
		return 1;
	}

	
	string str;
	
	cerr << "Connecting to IRC... ";
	if (!irc.Connect())
	{
		cerr << "ERROR" << endl;
		return 2;
	} else cerr << "OK" << endl;
	
	cerr << "Connecting to DC++... ";
	if (!dc.Connect())
	{
		cerr << "ERROR" << endl;
		return 2;
	} else cerr << "OK" << endl;
	
	// now recreate config, cause DC hub may change our nickname
	conf=Config(irc.getConfig(),dc.getConfig());
	
	if (!trans.setConfig(conf))
	{
		cerr << "Incorrect config for Translator" << endl;
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
			if (!dc.isLoggedIn())
			{
				cerr << "DC++ connection closed. Exiting." << endl;
				return 0;
			}
		}
		
		while(dc.readCommand(str))
		{
			if (trans.DCtoIRC(str,str))
			{
				irc.writeCommand(str);
			}
			if (!irc.isLoggedIn())
			{
				cerr << "IRC connection closed. Exiting." << endl;
				return 0;
			}
		}
	}
	
	return 0;
}
