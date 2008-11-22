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

#include <dcconfig.h>
#include <dcclient.h>
#include <poll.h>
#include <sys/select.h>
#include <string>

using namespace std;


int main()
{
	DCClient cl;
	DCConfig conf;
	conf.m_dc_server="dc";
	if (!cl.setConfig(conf))
	{
		cerr << "Incorrect config" << endl;
		return 1;
	}
	
	cl.Connect();
	cerr << "Connected" << endl;
	string t;
	
	pollfd in;
	in.fd=0;
	in.events=POLLIN;
	
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(0,&fdset);
	int max=cl.FdSet(fdset)+1;
	
	for(;;)
	{
		while (cl.readMessage(t))
		{
			cout << t << endl;
		}
		
		if (!cl.isLoggedIn()) break;
		
		while(poll(&in,1,0))
		{
			getline(cin,t);
			cl.writeMessage(t);
		}
		
		if (!cl.isLoggedIn()) break;
		select(max,&fdset,NULL,NULL,NULL);
	}
	
	cl.Disconnect();
	return 0;
}
