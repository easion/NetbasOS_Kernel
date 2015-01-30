/*
 * Dynamic loading of modules into the kernel.
 *
 * Rewritten by Richard Henderson <rth@tamu.edu> Dec 1996
 */

#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/linux_defs.h>
#include <linux/linux_compat.h>
#include <linux/atomic.h>

/* Bits of module.flags.  */

#define MOD_UNINITIALIZED	0
#define MOD_RUNNING		1
#define MOD_DELETED		2
#define MOD_AUTOCLEAN		4
#define MOD_VISITED  		8
#define MOD_USED_ONCE		16
#define MOD_JUST_FREED		32
#define MOD_INITIALIZING	64

/* Values for query_module's which.  */

#define QM_MODULES	1
#define QM_DEPS		2
#define QM_REFS		3
#define QM_SYMBOLS	4
#define QM_INFO		5

/* Can the module be queried? */
#define MOD_CAN_QUERY(mod) (((mod)->flags & (MOD_RUNNING | MOD_INITIALIZING)) && !((mod)->flags & MOD_DELETED))

/* Poke the use count of a module.  */

#define __MOD_INC_USE_COUNT(mod)					
#define __MOD_DEC_USE_COUNT(mod)					
#define __MOD_IN_USE(mod)						

/* Indirect stringification.  */

#define __MODULE_STRING_1(x)	#x
#define __MODULE_STRING(x)	__MODULE_STRING_1(x)

/* Generic inter module communication.
 *
 * NOTE: This interface is intended for small amounts of data that are
 *       passed between two objects and either or both of the objects
 *       might be compiled as modules.  Do not over use this interface.
 *
 *       If more than two objects need to communicate then you probably
 *       need a specific interface instead of abusing this generic
 *       interface.  If both objects are *always* built into the kernel
 *       then a global extern variable is good enough, you do not need
 *       this interface.
 *
 * Keith Owens <kaos@ocs.com.au> 28 Oct 2000.
 */


#define MODULE_AUTHOR(name)
#define MODULE_LICENSE(license)
#define MODULE_DESCRIPTION(desc)
#define MODULE_SUPPORTED_DEVICE(name)
#define MODULE_PARM(var,type)
#define MODULE_PARM_DESC(var,desc)

/* Create a dummy reference to the table to suppress gcc unused warnings.  Put
 * the reference in the .data.exit section which is discarded when code is built
 * in, so the reference does not bloat the running kernel.  Note: cannot be
 * const, other exit data may be writable.
 */


#define MODULE_DEVICE_TABLE(type,name)		\
  MODULE_GENERIC_TABLE(type##_device,name)

/* Export a symbol either from the kernel or a module.

   In the kernel, the symbol is added to the kernel's global symbol table.

   In a module, it controls which variables are exported.  If no
   variables are explicitly exported, the action is controled by the
   insmod -[xX] flags.  Otherwise, only the variables listed are exported.
   This obviates the need for the old register_symtab() function.  */

#define __EXPORT_SYMBOL(sym,str)
#define EXPORT_SYMBOL(var)
#define EXPORT_SYMBOL_NOVERS(var)
#define EXPORT_SYMBOL_GPL(var)

/*#define __EXPORT_SYMBOL(sym, str)			\
const char __kstrtab_##sym[]				\
__attribute__((section(".kstrtab"))) = str;		\
const struct module_symbol __ksymtab_##sym 		\
__attribute__((section("__ksymtab"))) =			\
{ (unsigned long)&sym, __kstrtab_##sym }

#if defined(MODVERSIONS) || !defined(CONFIG_MODVERSIONS)
#define EXPORT_SYMBOL(var)  __EXPORT_SYMBOL(var, __MODULE_STRING(var))
#define EXPORT_SYMBOL_GPL(var)  __EXPORT_SYMBOL_GPL(var, __MODULE_STRING(var))
#else
#define EXPORT_SYMBOL(var)  __EXPORT_SYMBOL(var, __MODULE_STRING(__VERSIONED_SYMBOL(var)))
#define EXPORT_SYMBOL_GPL(var)  __EXPORT_SYMBOL(var, __MODULE_STRING(__VERSIONED_SYMBOL(var)))
#endif

#define EXPORT_SYMBOL_NOVERS(var)  __EXPORT_SYMBOL(var, __MODULE_STRING(var))
*/


#define SET_MODULE_OWNER(some_struct) do { } while (0)
//  type##_device();\

#define module_init(type)		\
  int dll_main(){\
  type();\
}\
  int dll_version(){\
  kprintf("linux modules ##type running");\
}\

#define module_exit(type)		\
  int dll_destroy(){\
  type();\
}\


#ifdef __cplusplus
}
#endif

#endif /* _LINUX_MODULE_H */
