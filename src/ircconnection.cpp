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

IRCConnection::IRCConnection()
 : Connection()
{
}


IRCConnection::~IRCConnection()
{
}

/*!
    \fn IRCConnection::WriteCmd(const string& str)
 */
bool IRCConnection::WriteCmd(const string& s)
{
	// not sure is it needed. We may place data in buffer even if not connected
	if (!m_bConnected) return false;
	
	// 1) cut command to 510 bytes
	// 2) put it in m_sendbuf follower by "\r\n"
	string str=s;
	if (str.length()>510)
	{
		str.erase(510,string::npos);
	}
	
	m_sendbuf+=str;
	m_sendbuf+="\r\n";
	return true;
}


/*!
    \fn IRCConnection::ReadCmd(string& str)
 */
bool IRCConnection::ReadCmd(string& str)
{
	if (!m_bConnected || m_recvbuf.empty()) return false;
	
	string::size_type pos;
	pos=m_recvbuf.find("\r\n");
	if (pos==string::npos) return false;
	str=m_recvbuf.substr(0,pos);
	m_recvbuf.erase(0,pos+2);
	return true;
}
