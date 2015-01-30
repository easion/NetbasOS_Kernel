/* -*-C-*-
 * z8530Lib.h - Prototypes for Zilog 8530 SCC library
 *
 *               THIS SOFTWARE IS NOT COPYRIGHTED  
 * 
 * JANZ Computer offers the following for use in the public domain.  
 * JANZ Computer makes no warranty with regard to the software or 
 * it's performance and the user accepts the software "AS IS" with 
 * all faults.
 *
 * JANZ Computer  DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
 * TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IF THE ABOVE RESTRICTIONS CONFLICT WITH LOCAL LAW, THEN YOU MUST NOT
 * USE THIS SOFTWARE.
 *
 * 1997 (c) JANZ Computer AG
 *
 * $Id: z8530lib.h,v 1.1 2001/05/18 15:54:05 bgat Exp $
 */

/*
 * $Log: z8530lib.h,v $
 * Revision 1.1  2001/05/18 15:54:05  bgat
 * Added mvme172 target contribution
 *
 * Revision 1.1.1.1  2000/04/06 12:35:37  wolfgang
 * first import to cvs
 *
 * Revision 1.2  1997/11/30 17:26:01  oriada
 * Added deinit8530Uart()
 *
 * Revision 1.1  1997/11/24 18:23:35  oriada
 * Initial revision
 *
 */


#ifndef z8530Lib_DEFINED
#define z8530Lib_DEFINED
#if __cplusplus
extern "C" {
#endif


/***** defines **************************************************************/

/* To be used in init8530Uart() */
#define Z8530_CHANNEL_A      0
#define Z8530_CHANNEL_B      1
    
    
/***** typedefs *************************************************************/

    
/***** function declarations  ***********************************************/

extern int init8530Uart(int, int, int, int, int);
extern int deinit8530Uart(int);
extern int tx8530Uart(int, int);
extern int txStat8530Uart(int);
extern int rx8530Uart(int);
extern int rxStat8530Uart(int);


#if __cplusplus
}
#endif
#endif /* z8530Lib_DEFINED */
