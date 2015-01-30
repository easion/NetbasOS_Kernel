#include <net/net.h>

#include <jicama/module.h>
#include <jicama/msgport.h>

static int g_hRouteListMutex = -1, g_hTryReleaseMutex = -1;
static struct route g_sSentinel;
static struct route *g_psStaticRoutes = NULL;
static struct route *g_psClonedRoutes = NULL;
static struct route *g_psDeviceRoutes = NULL;

/**
 * \par Description:
 * Initialise routing table.
 */
int init_route_table( void )
{
	if ( g_hRouteListMutex > 0 )
	{
		kprintf( KERN_WARNING, "init_route_table(): Routing already initialised!\n" );
		return EINVAL;
	}
	else
	{
		g_hRouteListMutex = create_semaphore( "routing table mutex", 1, 0 );//( "routing table mutex" );
		g_hTryReleaseMutex = create_semaphore( "routing try-release mutex", 1, 0 );//( "routing try-release mutex" );
	}

	memset( &g_sSentinel, 0, sizeof( g_sSentinel ) );
	//g_sSentinel.rt_psNext = &g_sSentinel;

	/* Initialise lists */
	g_psDeviceRoutes = &g_sSentinel;
	g_psClonedRoutes = &g_sSentinel;
	g_psStaticRoutes = &g_sSentinel;



	return g_hRouteListMutex < 0 ? g_hRouteListMutex : 0;
}

//
// ip_route
//
// Finds the appropriate network interface for a given IP address. It
// searches the list of network interfaces linearly. A match is found
// if the masked IP address of the network interface equals the masked
// IP address given to the function. The routine also detects if the
// address is the IP address of one of the interfaces.
//

struct netif *ip_route(struct ip_addr *dest)
{
  struct netif *netif;
  
  for (netif = netif_list; netif != NULL; netif = netif->next)
  {
    if (ip_addr_cmp(dest, &netif->ipaddr)) return netif;
  }

  for (netif = netif_list; netif != NULL; netif = netif->next)
  {
    if (ip_addr_maskcmp(dest, &netif->ipaddr, &netif->netmask)) 
    {
      //kprintf("ip: route packet to %a to interface %s\n", dest, netif->name);
      return netif;
    }
  }

  if (netif_default)
  {
    //kprintf("ip: route packet to %a to default interface %s\n", dest, netif_default->name);
    return netif_default;
  }

  return NULL;
}


#if 0


/**
 * On Intel, there's no problem with misaligned words so the following
 * optimisation makes sense (and is used elsewhere).
 */
static inline bool compare_net_address( ipaddr_t anAddr1, ipaddr_t anAddr2, ipaddr_t anMask )
{
	return ( ( *( uint32 * )anAddr1 ) & ( *( uint32 * )anMask ) ) == ( ( *( uint32 * )anAddr2 ) & ( *( uint32 * )anMask ) );
}


static void db_dump_routes( int argc, char **argv );



/**
 * \par Description:
 * Internal routine to create a route.  The route is not added to any
 * lists and is not locked.
 */
static struct route *create_route( ipaddr_p pnIpAddr, ipaddr_p pnNetmask, ipaddr_p pnGateway, int nMetric, uint32 nFlags )
{
	struct route *psRoute;
	uint32 nNetMask;
	int nMaskBits;

	psRoute = kmalloc( sizeof( struct route ), MEMF_KERNEL | MEMF_CLEAR );
	if ( psRoute == NULL )
	{
		kprintf( KERN_FATAL, "create_route(): Unable to allocate memory for route.\n" );

		return NULL;
	}

	/* Allocate reader-writer lock */
	psRoute->rt_hMutex = CREATE_RWLOCK( "route mutex" );
	if ( ( psRoute->rt_hMutex & SEMTYPE_KERNEL ) == 0 )
	{
		kprintf(  __FUNCTION__ "(): Created non-kernel semaphore!\n" );
	}

	if ( psRoute->rt_hMutex < 0 )
	{
		kprintf( KERN_FATAL, "create_route(): Unable to allocate route mutex.\n" );

		kfree( psRoute );
		return NULL;
	}

