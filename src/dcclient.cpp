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
#include "dcclient.h"
#include <netdb.h>
#include <netinet/in.h>
#ifdef WITH_IPv6
#	include <netinet6/in6.h>
#endif

#include <iostream>
#include <vector>
using namespace std;

DCClient::DCClient()
{
	m_bLoggedIn=false;
}


DCClient::~DCClient()
{
	Disconnect();
}




/*!
    \fn DCClient::writeCommand(const string& str)
 */
bool DCClient::writeCommand(const string& str)
{
	if (!m_bLoggedIn) return false;
	bool r=m_connection.WriteCmdAsync(str);
	if (!m_connection.isConnected()) m_bLoggedIn=false;
	return r;
}


/*!
    \fn DCClient::Connect()
 */
bool DCClient::Connect()
{
    /// @todo implement me
	/**
	* 1) resolve hostname into IPv4/IPv6 address (gethostbyname(3))
	* 2) call m_connection.Connect()
	* 3) start usual registration procedure
	* 3.1) recieve Lock and decode it into Key
	* 3.2) send Key
	* 3.3) validate Nick
	* 3.4) send Password, if requested
	*/
	sockaddr_in addr;
	bzero(&addr,sizeof(addr));
#ifdef WITH_IPv6
	sockaddr_in6 addr6;
	bzero(&addr6,sizeof(addr6));
#endif
	
	hostent* p;
	p=gethostbyname(m_config.m_dc_server.c_str());
	if (p==NULL)
	{
		cerr << "Can not resolve name \"" << m_config.m_dc_server  << "\""
				<< endl;
		herror("DCClient::Connect(): gethostbyname(3) failed");
		return false;
	}
	switch(p->h_addrtype)
	{
	case AF_INET:
		addr.sin_len=sizeof(addr);
		addr.sin_family=AF_INET;
		addr.sin_port=htons((unsigned short)m_config.m_dc_port);
		addr.sin_addr= *((in_addr*)(p->h_addr_list[0]));
		if (m_connection.Connect((sockaddr*)&addr,sizeof(addr)))
		{
			cerr << "DCClient::Connect(): cann't connect to requested address"
					<< endl;
			return false;
		}
		break;
#ifdef WITH_IPv6
	case AF_INET6:
		addr6.sin_len=sizeof(addr6);
		addr6.sin_family=AF_INET6;
		addr6.sin_port=htons((unsigned short)m_config.m_dc_port);
		addr6.sin_addr= *((in6_addr*)(p->h_addr_list[0]));
		if (m_connection.Connect((sockaddr*)&addr6,sizeof(addr6)))
		{
			cerr << "DCClient::Connect(): cann't connect to requested address"
					<< endl;
			return false;
		}
		break;
#endif
	default:
		cerr << "DCClient::Connect(): unknown address family" << endl;
		return false;
	}
	
	// now we connected to server, starting registration
	string cmd;
	
	// get Lock
	for(;;)
	{
		m_connection.ReadCmdSync(cmd);
		if (cmd.length()>=string("$Lock ").length()
				  &&
				  cmd.substr(0,string("$Lock ").length())==string("$Lock "))
		{
			break;
		}
		if (!m_connection.isConnected()) return false;
	}
	
	// erase "$Lock "
	cmd.erase(0,string("$Lock ").length());
	
	// erase all after first space
	string::size_type pos;
	pos=cmd.find(' ');
	if (pos!=string::npos)
	{
		cmd.erase(pos,string::npos);
	}
	
	m_connection.WriteCmdSync(string("$Key ")+DecodeLock(cmd));
	if (!m_connection.isConnected()) return false;
	
	// read all other unneeded commands from server
	while(m_connection.ReadCmdAsync(cmd));
	if (!m_connection.isConnected()) return false;
	
	//starting nick validation
	for(;;)
	{
		m_connection.WriteCmdSync(string("$ValidateNick ")+m_config.m_dc_nick);
		if (!m_connection.isConnected()) return false;
		m_connection.ReadCmdSync(cmd);
		if (!m_connection.isConnected()) return false;
		if (cmd==string("$ValidateDenide"))
		{
			cerr << "Nickname \"" << m_config.m_dc_nick << "\" already taken. ";
			m_config.m_dc_nick+="_";
			cerr << "Trying \"" << m_config.m_dc_nick << "\"..." << endl;
		}
		else
		{
			break;
		}
	}
	
	// Nick valid, so next command may be "$GetPass" or "$Hello"
	if (cmd==string("$GetPass"))
	{
		m_connection.WriteCmdSync(string("$MyPass ")+m_config.m_dc_pass);
		m_connection.ReadCmdSync(cmd);
		if (cmd==string("$BadPass"))
		{
			cerr << "Hub said that our password is incorrect..." << endl;
			m_connection.Close();
			return false;
		}
	}

	while (!(cmd.length()>=string("$Hello ").length()
		   &&
		   cmd.substr(0,string("$Hello ").length())==string("$Hello ")))
	{
		m_connection.ReadCmdSync(cmd);
	}
	if (!m_connection.isConnected()) return false;
	
	cmd.erase(0,string("$Hello ").length());
	m_config.m_dc_nick=cmd; // new nickname sent in "$Hello" command
	
	m_connection.WriteCmdAsync("$Version 1,0096");
	if (!m_connection.isConnected()) return false;
	m_connection.WriteCmdSync(string("$MyINFO $ALL ")+
							 m_config.m_dc_nick+
							 string(" ")+
							 m_config.m_dc_description+
							 string("$ $")+
							 m_config.m_dc_speed+
							 (char)m_config.m_dc_speed_val+
							 string("$")+
							 m_config.m_dc_email+
							 string("$")+
							 m_config.m_dc_share_size+
							 string("$"));
	if (!m_connection.isConnected()) return false;
	m_bLoggedIn=true;
	
	return true;
}


