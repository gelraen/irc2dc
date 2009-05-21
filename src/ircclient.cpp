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
#include "ircclient.h"
#include "defs.h"
#include <netdb.h>
#include <netinet/in.h>
#ifdef WITH_IPv6
#	include <netinet6/in6.h>
#endif

#include <iostream>
#include <vector>
#include <string.h>

using namespace std;

IRCClient::IRCClient()
{
	m_bLoggedIn=false;
}


IRCClient::~IRCClient()
{
	Disconnect();
}




/*!
    \fn IRCClient::writeCommand(const string& str)
 */
bool IRCClient::writeCommand(const string& str)
{
	if (!m_bLoggedIn) return false;
	bool r=m_connection.WriteCmdAsync(str);
	if (!m_connection.isConnected()) m_bLoggedIn=false;
	return r;
}


/*!
    \fn IRCClient::Connect()
 */
bool IRCClient::Connect()
{
    /// @todo implement me
	/**
	 * 1) resolve hostname into IPv4/IPv6 address (gethostbyname(3))
	 * 2) call m_connection.Connect()
	 * 3) start usual registration procedure
	 * 3.1) send PASS, if needed
	 * 3.2) send NICK
	 * 3.3) send USER
	 * 3.4) send JOIN
	 */
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
#ifdef WITH_IPv6
	sockaddr_in6 addr6;
	memset(&addr6,0,sizeof(addr6));
#endif
	
	hostent* p;
	p=gethostbyname(m_config.m_irc_server.c_str());
	if (p==NULL)
	{
		LOG(log::error, "Can not resolve name \"" + m_config.m_irc_server + "\"");
		LOG(log::error, string("IRCClient::Connect(): gethostbyname(3) failed: ")+hstrerror(h_errno));
		return false;
	}
	switch(p->h_addrtype)
	{
		case AF_INET:
			//addr.sin_len=sizeof(addr);
			addr.sin_family=AF_INET;
			addr.sin_port=htons((unsigned short)m_config.m_irc_port);
			addr.sin_addr= *((in_addr*)(p->h_addr_list[0]));
			if (m_connection.Connect((sockaddr*)&addr,sizeof(addr)))
			{
				cerr << "IRCClient::Connect(): cann't connect to requested address"
						<< endl;
				return false;
			}
			break;
#ifdef WITH_IPv6
		case AF_INET6:
			addr6.sin_len=sizeof(addr6);
			addr6.sin_family=AF_INET6;
			addr6.sin_port=htons((unsigned short)m_config.m_irc_port);
			addr6.sin_addr= *((in6_addr*)(p->h_addr_list[0]));
			if (m_connection.Connect((sockaddr*)&addr6,sizeof(addr6)))
			{
				cerr << "IRCClient::Connect(): cann't connect to requested address"
						<< endl;
				return false;
			}
			break;
#endif
		default:
			cerr << "IRCClient::Connect(): unknown address family" << endl;
			return false;
	}
	
	// now we connected to server, starting registration
	string cmd;

	if (!m_config.m_irc_password.empty())
	{
		m_connection.WriteCmdSync(string("PASS ")+m_config.m_irc_password);
		if (!m_connection.isConnected()) return false;
	}
	
	m_connection.WriteCmdSync(string("NICK ")+m_config.m_irc_nick);
	if (!m_connection.isConnected()) return false;
	
	m_connection.WriteCmdSync(string("USER ")+m_config.m_irc_username+
							 string(" 0 * :")+m_config.m_irc_realname);
	if (!m_connection.isConnected()) return false;
	
	m_connection.WriteCmdSync(string("JOIN ")+m_config.m_irc_channel);
	if (!m_connection.isConnected()) return false;
	
	m_bLoggedIn=true;
	
	return true;
}


/*!
    \fn IRCClient::Disconnect()
 */
bool IRCClient::Disconnect()
{
	if (!m_bLoggedIn) return false;
	m_connection.WriteCmdSync(string("QUIT"));
	m_connection.Close();
	m_bLoggedIn=false;
}


/*!
    \fn IRCClient::readCommand(string& str)

	This function will also answer to PINGs
 */
bool IRCClient::readCommand(string& str)
{
	if (!m_bLoggedIn) return false;
	bool r=m_connection.ReadCmdAsync(str);
	if (!r) return false;
	
	// answer to PINGs
	if (str.length()>=string("PING").length()
			&&
		   str.substr(0,string("PING").length())==string("PING"))
	{
		m_connection.WriteCmdAsync(string("PONG ")+
					str.substr(string("PING ").length(),string::npos));
		r=m_connection.ReadCmdAsync(str);
	}
		
	if (!m_connection.isConnected()) m_bLoggedIn=false;
	return r;
}


/*!
    \fn IRCClient::setConfig(const IRCConfig& conf)
 */
bool IRCClient::setConfig(const IRCConfig& conf)
{
	// validate config
	if (conf.m_irc_nick.empty()) return false;
	if (conf.m_irc_server.empty()) return false;
	if (conf.m_irc_username.empty()) return false;
	if (conf.m_irc_channel.empty()) return false;
	if (conf.m_irc_port==0) return false;
	
	m_config=conf;
	return true;
}


/*!
    \fn IRCClient::getConfig()
 */
const IRCConfig& IRCClient::getConfig()
{
	return m_config;
}


/*!
    \fn IRCClient::isLoggedIn() const
 */
bool IRCClient::isLoggedIn() const
{
	return m_bLoggedIn;
}


/*!
    \fn IRCClient::FdSet(fd_set& fdset)
 */
int IRCClient::FdSet(fd_set& fdset)
{
	return m_connection.FdSet(fdset);
}


/*!
    \fn IRCClient::writeMessage(const string& str)
 */
bool IRCClient::writeMessage(const string& str)
{
	return writeCommand(string("PRIVMSG ")+
						m_config.m_irc_channel+
						string(" :")+str);
}