	/* Fill in route details */
	IP_COPYMASK( psRoute->rt_anNetAddr, pnIpAddr, pnNetmask );
	IP_COPYADDR( psRoute->rt_anNetMask, pnNetmask );

	/* Gateway address copied if RTF_GATEWAY specified (see below) */
	if ( nFlags & RTF_GATEWAY )
		IP_COPYADDR( psRoute->rt_anGatewayAddr, pnGateway );

	/* Count bits in netmask */
	nMaskBits = 0;
	nNetMask = *( ( uint32 * )pnNetmask );
	while ( nNetMask )
	{
		nMaskBits += ( nNetMask & 1 );
		nNetMask >>= 1;
	}

	psRoute->rt_nMaskBits = nMaskBits;
	psRoute->rt_nMetric = nMetric;
	psRoute->rt_nFlags = nFlags;
	/* psRoute->rt_psInterface = NULL; -- not necessary */

	return psRoute;
}

/**
 * Internal routine to delete a route.
 *
 * Others should call ip_release_route() to release a pointer to a route.
 * This function will be called as needed by ip_release_route().
 */
static void delete_route( struct route *psRoute )
{
	if ( psRoute == NULL )
	{
		kprintf( KERN_WARNING, "delete_route(): Asked to delete NULL route.\n" );

		return;
	}

	/* Release the route interface, if any */
	if ( psRoute->rt_psInterface )
	{
		/* Convert anonymous reference to local thread reference */
		if ( rwl_convert_from_anon( psRoute->rt_psInterface->ni_hMutex ) < 0 )
		{
			kprintf(  __FUNCTION__ "(): Interface mutex conversion failed.\n" );
		}

		release_net_interface( psRoute->rt_psInterface );
	}

	delete_semaphore( psRoute->rt_hMutex );

	kprintf(  "delete_route(): Freeing memory for route (0x%8.8lX).\n", ( uint32 )psRoute );

	kfree( psRoute );
}

/**
 * Internal routine to insert route into a list.
 *
 * This insert routine sorts routes into descending order by number of mask
 * bits, which means searches will always find the best route first.
 *
 * If bReplace is true, the new route will replace another with identical
 * address/netmask fields if found.  This expects to release an anonymous
 * read lock.
 * 
 * This routine must be called with an exclusive lock held on the route list
 * mutex.
 */
void insert_route( struct route *psRoute, struct route **ppsList, bool bReplace )
{
	struct route **ppsCurrent, *psReplace;

	for ( ppsCurrent = ppsList; ( *ppsCurrent ) != &g_sSentinel; ppsCurrent = &( *ppsCurrent )->rt_psNext )
	{
		if ( ( *ppsCurrent )->rt_nMaskBits <= psRoute->rt_nMaskBits )
			break;
	}

	/* Keep cycling to check that this route is not replacing another */
	for ( ; ( *ppsCurrent ) != &g_sSentinel; ppsCurrent = &( *ppsCurrent )->rt_psNext )
	{
		if ( ( *ppsCurrent )->rt_nMaskBits < psRoute->rt_nMaskBits )
			break;

		if ( IP_SAMEADDR( ( *ppsCurrent )->rt_anNetAddr, psRoute->rt_anNetAddr ) && IP_SAMEADDR( ( *ppsCurrent )->rt_anNetMask, psRoute->rt_anNetMask ) )
			break;
	}

	if ( bReplace && IP_SAMEADDR( ( *ppsCurrent )->rt_anNetAddr, psRoute->rt_anNetAddr ) && IP_SAMEADDR( ( *ppsCurrent )->rt_anNetMask, psRoute->rt_anNetMask ) && ( *ppsCurrent ) != &g_sSentinel )
	{
		/* Save route being replaced */
		psReplace = ( *ppsCurrent );

		/* Unlink old and insert new  */
		psRoute->rt_psNext = psReplace->rt_psNext;
		( *ppsCurrent ) = psRoute;

		/* Mark for deletion */
		psReplace->rt_psNext = NULL;

		/* Convert anonymous route reference acquired for list to local thread */
		if ( rwl_convert_from_anon( psReplace->rt_hMutex ) < 0 )
		{
			kprintf(  __FUNCTION__ "(): Route mutex conversion failed (1).\n" );
		}

		/* Release the route */
		ip_release_route( psReplace );
	}
	else
	{
		/* Insert */
		psRoute->rt_psNext = ( *ppsCurrent );
		( *ppsCurrent ) = psRoute;
	}

	/* Convert incoming route mutex reference to anonymous reference */
	if ( rwl_convert_to_anon( psRoute->rt_hMutex ) < 0 )
	{
		kprintf(  __FUNCTION__ "(): Route mutex conversion failed (2).\n" );
	}

}

