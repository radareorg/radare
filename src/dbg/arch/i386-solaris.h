#ifndef _INCLUDE_I386_SOLARIS_H_
#define _INCLUDE_I386_SOLARIS_H_

#include <sys/user.h>
#include <sys/regset.h>

#if 0
typedef int32_t greg32_t;
typedef int64_t	greg64_t;

#define	_NGREG32	19
#define	_NGREG64	28

typedef greg_t	gregset_t[_NGREG];

typedef	struct	user {
	gregset_t	u_reg;		/* user's saved registers */
	greg_t		*u_ar0;		/* address of user's saved R0 */
	char	u_psargs[PSARGSZ];	/* arguments from exec */
	void	(*u_signal[MAXSIG])();	/* Disposition of signals */
	int		u_code;		/* fault code on trap */
	caddr_t		u_addr;		/* fault PC on trap */
} user_t;
#endif

#define regs_t gregset_t
#define R_EIP(x) x[EIP]
#define R_EFLAGS(x) x[EFL]
#define R_EBP(x) x[EBP]
#define R_ESP(x) x[ESP]
#define R_EAX(x) x[EAX]
#define R_OEAX(x) x[TRAPNO] // trapno!?!
#define R_EBX(x) x[EBX]
#define R_ECX(x) x[ECX]
#define R_EDX(x) x[EDX]
#define R_ESI(x) x[ESI]
#define R_EDI(x) x[EDI]

/* registers offset */

#define R_EIP_OFF EIP*sizeof(int32_t)
#define R_EFLAGS_OFF EFL*sizeof(int32_t)
#define R_EBP_OFF EBP*sizeof(int32_t)
#define R_ESP_OFF ESP*sizeof(int32_t)
#define R_EAX_OFF EAX*sizeof(int32_t)
#define R_OEAX_OFF EAX*sizeof(int32_t)
#define R_EBX_OFF EBX*sizeof(int32_t)
#define R_ECX_OFF ECX*sizeof(int32_t)
#define R_EDX_OFF EDX*sizeof(int32_t)
#define R_ESI_OFF ESI*sizeof(int32_t)
#define R_EDI_OFF EDI*sizeof(int32_t)

#endif