/*!
    \fn DCClient::Disconnect()
 */
bool DCClient::Disconnect()
{
	if (!m_bLoggedIn) return false;
	m_connection.WriteCmdSync(string("$Quit ")+m_config.m_dc_nick);
    m_connection.Close();
	m_bLoggedIn=false;
}


/*!
    \fn DCClient::readCommand(string& str)
 */
bool DCClient::readCommand(string& str)
{
	if (!m_bLoggedIn) return false;
	bool r=m_connection.ReadCmdAsync(str);
	if (!m_connection.isConnected()) m_bLoggedIn=false;
	return r;
}


/*!
    \fn DCClient::setConfig(const DCConfig& conf)
 */
bool DCClient::setConfig(const DCConfig& conf)
{
	// validate config
	if (conf.m_dc_nick.empty()) return false;
	if (conf.m_dc_server.empty()) return false;
	if (conf.m_dc_speed.empty()) return false;
	if (conf.m_dc_port==0) return false;
	
	m_config=conf;
	return true;
}


/*!
    \fn DCClient::getConfig()
 */
const DCConfig& DCClient::getConfig()
{
    return m_config;
}


/*!
    \fn DCClient::DecodeLock(string lock)
 */
string DCClient::DecodeLock(string lock)
{
	/*
	 * algorithm was taken from here:
	 * http://www.teamfair.info/DC-Protocol.htm#AppendixA
	*/
	string r(lock.length(),' ');
	string::size_type i;

	r[0]=lock[0]^lock[lock.length()-2]^lock[lock.length()-1]^5;
	for(i=1;i<r.length();i++)
	{
		r[i]=lock[i]^lock[i-1];
	}
	
	for(i=0;i<r.length();i++)
	{
		r[i]= (((r[i]<<4)&0xf0)|((r[i]>>4)&0x0f));
	}
	
	vector<char> specChars;
	vector<string> replaceWith;
	specChars.push_back(0);		replaceWith.push_back("/%DCN000%/");
	specChars.push_back(5);		replaceWith.push_back("/%DCN005%/");
	specChars.push_back(36);	replaceWith.push_back("/%DCN036%/");
	specChars.push_back(96);	replaceWith.push_back("/%DCN096%/");
	specChars.push_back(124);	replaceWith.push_back("/%DCN124%/");
	specChars.push_back(126);	replaceWith.push_back("/%DCN126%/");

	int j;
	for(j=0;j<specChars.size();j++)
	{
		while ((i=r.find(specChars[j]))!=string::npos)
		{
			r.replace(i,1,replaceWith[j]);
		}
	}
	
	return r;
}


/*!
    \fn DCClient::isLoggedIn() const
 */
bool DCClient::isLoggedIn() const
{
	return m_bLoggedIn;
}


/*!
    \fn DCClient::FdSet(fd_set& fdset)
 */
int DCClient::FdSet(fd_set& fdset)
{
	return m_connection.FdSet(fdset);
}


/*!
    \fn DCClient::writeMessage(const string& str)
 */
bool DCClient::writeMessage(const string& str)
{
	return writeCommand(string("<")+m_config.m_dc_nick+string("> ")+str);
}