/**
 * \par Description:
 * Adds a reference to a route pointer.
 *
 * Will return a newly referenced route, or NULL if unsuccessful.
 */
struct route *ip_acquire_route( struct route *psRoute )
{
	if ( psRoute == NULL )
		return NULL;

	if ( RWL_LOCK_RO( psRoute->rt_hMutex ) < 0 )
	{
		kprintf( KERN_FATAL, "ip_acquire_route(): Unable to lock route mutex.\n" );

		return NULL;
	}

	return psRoute;
}

/**
 * \par Description:
 * Finds the routing table entry that would reach pDstAddr.
 * If there are no explicit matching routes, the default
 * route is returned.  The route that is returned is always
 * a clone of the original route, with the appropriate
 * interface referenced.
 *
 * \par Note:
 * Routes that are returned must be released after use
 * with ip_release_route().
 */
struct route *ip_find_route( ipaddr_t pDstAddr )
{
	struct route *psRoute;
	struct route *psFound;
	NetInterface_s *psIf;
	int nError;

	if ( ( nError = RWL_LOCK_RW( g_hRouteListMutex ) ) < 0 )
	{
		kprintf( KERN_FATAL, "ip_find_route(): Unable to lock routing table (%d)\n", nError );

		return NULL;
	}

	/* Scan for a matching cloned route */
	psFound = NULL;
	for ( psRoute = g_psClonedRoutes; psRoute != &g_sSentinel; psRoute = psRoute->rt_psNext )
	{
		/* XXXKV: This is a hack. I am unsure how the initial "NULL" route ever gets into the table,
		   and it disappears after the interface is taken down and back up. This just avoids the
		   problem from mucking up routing. */
		ipaddr_t pNull = { 0x0, 0x0, 0x0, 0x0 };
		if( IP_SAMEADDR( psRoute->rt_anNetAddr, pNull ) && IP_SAMEADDR( psRoute->rt_anNetMask, pNull ) )
			continue;

		if ( compare_net_address( pDstAddr, psRoute->rt_anNetAddr, psRoute->rt_anNetMask ) )
		{
			psFound = ip_acquire_route( psRoute );
			if ( psFound == NULL )
			{
				kprintf(  "ip_find_route(): Unable to acquire cloned route!\n" );
				goto exit;
			}

			break;
		}
	}

	/* If we found a cloned route, go to the exit */
	if ( psFound != NULL )
		goto exit;

	/* Plan B: Is there a directly connected interface? */
	if ( ( psIf = find_interface_by_addr( pDstAddr ) ) != NULL )
	{
		/* Great! Create a cloned route based on the interface. */
		psRoute = create_route( psIf->ni_anIpAddr, psIf->ni_anSubNetMask, NULL, 0, RTF_UP | RTF_LOCAL | RTF_INTERFACE );

		if ( psRoute == NULL )
		{
			kprintf(  __FUNCTION__ "(): No memory for new cloned route.\n" );
			release_net_interface( psIf );

			goto exit;
		}

		/* Copy in interface pointer */
		psRoute->rt_psInterface = psIf;

		goto finish_clone;
	}

	/* Plan C: Look for a static route to clone. */
	for ( psRoute = g_psStaticRoutes; psRoute != &g_sSentinel; psRoute = psRoute->rt_psNext )
	{
		if ( compare_net_address( pDstAddr, psRoute->rt_anNetAddr, psRoute->rt_anNetMask ) )
		{
			psFound = psRoute;
			break;
		}
	}

