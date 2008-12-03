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
#include "ircconnection.h"
#include "defs.h"

IRCConnection::IRCConnection()
 : Connection()
{
}


IRCConnection::~IRCConnection()
{
}

/*!
    \fn IRCConnection::WriteCmdAsync(const string& str)
 */
bool IRCConnection::WriteCmdAsync(const string& s)
{
	// not sure is it needed. We may place data in buffer even if not connected
	if (!isConnected()) return false;
	
	// 1) replace all '\r' and '\n' with spaces
	// 2) cut command to 510 bytes
	// 3) put it in m_sendbuf follower by "\r\n"
	string str=s;
	string::size_type pos=0;
	while((pos=str.find_first_of("\r\n",pos))!=string::npos)
	{
		str.replace(pos,1,1,' ');
	}
	if (str.length()>510)
	{
		str.erase(510,string::npos);
	}
	
	m_sendbuf+=str;
	LOG(LOG_COMMAND,string("to ")+int2str(m_socket)+string(" > ")+str);
	m_sendbuf+="\r\n";
	
	_write();
	
	return true;
}


/*!
    \fn IRCConnection::ReadCmdAsync(string& str)
 */
bool IRCConnection::ReadCmdAsync(string& str)
{
	if (!isConnected()) return false;
	
	_read(); // check for new data from server
	
	if (m_recvbuf.empty()) return false;
	
	string::size_type pos;
	pos=m_recvbuf.find("\r\n");
	if (pos==string::npos) return false;
	str=m_recvbuf.substr(0,pos);
	LOG(LOG_COMMAND,string("from ")+int2str(m_socket)+string(" > ")+str);
	m_recvbuf.erase(0,pos+2);
	return true;
}
