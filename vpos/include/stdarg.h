
#ifndef _VPOS_STDARG_H_
#define _VPOS_STDARG_H_

typedef char *va_list;

#define va_start(ap, p)	(ap = (char *) (&(p)+1))
#define va_arg(ap, type)	((type *) (ap += sizeof(type)))[-1]
#define va_end(ap)		((void)0)

#endif // _VPOS_STDARG_H_