	/* If we didn't find a static route, jump to exit */
	if ( psFound == NULL )
		goto exit;

	/**
	 * Find interface for the route's gateway
	 * Note that non-gateway static routes are ignored, as they can only specify
	 * directly connected hosts, which we will pick up with the code above.
	 **/
	if ( ( psFound->rt_nFlags & RTF_GATEWAY ) == 0 )
	{
		kprintf( KERN_INFO, "ip_find_route(): Ignoring non-gateway route.\n" );

		psFound = NULL;

		goto exit;
	}

	/* Look up interface that can reach the specified gateway */
	psIf = find_interface_by_addr( psFound->rt_anGatewayAddr );
	if ( psIf == NULL )
	{
		kprintf( KERN_INFO, "ip_find_route(): Gateway is not directly reachable.\n" );

		psFound = NULL;

		goto exit;
	}

	/* Create a cloned route from a static route */
	psRoute = create_route( psFound->rt_anNetAddr, psFound->rt_anNetMask, psFound->rt_anGatewayAddr, psFound->rt_nMetric, psFound->rt_nFlags );

	if ( psRoute == NULL )
	{
		kprintf(  __FUNCTION__ "(): No memory for new cloned route.\n" );
		release_net_interface( psIf );

		goto exit;
	}

	/* Copy in the new interface */
	psRoute->rt_psInterface = psIf;


      finish_clone:
	/* Convert interface reference to anonymous reference */
	if ( rwl_convert_to_anon( psRoute->rt_psInterface->ni_hMutex ) < 0 )
	{
		kprintf(  __FUNCTION__ "(): Conversion to anonymous lock failed.\n" );

		release_net_interface( psRoute->rt_psInterface );
		psRoute->rt_psInterface = NULL;

		delete_route( psRoute );
	}

	/* Acquire a reference for the cloned route list and make it anonymous */
	if ( ip_acquire_route( psRoute ) != psRoute )
	{
		kprintf(  __FUNCTION__ "(): Unable to acquire route reference for clone list.\n" );

		delete_route( psRoute );
	}

	/* Add to cloned route list */
	insert_route( psRoute, &g_psClonedRoutes, true );

	/* Acquire a reference to the new cloned route for the caller */
	psFound = ip_acquire_route( psRoute );
	if ( psFound == NULL )
	{
		kprintf( KERN_FATAL, "ip_find_route(): Unable to add reference to new cloned route.\n" );
	}

      exit:
	RWL_UNLOCK_RW( g_hRouteListMutex );

	return psFound;
}


/**
 * \par Creates a cloned route that provides a default route via the
 * interface named pzIfName.
 *
 * The returned route must be released with ip_release_route() when
 * finished with.
 *
 * If an error occurs, NULL is returned.
 */
struct route *ip_find_device_route( const char *pzIfName )
{
	struct route *psRoute, *psFound = NULL;
	ipaddr_t anNullAddr = { 0, 0, 0, 0 };

	/* Lock the route lists to look for an existing device route */
	if ( RWL_LOCK_RW( g_hRouteListMutex ) < 0 )
	{
		kprintf( KERN_FATAL, "ip_find_device_route(): Unable to lock the route list read-write.\n" );

		return NULL;
	}

	/* Scan for a matching device route */
	psFound = NULL;
	for ( psRoute = g_psDeviceRoutes; psRoute != &g_sSentinel; psRoute = psRoute->rt_psNext )
	{
		if ( psRoute->rt_psInterface == NULL )
		{
			kprintf( KERN_FATAL, "ip_find_device_route(): Device route does not have an interface!\n" );

			continue;
		}

		if ( strcmp( psRoute->rt_psInterface->ni_zName, pzIfName ) == 0 )
		{
			psFound = ip_acquire_route( psRoute );
			if ( psFound == NULL )
			{
				kprintf(  "ip_find_route(): Unable to acquire cloned route!\n" );
				goto exit;
			}

			break;
		}
	}

	/* If we found a cloned route, go to the exit */
	if ( psFound != NULL )
		goto exit;

	/* Create a default route for the interface */
	if ( ( psRoute = create_route( anNullAddr, anNullAddr, NULL, 0, RTF_UP ) ) == NULL )
		goto exit;

