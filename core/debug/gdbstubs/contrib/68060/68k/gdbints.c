#include "gdbints.h"

/******************************************************************************
 *
 * exceptionHandler - initialise an interrupt handler
 *
 *****************************************************************************/

void exceptionHandler(
	int		_vec,
	void* 	_pFunc
)
{
	volatile unsigned long*	pEntry;

	pEntry = (unsigned long*) ( _vec * 4 );
	*pEntry = (unsigned long) _pFunc;
}
