/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * CHANGELOG: this file has been modified by Sergio Perez Alca�iz <serpeal@upvnet.upv.es> 
 *            Departamento de Inform�tica de Sistemas y Computadores          
 *            Universidad Polit�cnica de Valencia                             
 *            Valencia (Spain)    
 *            Date: April 2003                                          
 *
 */

#include "ares_private.h"

int ares_fds(ares_channel channel, fd_set *read_fds, fd_set *write_fds)
{
  struct server_state *server;
  int i, nfds;

  /* No queries, no file descriptors. */
  if (!channel->queries)
    return 0;

  nfds = 0;
  for (i = 0; i < channel->nservers; i++)
    {
      server = &channel->servers[i];
      if (server->udp_socket != -1)
	{
	  FD_SET(server->udp_socket, read_fds);
	  if (server->udp_socket >= nfds)
	    nfds = server->udp_socket + 1;
	}
      if (server->tcp_socket != -1)
	{
	  FD_SET(server->tcp_socket, read_fds);
	  if (server->qhead)
	    FD_SET(server->tcp_socket, write_fds);
	  if (server->tcp_socket >= nfds)
	    nfds = server->tcp_socket + 1;
	}
    }
  return nfds;
}