	/* Copy in the new interface */
	psRoute->rt_psInterface = find_interface( pzIfName );
	if ( psRoute->rt_psInterface == NULL )
	{
		delete_route( psRoute );

		goto exit;
	}

	/* Convert the interface reference to an anonymous reference */
	if ( rwl_convert_to_anon( psRoute->rt_psInterface->ni_hMutex ) < 0 )
	{
		kprintf(  __FUNCTION__ "(): Interface mutex conversion failed.\n" );
		release_net_interface( psRoute->rt_psInterface );
		psRoute->rt_psInterface = NULL;
		delete_route( psRoute );

		goto exit;
	}

	/* Acquire the new route reference for the list */
	if ( ip_acquire_route( psRoute ) != psRoute )
	{
		kprintf(  __FUNCTION__ "(): Unable to acquire reference for route list.\n" );
		delete_route( psRoute );

		goto exit;
	}

	/* Add to device route list */
	insert_route( psRoute, &g_psDeviceRoutes, false );

	/* Acquire the new route for the calling thread */
	if ( ( psFound = ip_acquire_route( psRoute ) ) == NULL )
	{
		kprintf(  __FUNCTION__ "(): Unable to acquire reference for caller.\n" );
	}

      exit:
	RWL_UNLOCK_RW( g_hRouteListMutex );

	return psFound;
}


/**
 * \par Description:
 * Locates the first static route that matches all of the provided criteria.
 * The route will have to be released after use by calling ip_release_route().
 */
struct route *ip_find_static_route( ipaddr_p anIpAddr, ipaddr_p anNetMask, ipaddr_p anGwAddr )
{
	struct route *psCurrent;

	if ( RWL_LOCK_RO( g_hRouteListMutex ) < 0 )
	{
		kprintf( KERN_FATAL, "ip_find_static_route(): Unable to lock route list!\n" );

		return NULL;
	}

	for ( psCurrent = g_psStaticRoutes; psCurrent != &g_sSentinel; psCurrent = psCurrent->rt_psNext )
	{
		if ( ( anIpAddr == NULL || IP_SAMEADDR( psCurrent->rt_anNetAddr, anIpAddr ) ) && ( anNetMask == NULL || IP_SAMEADDR( psCurrent->rt_anNetMask, anNetMask ) ) && ( anGwAddr == NULL || IP_SAMEADDR( psCurrent->rt_anGatewayAddr, anGwAddr ) ) )
			break;
	}

	if ( psCurrent != &g_sSentinel )
	{
		psCurrent = ip_acquire_route( psCurrent );
		if ( psCurrent == NULL )
		{
			kprintf( KERN_FATAL, "ip_find_static_route(): Unable to add ref to route!\n" );
		}
	}
	else
	{
		psCurrent = NULL;
	}

	RWL_UNLOCK_RO( g_hRouteListMutex );

	return psCurrent;
}

/**
 * \par Description:
 * Decrease the route's reference count.	If the route is marked as deleted,
 * and its reference count goes to zero, the route structure is freed.
 */
