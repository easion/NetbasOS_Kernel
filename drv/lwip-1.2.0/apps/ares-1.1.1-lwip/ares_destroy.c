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
 * CHANGELOG: this file has been modified by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> 
 *            Departamento de Informática de Sistemas y Computadores          
 *            Universidad Politécnica de Valencia                             
 *            Valencia (Spain)    
 *            Date: April 2003                                          
 *
 */

#include "ares_private.h"

void ares_destroy(ares_channel channel)
{
  int i;
  struct query *query;

  for (i = 0; i < channel->nservers; i++)
    ares__close_sockets(&channel->servers[i]);
  free(channel->servers);
  for (i = 0; i < channel->ndomains; i++)
    free(channel->domains[i]);
  free(channel->domains);
  free(channel->sortlist);
  free(channel->lookups);
  while (channel->queries)
    {
      query = channel->queries;
      channel->queries = query->next;
      query->callback(query->arg, ARES_EDESTRUCTION, NULL, 0);
      free(query->tcpbuf);
      free(query->skip_server);
      free(query);
    }
  free(channel);
}