void ip_release_route( struct route *psRoute )
{
	int nError, nMayDelete = 0;

	if ( psRoute == NULL )
	{
		kprintf( KERN_WARNING, "ip_release_route(): Asked to release null pointer.\n" );

		return;
	}

	/* Lock the route list so we can see if we should try to delete this
	 * interface (e.g. is rt_psNext == NULL).
	 */
	if ( RWL_LOCK_RO( g_hRouteListMutex ) < 0 )
	{
		kprintf( KERN_FATAL, "ip_release_route(): Unable to lock route list!\n" );

		return;
	}

	nMayDelete = ( psRoute->rt_psNext == NULL );

	/* Unlock the list */
	RWL_UNLOCK_RO( g_hRouteListMutex );


	if ( nMayDelete == 0 )
	{
		/* No need to delete, just release our reference */
		RWL_UNLOCK_RO( psRoute->rt_hMutex );
		return;
	}

	/* As for interfaces, we must enforce a critical section between trying
	 * to upgrade to a full write lock and unlocking.
	 */
	if ( LOCK( g_hTryReleaseMutex ) < 0 )
	{
		kprintf( KERN_FATAL, "ip_release_route(): Unable to lock try-release mutex.\n" );

		return;
	}

	if ( ( nError = RWL_TRY_UPGRADE( psRoute->rt_hMutex ) ) < 0 )
	{
		if ( nError == ETIME || nError == EBUSY )
		{
			/* There are others yet to release */
			/* Release our read-only lock (decrease reference count) */
			RWL_UNLOCK_RO( psRoute->rt_hMutex );
		}
		else
		{
			kprintf( KERN_FATAL, "ip_release_route(): Unable to upgrade route mutex.\n" );
		}
	}
	else
	{

		/**
		 * Check that semaphore reader and writer counts are one, as other values
		 * indicate someone else has the route in use.
		 */
		if ( rwl_count_all_readers( psRoute->rt_hMutex ) == 1 && rwl_count_all_writers( psRoute->rt_hMutex ) == 1 )
		{
			/* Delete the route instance */
			delete_route( psRoute );
		}
		else
		{
			/* There are others yet to release */
			/* Release our read-write lock (decrease reference count) */
			kprintf(  __FUNCTION__ "(): RWLock count snafu: Lock ID %d.\n", psRoute->rt_hMutex );

			RWL_UNLOCK_RW( psRoute->rt_hMutex );
			RWL_UNLOCK_RO( psRoute->rt_hMutex );
		}
	}

	UNLOCK( g_hTryReleaseMutex );
}

/**
 * \par Description:
 * Adds a new static route for the network/host specified by pNetAddr and
 * pNetMask.
 *
 * Static routes are only considered if they set the RTF_GATEWAY flag.
 * Non-gateway routes (e.g. directly connected routes) are always based on
 * the available interfaces and cannot be statically configured.
 *
 * Once a route has been added, it can be retrieved by searching with the
 * same address, netmask and gateway using ip_find_static_route().
 *
 * \return Returns zero on success, or a negated errno error code if
 * unsuccessful.
 */
int add_route( ipaddr_p pNetAddr, ipaddr_p pNetMask, ipaddr_p pGateway, int nMetric, uint32 nFlags )
{
	struct route *psRoute;
	int nError;

	USE_FORMAT_IP( 3 );

	/* Check arguments */
	if ( pNetAddr == NULL || pNetMask == NULL || ( ( nFlags & RTF_GATEWAY ) > 0 && pGateway == NULL ) )
	{
		kprintf(  "add_route(): Invalid arguments.\n" );

		return EINVAL;
	}

	/* Allocate structure */
	psRoute = create_route( pNetAddr, pNetMask, pGateway, nMetric, nFlags | RTF_UP | RTF_STATIC );

	if ( psRoute == NULL )
	{
		kprintf( KERN_FATAL, "add_route(): Unable to allocate memory for new route.\n" );
		return ENOMEM;
	}

	if ( ip_acquire_route( psRoute ) != psRoute )
	{
		kprintf( KERN_FATAL, "add_route(): Unable to add reference to new route.\n" );

		delete_route( psRoute );
		return ENOLCK;
	}

	/* Lock the route list */
	if ( ( nError = RWL_LOCK_RW( g_hRouteListMutex ) ) < 0 )
	{
		kprintf( KERN_FATAL, "add_route(): Unable to lock route list\n" );

		delete_route( psRoute );
		return ENOLCK;
	}

	FORMAT_IP( psRoute->rt_anNetAddr, 0 );
	FORMAT_IP( psRoute->rt_anNetMask, 1 );
	FORMAT_IP( psRoute->rt_anGatewayAddr, 2 );

	kprintf(  __FUNCTION__ "(): Add static route to %s/%s via %s.\n", FORMATTED_IP( 0 ), FORMATTED_IP( 1 ), FORMATTED_IP( 2 ) );

	/* Insert into static route list (may replace existing route) */
	insert_route( psRoute, &g_psStaticRoutes, true );

	RWL_UNLOCK_RW( g_hRouteListMutex );

	return 0;
}

/**
 * \par Description:
 * Removes the specified route from the list and marks it for deletion.
 *
 * Pointers to the route remain valid until the last one is released with
 * ip_release_route().
 */
int del_route( ipaddr_p anIpAddr, ipaddr_p anNetMask, ipaddr_p anGwAddr )
{
	struct route **ppsCurrent, *psRemove;
	int nError;

	if ( ( nError = RWL_LOCK_RW( g_hRouteListMutex ) ) < 0 )
	{
		kprintf( KERN_FATAL, "del_route(): Unable to lock route list!\n" );

		return nError;
	}

	for ( ppsCurrent = &g_psStaticRoutes; ( *ppsCurrent ) != &g_sSentinel; ppsCurrent = &( *ppsCurrent )->rt_psNext )
	{
		if ( ( anIpAddr == NULL || IP_SAMEADDR( ( *ppsCurrent )->rt_anNetAddr, anIpAddr ) ) && ( anNetMask == NULL || IP_SAMEADDR( ( *ppsCurrent )->rt_anNetMask, anNetMask ) ) && ( anGwAddr == NULL || IP_SAMEADDR( ( *ppsCurrent )->rt_anGatewayAddr, anGwAddr ) ) )
			break;
	}

	if ( ( *ppsCurrent ) != &g_sSentinel )
	{
		psRemove = ( *ppsCurrent );

		( *ppsCurrent ) = psRemove->rt_psNext;
		psRemove->rt_psNext = NULL;

		/* Convert anonymous reference to local thread reference */
		if ( rwl_convert_from_anon( psRemove->rt_hMutex ) < 0 )
		{
			kprintf(  __FUNCTION__ "(): Conversion to thread reference failed.\n" );
		}

		ip_release_route( psRemove );
	}
	else
	{
		nError = -ENOENT;
	}

	RWL_UNLOCK_RW( g_hRouteListMutex );

	return nError;
}




/**
 * \par Description:
 * Deletes all cloned routes based on psInterface.
 */
void rt_release_interface( NetInterface_s *psInterface )
{
	struct route **ppsRoute;

	if ( psInterface == NULL )
		return;

	if ( RWL_LOCK_RW( g_hRouteListMutex ) < 0 )
	{
		kprintf( KERN_FATAL, "rt_release_interface(): Unable to lock route list.\n" );

		return;
	}

	/* Scan cloned route list for routes to delete */
	for ( ppsRoute = &g_psClonedRoutes; ( *ppsRoute ) != &g_sSentinel; ppsRoute = &( *ppsRoute )->rt_psNext )
	{
		if ( ( *ppsRoute )->rt_psInterface == psInterface )
		{
			struct route *psRemove = ( *ppsRoute );

			/* Unlink */
			( *ppsRoute ) = psRemove->rt_psNext;

			/* Convert anonymous list reference to local thread reference */
			if ( rwl_convert_from_anon( psRemove->rt_hMutex ) < 0 )
			{
				kprintf(  __FUNCTION__ "(): Conversion from anonymous mutex failed (1).\n" );
			}

			/* Mark for deletion */
			psRemove->rt_psNext = NULL;

			/* Release */
			ip_release_route( psRemove );
		}
	}

	/* Scan cloned route list for routes to delete */
	for ( ppsRoute = &g_psDeviceRoutes; ( *ppsRoute ) != &g_sSentinel; ppsRoute = &( *ppsRoute )->rt_psNext )
	{
		if ( ( *ppsRoute )->rt_psInterface == psInterface )
		{
			struct route *psRemove = ( *ppsRoute );

			/* Unlink */
			( *ppsRoute ) = psRemove->rt_psNext;

			/* Convert anonymous list reference to local thread reference */
			if ( rwl_convert_from_anon( psRemove->rt_hMutex ) < 0 )
			{
				kprintf(  __FUNCTION__ "(): Conversion from anonymous mutex failed (2).\n" );
			}

			/* Mark for deletion */
			psRemove->rt_psNext = NULL;

			/* Release */
			ip_release_route( psRemove );
		}
	}

	RWL_UNLOCK_RW( g_hRouteListMutex );
}
#endif

