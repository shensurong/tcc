/*
*  TMS320C67xx code generator for TCC
*
*  Copyright (c) 2001, 2002 Fabrice Bellard
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef TARGET_DEFS_ONLY

/* #define ASSEMBLY_LISTING_C67 */

/* number of available registers */
#define NB_REGS            24

/* a register can belong to several classes. The classes must be
sorted from more general to more precise (see gv2() code which does
assumptions on it). */
#define RC_INT     0x0001	/* generic integer register */
#define RC_FLOAT   0x0002	/* generic float register */
#define RC_EAX     0x0004
#define RC_ST0     0x0008
#define RC_ECX     0x0010
#define RC_EDX     0x0020
#define RC_INT_BSIDE  0x00000040	/* generic integer register  on b side */
#define RC_C67_A4     0x00000100
#define RC_C67_A5     0x00000200
#define RC_C67_B4     0x00000400
#define RC_C67_B5     0x00000800
#define RC_C67_A6     0x00001000
#define RC_C67_A7     0x00002000
#define RC_C67_B6     0x00004000
#define RC_C67_B7     0x00008000
#define RC_C67_A8     0x00010000
#define RC_C67_A9     0x00020000
#define RC_C67_B8     0x00040000
#define RC_C67_B9     0x00080000
#define RC_C67_A10    0x00100000
#define RC_C67_A11    0x00200000
#define RC_C67_B10    0x00400000
#define RC_C67_B11    0x00800000
#define RC_C67_A12    0x01000000
#define RC_C67_A13    0x02000000
#define RC_C67_B12    0x04000000
#define RC_C67_B13    0x08000000
#define RC_IRET    RC_C67_A4	/* function return: integer register */
#define RC_LRET    RC_C67_A5	/* function return: second integer register */
#define RC_FRET    RC_C67_A4	/* function return: float register */

/* pretty names for the registers */
enum {
	TREG_EAX = 0,		// really A2
	TREG_ECX,			// really A3
	TREG_EDX,			// really B0
	TREG_ST0,			// really B1
	TREG_C67_A4,
	TREG_C67_A5,
	TREG_C67_B4,
	TREG_C67_B5,
	TREG_C67_A6,
	TREG_C67_A7,
	TREG_C67_B6,
	TREG_C67_B7,
	TREG_C67_A8,
	TREG_C67_A9,
	TREG_C67_B8,
	TREG_C67_B9,
	TREG_C67_A10,
	TREG_C67_A11,
	TREG_C67_B10,
	TREG_C67_B11,
	TREG_C67_A12,
	TREG_C67_A13,
	TREG_C67_B12,
	TREG_C67_B13,
};

/* return registers for function */
#define REG_IRET TREG_C67_A4	/* single word int return register */
#define REG_LRET TREG_C67_A5	/* second word return register (for long long) */
#define REG_FRET TREG_C67_A4	/* float return register */

/* defined if function parameters must be evaluated in reverse order */
/* #define INVERT_FUNC_PARAMS */

/* defined if structures are passed as pointers. Otherwise structures
are directly pushed on stack. */
/* #define FUNC_STRUCT_PARAM_AS_PTR */

/* pointer size, in bytes */
#define PTR_SIZE 4

/* long double size and alignment, in bytes */
#define LDOUBLE_SIZE  12
#define LDOUBLE_ALIGN 4
/* maximum alignment (for aligned attribute support) */
#define MAX_ALIGN     8

/******************************************************/
/* ELF defines */

#define EM_TCC_TARGET EM_C60

/* relocation type for 32 bit data relocation */
#define R_DATA_32   R_C60_32
#define R_DATA_PTR  R_C60_32
#define R_JMP_SLOT  R_C60_JMP_SLOT
#define R_COPY      R_C60_COPY

#define ELF_START_ADDR 0x00000400
#define ELF_PAGE_SIZE  0x1000

/******************************************************/
#else /* ! TARGET_DEFS_ONLY */
/******************************************************/
#include "tcc.h"

ST_DATA const int reg_classes[NB_REGS] = {
	/* eax */ RC_INT | RC_FLOAT | RC_EAX,
	// only allow even regs for floats (allow for doubles)
	/* ecx */ RC_INT | RC_ECX,
	/* edx */ RC_INT | RC_INT_BSIDE | RC_FLOAT | RC_EDX,
	// only allow even regs for floats (allow for doubles)
	/* st0 */ RC_INT | RC_INT_BSIDE | RC_ST0,
	/* A4  */ RC_C67_A4,
	/* A5  */ RC_C67_A5,
	/* B4  */ RC_C67_B4,
	/* B5  */ RC_C67_B5,
	/* A6  */ RC_C67_A6,
	/* A7  */ RC_C67_A7,
	/* B6  */ RC_C67_B6,
	/* B7  */ RC_C67_B7,
	/* A8  */ RC_C67_A8,
	/* A9  */ RC_C67_A9,
	/* B8  */ RC_C67_B8,
	/* B9  */ RC_C67_B9,
	/* A10  */ RC_C67_A10,
	/* A11  */ RC_C67_A11,
	/* B10  */ RC_C67_B10,
	/* B11  */ RC_C67_B11,
	/* A12  */ RC_C67_A10,
	/* A13  */ RC_C67_A11,
	/* B12  */ RC_C67_B10,
	/* B13  */ RC_C67_B11
};

// although tcc thinks it is passing parameters on the stack,
// the C67 really passes up to the first 10 params in special
// regs or regs pairs (for 64 bit params).  So keep track of
// the stack offsets so we can translate to the appropriate 
// reg (pair)

#define NoCallArgsPassedOnStack 10
int NoOfCurFuncArgs;
int TranslateStackToReg[NoCallArgsPassedOnStack];
int ParamLocOnStack[NoCallArgsPassedOnStack];
int TotalBytesPushedOnStack;

#ifndef FALSE
# define FALSE 0
# define TRUE 1
#endif

#undef BOOL
#define BOOL int

#define ALWAYS_ASSERT(tcc_state, x) \
do {\
   if (!(x))\
       tcc_error(tcc_state, "internal compiler error file at %s:%d", __FILE__, __LINE__);\
} while (0)

/******************************************************/
static unsigned long func_sub_sp_offset;
static int func_ret_sub;

static BOOL C67_invert_test;
static int C67_compare_reg;

#ifdef ASSEMBLY_LISTING_C67
FILE *f = NULL;
#endif

void C67_g(TCCState *tcc_state, int c)
{
	int ind1;

#ifdef ASSEMBLY_LISTING_C67
	fprintf(f, " %08X", c);
#endif
	ind1 = tcc_state->ind + 4;
	if (ind1 > (int)tcc_state->cur_text_section->data_allocated)
		section_realloc(tcc_state, tcc_state->cur_text_section, ind1);
	tcc_state->cur_text_section->data[tcc_state->ind] = c & 0xff;
	tcc_state->cur_text_section->data[tcc_state->ind + 1] = (c >> 8) & 0xff;
	tcc_state->cur_text_section->data[tcc_state->ind + 2] = (c >> 16) & 0xff;
	tcc_state->cur_text_section->data[tcc_state->ind + 3] = (c >> 24) & 0xff;
	tcc_state->ind = ind1;
}


/* output a symbol and patch all calls to it */
void gsym_addr(TCCState *tcc_state, int t, int a)
{
	int n, *ptr;
	while (t) {
		ptr = (int *)(tcc_state->cur_text_section->data + t);
		{
			Sym *sym;

			// extract 32 bit address from MVKH/MVKL
			n = ((*ptr >> 7) & 0xffff);
			n |= ((*(ptr + 1) >> 7) & 0xffff) << 16;

			// define a label that will be relocated

			sym = get_sym_ref(tcc_state, &tcc_state->char_pointer_type, tcc_state->cur_text_section, a, 0);
			greloc(tcc_state, tcc_state->cur_text_section, sym, t, R_C60LO16);
			greloc(tcc_state, tcc_state->cur_text_section, sym, t + 4, R_C60HI16);

			// clear out where the pointer was

			*ptr &= ~(0xffff << 7);
			*(ptr + 1) &= ~(0xffff << 7);
		}
		t = n;
	}
}

void gsym(TCCState *tcc_state, int t)
{
	gsym_addr(tcc_state, t, tcc_state->ind);
}

// these are regs that tcc doesn't really know about, 
// but assign them unique values so the mapping routines
// can distinguish them

#define C67_A0 105
#define C67_SP 106
#define C67_B3 107
#define C67_FP 108
#define C67_B2 109
#define C67_CREG_ZERO -1	/* Special code for no condition reg test */


int ConvertRegToRegClass(int r)
{
	// only works for A4-B13

	return RC_C67_A4 << (r - TREG_C67_A4);
}


// map TCC reg to C67 reg number

int C67_map_regn(TCCState *tcc_state, int r)
{
	if (r == 0)			// normal tcc regs
		return 0x2;		// A2
	else if (r == 1)		// normal tcc regs
		return 3;		// A3
	else if (r == 2)		// normal tcc regs
		return 0;		// B0
	else if (r == 3)		// normal tcc regs
		return 1;		// B1
	else if (r >= TREG_C67_A4 && r <= TREG_C67_B13)	// these form a pattern of alt pairs
		return (((r & 0xfffffffc) >> 1) | (r & 1)) + 2;
	else if (r == C67_A0)
		return 0;		// set to A0 (offset reg)
	else if (r == C67_B2)
		return 2;		// set to B2 (offset reg)
	else if (r == C67_B3)
		return 3;		// set to B3 (return address reg)
	else if (r == C67_SP)
		return 15;		// set to SP (B15) (offset reg)
	else if (r == C67_FP)
		return 15;		// set to FP (A15) (offset reg)
	else if (r == C67_CREG_ZERO)
		return 0;		// Special code for no condition reg test
	else
		ALWAYS_ASSERT(tcc_state, FALSE);

	return 0;
}

// mapping from tcc reg number to 
// C67 register to condition code field
//
// valid condition code regs are:
//
// tcc reg 2 ->B0 -> 1
// tcc reg 3 ->B1 -> 2
// tcc reg 0 -> A2 -> 5
// tcc reg 1 -> A3 -> X
// tcc reg      B2 -> 3

int C67_map_regc(TCCState *tcc_state, int r)
{
	if (r == 0)			// normal tcc regs
		return 0x5;
	else if (r == 2)		// normal tcc regs
		return 0x1;
	else if (r == 3)		// normal tcc regs
		return 0x2;
	else if (r == C67_B2)	// normal tcc regs
		return 0x3;
	else if (r == C67_CREG_ZERO)
		return 0;		// Special code for no condition reg test
	else
		ALWAYS_ASSERT(tcc_state, FALSE);

	return 0;
}


// map TCC reg to C67 reg side A or B

int C67_map_regs(TCCState *tcc_state, int r)
{
	if (r == 0)			// normal tcc regs
		return 0x0;
	else if (r == 1)		// normal tcc regs
		return 0x0;
	else if (r == 2)		// normal tcc regs
		return 0x1;
	else if (r == 3)		// normal tcc regs
		return 0x1;
	else if (r >= TREG_C67_A4 && r <= TREG_C67_B13)	// these form a pattern of alt pairs
		return (r & 2) >> 1;
	else if (r == C67_A0)
		return 0;		// set to A side 
	else if (r == C67_B2)
		return 1;		// set to B side 
	else if (r == C67_B3)
		return 1;		// set to B side
	else if (r == C67_SP)
		return 0x1;		// set to SP (B15) B side 
	else if (r == C67_FP)
		return 0x0;		// set to FP (A15) A side 
	else
		ALWAYS_ASSERT(tcc_state, FALSE);

	return 0;
}

int C67_map_S12(TCCState *tcc_state, char *s)
{
	if (strstr(s, ".S1") != NULL)
		return 0;
	else if (strcmp(s, ".S2"))
		return 1;
	else
		ALWAYS_ASSERT(tcc_state, FALSE);

	return 0;
}

int C67_map_D12(TCCState *tcc_state, char *s)
{
	if (strstr(s, ".D1") != NULL)
		return 0;
	else if (strcmp(s, ".D2"))
		return 1;
	else
		ALWAYS_ASSERT(tcc_state, FALSE);

	return 0;
}



void C67_asm(TCCState *tcc_state, char *s, int a, int b, int c)
{
	BOOL xpath;

#ifdef ASSEMBLY_LISTING_C67
	if (!f) {
		f = fopen("TCC67_out.txt", "wt");
	}
	fprintf(f, "%04X ", ind);
#endif

	if (strstr(s, "MVKL") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |
			((a & 0xffff) << 7) | (0x0a << 2) | (C67_map_regs(tcc_state, b) << 1));
	}
	else if (strstr(s, "MVKH") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |
			(((a >> 16) & 0xffff) << 7) |
			(0x1a << 2) | (C67_map_regs(tcc_state, b) << 1));
	}
	else if (strstr(s, "STW.D SP POST DEC") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(15 << 18) |	//SP B15
			(2 << 13) |	//ucst5 (must keep 8 byte boundary !!)
			(0xa << 9) |	//mode a = post dec ucst
			(0 << 8) |	//r (LDDW bit 0)
			(1 << 7) |	//y D1/D2 use B side
			(7 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STB.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2 A side
			(3 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STH.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2 A side
			(5 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STB.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2 A side
			(3 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STH.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2 A side
			(5 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STW.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2 A side
			(7 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STW.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(C67_map_regn(tcc_state, b) << 18) |	//base reg A0
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, b) << 7) |	//y D1/D2 base reg side
			(7 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STH.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(C67_map_regn(tcc_state, b) << 18) |	//base reg A0
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, b) << 7) |	//y D1/D2 base reg side
			(5 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STB.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(C67_map_regn(tcc_state, b) << 18) |	//base reg A0
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, b) << 7) |	//y D1/D2 base reg side
			(3 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "STW.D +*") == s) {
		ALWAYS_ASSERT(tcc_state, c < 32);
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//src
			(C67_map_regn(tcc_state, b) << 18) |	//base reg A0
			(c << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, b) << 7) |	//y D1/D2 base reg side
			(7 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of src
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDW.D SP PRE INC") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//dst
			(15 << 18) |	//base reg B15
			(2 << 13) |	//ucst5 (must keep 8 byte boundary)
			(9 << 9) |	//mode 9 = pre inc ucst5
			(0 << 8) |	//r (LDDW bit 0)
			(1 << 7) |	//y D1/D2  B side
			(6 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDDW.D SP PRE INC") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//dst
			(15 << 18) |	//base reg B15
			(1 << 13) |	//ucst5 (must keep 8 byte boundary)
			(9 << 9) |	//mode 9 = pre inc ucst5
			(1 << 8) |	//r (LDDW bit 1)
			(1 << 7) |	//y D1/D2  B side
			(6 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDW.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//dst
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2  A side
			(6 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDDW.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//dst
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(1 << 8) |	//r (LDDW bit 1)
			(0 << 7) |	//y D1/D2  A side
			(6 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDH.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//dst
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2  A side
			(4 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDB.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//dst
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2  A side
			(2 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDHU.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//dst
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2  A side
			(0 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDBU.D *+SP[A0]") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, a) << 23) |	//dst
			(15 << 18) |	//base reg A15
			(0 << 13) |	//offset reg A0
			(5 << 9) |	//mode 5 = pos offset, base reg + off reg
			(0 << 8) |	//r (LDDW bit 0)
			(0 << 7) |	//y D1/D2  A side
			(1 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, a) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDW.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(C67_map_regn(tcc_state, a) << 18) |	//base reg A15
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, a) << 7) |	//y D1/D2  src side
			(6 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDDW.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(C67_map_regn(tcc_state, a) << 18) |	//base reg A15
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(1 << 8) |	//r (LDDW bit 1)
			(C67_map_regs(tcc_state, a) << 7) |	//y D1/D2  src side
			(6 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDH.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(C67_map_regn(tcc_state, a) << 18) |	//base reg A15
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, a) << 7) |	//y D1/D2  src side
			(4 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDB.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(C67_map_regn(tcc_state, a) << 18) |	//base reg A15
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, a) << 7) |	//y D1/D2  src side
			(2 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDHU.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(C67_map_regn(tcc_state, a) << 18) |	//base reg A15
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, a) << 7) |	//y D1/D2  src side
			(0 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDBU.D *") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(C67_map_regn(tcc_state, a) << 18) |	//base reg A15
			(0 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, a) << 7) |	//y D1/D2  src side
			(1 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "LDW.D +*") == s) {
		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(C67_map_regn(tcc_state, a) << 18) |	//base reg A15
			(1 << 13) |	//cst5
			(1 << 9) |	//mode 1 = pos cst offset
			(0 << 8) |	//r (LDDW bit 0)
			(C67_map_regs(tcc_state, a) << 7) |	//y D1/D2  src side
			(6 << 4) |	//ldst 3=STB, 5=STH 5, 7=STW, 6=LDW 4=LDH 2=LDB 0=LDHU 1=LDBU 
			(1 << 2) |	//opcode
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPLTSP") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x3a << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPGTSP") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x39 << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPEQSP") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x38 << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}

	else if (strstr(s, "CMPLTDP") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x2a << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPGTDP") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x29 << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPEQDP") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x28 << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPLT") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x57 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPGT") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x47 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPEQ") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x53 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPLTU") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x5f << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "CMPGTU") == s) {
		xpath = C67_map_regs(tcc_state, a) ^ C67_map_regs(tcc_state, b);
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1
			(xpath << 12) |	//x use cross path for src2
			(0x4f << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side for reg c
			(0 << 0));	//parallel
	}
	else if (strstr(s, "B DISP") == s) {
		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//z
			(a << 7) |	//cnst
			(0x4 << 2) |	//opcode fixed
			(0 << 1) |	//S0/S1
			(0 << 0));	//parallel
	}
	else if (strstr(s, "B.") == s) {
		xpath = C67_map_regs(tcc_state, c) ^ 1;

		C67_g(tcc_state, 
			(C67_map_regc(tcc_state, b) << 29) |	//creg
			(a << 28) |	//inv
			(0 << 23) |	//dst
			(C67_map_regn(tcc_state, c) << 18) |	//src2
			(0 << 13) |	//
			(xpath << 12) |	//x cross path if !B side
			(0xd << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(1 << 1) |	//must be S2
			(0 << 0));	//parallel
	}
	else if (strstr(s, "MV.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(0 << 13) |	//src1 (cst5)
			(xpath << 12) |	//x cross path if opposite sides
			(0x2 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "SPTRUNC.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(0 << 13) |	//src1 NA
			(xpath << 12) |	//x cross path if opposite sides
			(0xb << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "DPTRUNC.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			((C67_map_regn(tcc_state, b) + 1) << 18) |	//src2   WEIRD CPU must specify odd reg for some reason
			(0 << 13) |	//src1 NA
			(xpath << 12) |	//x cross path if opposite sides
			(0x1 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "INTSP.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2   
			(0 << 13) |	//src1 NA
			(xpath << 12) |	//x cross path if opposite sides
			(0x4a << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "INTSPU.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2  
			(0 << 13) |	//src1 NA
			(xpath << 12) |	//x cross path if opposite sides
			(0x49 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "INTDP.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2  
			(0 << 13) |	//src1 NA
			(xpath << 12) |	//x cross path if opposite sides
			(0x39 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "INTDPU.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			((C67_map_regn(tcc_state, b) + 1) << 18) |	//src2   WEIRD CPU must specify odd reg for some reason
			(0 << 13) |	//src1 NA
			(xpath << 12) |	//x cross path if opposite sides
			(0x3b << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "SPDP.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(0 << 13) |	//src1 NA
			(xpath << 12) |	//x cross path if opposite sides
			(0x2 << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "DPSP.L") == s) {
		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, b) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			((C67_map_regn(tcc_state, b) + 1) << 18) |	//src2 WEIRD CPU must specify odd reg for some reason
			(0 << 13) |	//src1 NA
			(0 << 12) |	//x cross path if opposite sides
			(0x9 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "ADD.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x3 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "SUB.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x7 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "OR.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x7f << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "AND.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x7b << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "XOR.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x6f << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "ADDSP.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x10 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "ADDDP.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x18 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "SUBSP.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x11 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "SUBDP.L") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x19 << 5) |	//opcode
			(0x6 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "MPYSP.M") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x1c << 7) |	//opcode
			(0x0 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "MPYDP.M") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2 (possible x path)
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x0e << 7) |	//opcode
			(0x0 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "MPYI.M") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, a) == C67_map_regs(tcc_state, c));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1 (cst5)
			(xpath << 12) |	//x cross path if opposite sides
			(0x4 << 7) |	//opcode
			(0x0 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "SHR.S") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x37 << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "SHRU.S") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x27 << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "SHL.S") == s) {
		xpath = C67_map_regs(tcc_state, b) ^ C67_map_regs(tcc_state, c);

		ALWAYS_ASSERT(tcc_state, C67_map_regs(tcc_state, c) == C67_map_regs(tcc_state, a));

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, c) << 23) |	//dst
			(C67_map_regn(tcc_state, b) << 18) |	//src2
			(C67_map_regn(tcc_state, a) << 13) |	//src1 
			(xpath << 12) |	//x cross path if opposite sides
			(0x33 << 6) |	//opcode
			(0x8 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, c) << 1) |	//side of dest
			(0 << 0));	//parallel
	}
	else if (strstr(s, "||ADDK") == s) {
		xpath = 0;		// no xpath required just use the side of the src/dst

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(a << 07) |	//scst16
			(0x14 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(1 << 0));	//parallel
	}
	else if (strstr(s, "ADDK") == s) {
		xpath = 0;		// no xpath required just use the side of the src/dst

		C67_g(tcc_state, 
			(0 << 29) |	//creg
			(0 << 28) |	//inv
			(C67_map_regn(tcc_state, b) << 23) |	//dst
			(a << 07) |	//scst16
			(0x14 << 2) |	//opcode fixed
			(C67_map_regs(tcc_state, b) << 1) |	//side of dst
			(0 << 0));	//parallel
	}
	else if (strstr(s, "NOP") == s) {
		C67_g(tcc_state, 
			((a - 1) << 13) |	//no of cycles
			(0 << 0));	//parallel
	}
	else
		ALWAYS_ASSERT(tcc_state, FALSE);

#ifdef ASSEMBLY_LISTING_C67
	fprintf(f, " %s %d %d %d\n", s, a, b, c);
#endif

}

//r=reg to load, fr=from reg, symbol for relocation, constant

void C67_MVKL(TCCState *tcc_state, int r, int fc)
{
	C67_asm(tcc_state, "MVKL.", fc, r, 0);
}

void C67_MVKH(TCCState *tcc_state, int r, int fc)
{
	C67_asm(tcc_state, "MVKH.", fc, r, 0);
}

void C67_STB_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "STB.D *+SP[A0]", r, 0, 0);	// STB  r,*+SP[A0]
}

void C67_STH_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "STH.D *+SP[A0]", r, 0, 0);	// STH  r,*+SP[A0]
}

void C67_STW_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "STW.D *+SP[A0]", r, 0, 0);	// STW  r,*+SP[A0]
}

void C67_STB_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "STB.D *", r, r2, 0);	// STB  r, *r2
}

void C67_STH_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "STH.D *", r, r2, 0);	// STH  r, *r2
}

void C67_STW_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "STW.D *", r, r2, 0);	// STW  r, *r2
}

void C67_STW_PTR_PRE_INC(TCCState *tcc_state, int r, int r2, int n)
{
	C67_asm(tcc_state, "STW.D +*", r, r2, n);	// STW  r, *+r2
}

void C67_PUSH(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "STW.D SP POST DEC", r, 0, 0);	// STW  r,*SP--
}

void C67_LDW_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "LDW.D *+SP[A0]", r, 0, 0);	// LDW  *+SP[A0],r
}

void C67_LDDW_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "LDDW.D *+SP[A0]", r, 0, 0);	// LDDW  *+SP[A0],r
}

void C67_LDH_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "LDH.D *+SP[A0]", r, 0, 0);	// LDH  *+SP[A0],r
}

void C67_LDB_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "LDB.D *+SP[A0]", r, 0, 0);	// LDB  *+SP[A0],r
}

void C67_LDHU_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "LDHU.D *+SP[A0]", r, 0, 0);	// LDHU  *+SP[A0],r
}

void C67_LDBU_SP_A0(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "LDBU.D *+SP[A0]", r, 0, 0);	// LDBU  *+SP[A0],r
}

void C67_LDW_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "LDW.D *", r, r2, 0);	// LDW  *r,r2
}

void C67_LDDW_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "LDDW.D *", r, r2, 0);	// LDDW  *r,r2
}

void C67_LDH_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "LDH.D *", r, r2, 0);	// LDH  *r,r2
}

void C67_LDB_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "LDB.D *", r, r2, 0);	// LDB  *r,r2
}

void C67_LDHU_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "LDHU.D *", r, r2, 0);	// LDHU  *r,r2
}

void C67_LDBU_PTR(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "LDBU.D *", r, r2, 0);	// LDBU  *r,r2
}

void C67_LDW_PTR_PRE_INC(TCCState *tcc_state, int r, int r2)
{
	C67_asm(tcc_state, "LDW.D +*", r, r2, 0);	// LDW  *+r,r2
}

void C67_POP(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "LDW.D SP PRE INC", r, 0, 0);	// LDW  *++SP,r
}

void C67_POP_DW(TCCState *tcc_state, int r)
{
	C67_asm(tcc_state, "LDDW.D SP PRE INC", r, 0, 0);	// LDDW  *++SP,r
}

void C67_CMPLT(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPLT.L1", s1, s2, dst);
}

void C67_CMPGT(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPGT.L1", s1, s2, dst);
}

void C67_CMPEQ(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPEQ.L1", s1, s2, dst);
}

void C67_CMPLTU(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPLTU.L1", s1, s2, dst);
}

void C67_CMPGTU(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPGTU.L1", s1, s2, dst);
}


void C67_CMPLTSP(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPLTSP.S1", s1, s2, dst);
}

void C67_CMPGTSP(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPGTSP.S1", s1, s2, dst);
}

void C67_CMPEQSP(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPEQSP.S1", s1, s2, dst);
}

void C67_CMPLTDP(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPLTDP.S1", s1, s2, dst);
}

void C67_CMPGTDP(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPGTDP.S1", s1, s2, dst);
}

void C67_CMPEQDP(TCCState *tcc_state, int s1, int s2, int dst)
{
	C67_asm(tcc_state, "CMPEQDP.S1", s1, s2, dst);
}


void C67_IREG_B_REG(TCCState *tcc_state, int inv, int r1, int r2)	// [!R] B  r2
{
	C67_asm(tcc_state, "B.S2", inv, r1, r2);
}


// call with how many 32 bit words to skip
// (0 would branch to the branch instruction)

void C67_B_DISP(TCCState *tcc_state, int disp)	//  B  +2  Branch with constant displacement
{
	// Branch point is relative to the 8 word fetch packet
	//
	// we will assume the text section always starts on an 8 word (32 byte boundary)
	//
	// so add in how many words into the fetch packet the branch is


	C67_asm(tcc_state, "B DISP", disp + ((tcc_state->ind & 31) >> 2), 0, 0);
}

void C67_NOP(TCCState *tcc_state, int n)
{
	C67_asm(tcc_state, "NOP", n, 0, 0);
}

void C67_ADDK(TCCState *tcc_state, int n, int r)
{
	ALWAYS_ASSERT(tcc_state, abs(n) < 32767);

	C67_asm(tcc_state, "ADDK", n, r, 0);
}

void C67_ADDK_PARALLEL(TCCState *tcc_state, int n, int r)
{
	ALWAYS_ASSERT(tcc_state, abs(n) < 32767);

	C67_asm(tcc_state, "||ADDK", n, r, 0);
}

void C67_Adjust_ADDK(TCCState *tcc_state, int *inst, int n)
{
	ALWAYS_ASSERT(tcc_state, abs(n) < 32767);

	*inst = (*inst & (~(0xffff << 7))) | ((n & 0xffff) << 7);
}

void C67_MV(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "MV.L", 0, r, v);
}


void C67_DPTRUNC(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "DPTRUNC.L", 0, r, v);
}

void C67_SPTRUNC(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "SPTRUNC.L", 0, r, v);
}

void C67_INTSP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "INTSP.L", 0, r, v);
}

void C67_INTDP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "INTDP.L", 0, r, v);
}

void C67_INTSPU(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "INTSPU.L", 0, r, v);
}

void C67_INTDPU(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "INTDPU.L", 0, r, v);
}

void C67_SPDP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "SPDP.L", 0, r, v);
}

void C67_DPSP(TCCState *tcc_state, int r, int v)	// note regs must be on the same side
{
	C67_asm(tcc_state, "DPSP.L", 0, r, v);
}

void C67_ADD(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "ADD.L", v, r, v);
}

void C67_SUB(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "SUB.L", v, r, v);
}

void C67_AND(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "AND.L", v, r, v);
}

void C67_OR(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "OR.L", v, r, v);
}

void C67_XOR(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "XOR.L", v, r, v);
}

void C67_ADDSP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "ADDSP.L", v, r, v);
}

void C67_SUBSP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "SUBSP.L", v, r, v);
}

void C67_MPYSP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "MPYSP.M", v, r, v);
}

void C67_ADDDP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "ADDDP.L", v, r, v);
}

void C67_SUBDP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "SUBDP.L", v, r, v);
}

void C67_MPYDP(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "MPYDP.M", v, r, v);
}

void C67_MPYI(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "MPYI.M", v, r, v);
}

void C67_SHL(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "SHL.S", r, v, v);
}

void C67_SHRU(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "SHRU.S", r, v, v);
}

void C67_SHR(TCCState *tcc_state, int r, int v)
{
	C67_asm(tcc_state, "SHR.S", r, v, v);
}



/* load 'r' from value 'sv' */
void load(TCCState *tcc_state, int r, SValue * sv)
{
	int v, t, ft, fc, fr, size = 0, element;
	BOOL Unsigned = FALSE;
	SValue v1;

	fr = sv->r;
	ft = sv->type.t;
	fc = sv->c.ul;

	v = fr & VT_VALMASK;
	if (fr & VT_LVAL) {
		if (v == VT_LLOCAL) {
			v1.type.t = VT_INT;
			v1.r = VT_LOCAL | VT_LVAL;
			v1.c.ul = fc;
			load(tcc_state, r, &v1);
			fr = r;
		}
		else if ((ft & VT_BTYPE) == VT_LDOUBLE) {
			tcc_error(tcc_state, "long double not supported");
		}
		else if ((ft & VT_TYPE) == VT_BYTE) {
			size = 1;
		}
		else if ((ft & VT_TYPE) == (VT_BYTE | VT_UNSIGNED)) {
			size = 1;
			Unsigned = TRUE;
		}
		else if ((ft & VT_TYPE) == VT_SHORT) {
			size = 2;
		}
		else if ((ft & VT_TYPE) == (VT_SHORT | VT_UNSIGNED)) {
			size = 2;
			Unsigned = TRUE;
		}
		else if ((ft & VT_BTYPE) == VT_DOUBLE) {
			size = 8;
		}
		else {
			size = 4;
		}

		// check if fc is a positive reference on the stack, 
		// if it is tcc is referencing what it thinks is a parameter
		// on the stack, so check if it is really in a register.


		if (v == VT_LOCAL && fc > 0) {
			int stack_pos = 8;

			for (t = 0; t < NoCallArgsPassedOnStack; t++) {
				if (fc == stack_pos)
					break;

				stack_pos += TranslateStackToReg[t];
			}

			// param has been pushed on stack, get it like a local var

			fc = ParamLocOnStack[t] - 8;
		}

		if ((fr & VT_VALMASK) < VT_CONST)	// check for pure indirect
		{
			if (size == 1) {
				if (Unsigned)
					C67_LDBU_PTR(tcc_state, v, r);	// LDBU  *v,r
				else
					C67_LDB_PTR(tcc_state, v, r);	// LDB  *v,r
			}
			else if (size == 2) {
				if (Unsigned)
					C67_LDHU_PTR(tcc_state, v, r);	// LDHU  *v,r
				else
					C67_LDH_PTR(tcc_state, v, r);	// LDH  *v,r
			}
			else if (size == 4) {
				C67_LDW_PTR(tcc_state, v, r);	// LDW  *v,r
			}
			else if (size == 8) {
				C67_LDDW_PTR(tcc_state, v, r);	// LDDW  *v,r
			}

			C67_NOP(tcc_state, 4);		// NOP 4
			return;
		}
		else if (fr & VT_SYM) {
			greloc(tcc_state, tcc_state->cur_text_section, sv->sym, tcc_state->ind, R_C60LO16);	// rem the inst need to be patched
			greloc(tcc_state, tcc_state->cur_text_section, sv->sym, tcc_state->ind + 4, R_C60HI16);


			C67_MVKL(tcc_state, C67_A0, fc);	//r=reg to load,  constant
			C67_MVKH(tcc_state, C67_A0, fc);	//r=reg to load,  constant


			if (size == 1) {
				if (Unsigned)
					C67_LDBU_PTR(tcc_state, C67_A0, r);	// LDBU  *A0,r
				else
					C67_LDB_PTR(tcc_state, C67_A0, r);	// LDB  *A0,r
			}
			else if (size == 2) {
				if (Unsigned)
					C67_LDHU_PTR(tcc_state, C67_A0, r);	// LDHU  *A0,r
				else
					C67_LDH_PTR(tcc_state, C67_A0, r);	// LDH  *A0,r
			}
			else if (size == 4) {
				C67_LDW_PTR(tcc_state, C67_A0, r);	// LDW  *A0,r
			}
			else if (size == 8) {
				C67_LDDW_PTR(tcc_state, C67_A0, r);	// LDDW  *A0,r
			}

			C67_NOP(tcc_state, 4);		// NOP 4
			return;
		}
		else {
			element = size;

			// divide offset in bytes to create element index
			C67_MVKL(tcc_state, C67_A0, (fc / element) + 8 / element);	//r=reg to load,  constant
			C67_MVKH(tcc_state, C67_A0, (fc / element) + 8 / element);	//r=reg to load,  constant

			if (size == 1) {
				if (Unsigned)
					C67_LDBU_SP_A0(tcc_state, r);	// LDBU  r, SP[A0]
				else
					C67_LDB_SP_A0(tcc_state, r);	// LDB  r, SP[A0]
			}
			else if (size == 2) {
				if (Unsigned)
					C67_LDHU_SP_A0(tcc_state, r);	// LDHU  r, SP[A0]
				else
					C67_LDH_SP_A0(tcc_state, r);	// LDH  r, SP[A0]
			}
			else if (size == 4) {
				C67_LDW_SP_A0(tcc_state, r);	// LDW  r, SP[A0]
			}
			else if (size == 8) {
				C67_LDDW_SP_A0(tcc_state, r);	// LDDW  r, SP[A0]
			}


			C67_NOP(tcc_state, 4);		// NOP 4
			return;
		}
	}
	else {
		if (v == VT_CONST) {
			if (fr & VT_SYM) {
				greloc(tcc_state, tcc_state->cur_text_section, sv->sym, tcc_state->ind, R_C60LO16);	// rem the inst need to be patched
				greloc(tcc_state, tcc_state->cur_text_section, sv->sym, tcc_state->ind + 4, R_C60HI16);
			}
			C67_MVKL(tcc_state, r, fc);	//r=reg to load, constant
			C67_MVKH(tcc_state, r, fc);	//r=reg to load, constant
		}
		else if (v == VT_LOCAL) {
			C67_MVKL(tcc_state, r, fc + 8);	//r=reg to load, constant C67 stack points to next free
			C67_MVKH(tcc_state, r, fc + 8);	//r=reg to load, constant
			C67_ADD(tcc_state, C67_FP, r);	// MV v,r   v -> r
		}
		else if (v == VT_CMP) {
			C67_MV(tcc_state, C67_compare_reg, r);	// MV v,r   v -> r
		}
		else if (v == VT_JMP || v == VT_JMPI) {
			t = v & 1;
			C67_B_DISP(tcc_state, 4);	//  Branch with constant displacement, skip over this branch, load, nop, load
			C67_MVKL(tcc_state, r, t);	//  r=reg to load, 0 or 1 (do this while branching)
			C67_NOP(tcc_state, 4);		//  NOP 4
			gsym(tcc_state, fc);		//  modifies other branches to branch here
			C67_MVKL(tcc_state, r, t ^ 1);	//  r=reg to load, 0 or 1
		}
		else if (v != r) {
			C67_MV(tcc_state, v, r);	// MV v,r   v -> r

			if ((ft & VT_BTYPE) == VT_DOUBLE)
				C67_MV(tcc_state, v + 1, r + 1);	// MV v,r   v -> r
		}
	}
}


/* store register 'r' in lvalue 'v' */
void store(TCCState *tcc_state, int r, SValue * v)
{
	int fr, bt, ft, fc, size, t, element;

	ft = v->type.t;
	fc = v->c.ul;
	fr = v->r & VT_VALMASK;
	bt = ft & VT_BTYPE;
	/* XXX: incorrect if float reg to reg */

	if (bt == VT_LDOUBLE) {
		tcc_error(tcc_state, "long double not supported");
	}
	else {
		if (bt == VT_SHORT)
			size = 2;
		else if (bt == VT_BYTE)
			size = 1;
		else if (bt == VT_DOUBLE)
			size = 8;
		else
			size = 4;

		if ((v->r & VT_VALMASK) == VT_CONST) {
			/* constant memory reference */

			if (v->r & VT_SYM) {
				greloc(tcc_state, tcc_state->cur_text_section, v->sym, tcc_state->ind, R_C60LO16);	// rem the inst need to be patched
				greloc(tcc_state, tcc_state->cur_text_section, v->sym, tcc_state->ind + 4, R_C60HI16);
			}
			C67_MVKL(tcc_state, C67_A0, fc);	//r=reg to load,  constant
			C67_MVKH(tcc_state, C67_A0, fc);	//r=reg to load,  constant

			if (size == 1)
				C67_STB_PTR(tcc_state, r, C67_A0);	// STB  r, *A0
			else if (size == 2)
				C67_STH_PTR(tcc_state, r, C67_A0);	// STH  r, *A0
			else if (size == 4 || size == 8)
				C67_STW_PTR(tcc_state, r, C67_A0);	// STW  r, *A0

			if (size == 8)
				C67_STW_PTR_PRE_INC(tcc_state, r + 1, C67_A0, 1);	// STW  r, *+A0[1]
		}
		else if ((v->r & VT_VALMASK) == VT_LOCAL) {
			// check case of storing to passed argument that
			// tcc thinks is on the stack but for C67 is
			// passed as a reg.  However it may have been
			// saved to the stack, if that reg was required
			// for a call to a child function

			if (fc > 0)		// argument ??
			{
				// walk through sizes and figure which param

				int stack_pos = 8;

				for (t = 0; t < NoCallArgsPassedOnStack; t++) {
					if (fc == stack_pos)
						break;

					stack_pos += TranslateStackToReg[t];
				}

				// param has been pushed on stack, get it like a local var
				fc = ParamLocOnStack[t] - 8;
			}

			if (size == 8)
				element = 4;
			else
				element = size;

			// divide offset in bytes to create word index
			C67_MVKL(tcc_state, C67_A0, (fc / element) + 8 / element);	//r=reg to load,  constant
			C67_MVKH(tcc_state, C67_A0, (fc / element) + 8 / element);	//r=reg to load,  constant



			if (size == 1)
				C67_STB_SP_A0(tcc_state, r);	// STB  r, SP[A0]
			else if (size == 2)
				C67_STH_SP_A0(tcc_state, r);	// STH  r, SP[A0]
			else if (size == 4 || size == 8)
				C67_STW_SP_A0(tcc_state, r);	// STW  r, SP[A0]

			if (size == 8) {
				C67_ADDK(tcc_state, 1, C67_A0);	//  ADDK 1,A0
				C67_STW_SP_A0(tcc_state, r + 1);	//  STW  r, SP[A0]
			}
		}
		else {
			if (size == 1)
				C67_STB_PTR(tcc_state, r, fr);	// STB  r, *fr
			else if (size == 2)
				C67_STH_PTR(tcc_state, r, fr);	// STH  r, *fr
			else if (size == 4 || size == 8)
				C67_STW_PTR(tcc_state, r, fr);	// STW  r, *fr

			if (size == 8) {
				C67_STW_PTR_PRE_INC(tcc_state, r + 1, fr, 1);	// STW  r, *+fr[1]
			}
		}
	}
}

/* 'is_jmp' is '1' if it is a jump */
static void gcall_or_jmp(TCCState *tcc_state, int is_jmp)
{
	int r;
	Sym *sym;

	if ((tcc_state->vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
		/* constant case */
		if (tcc_state->vtop->r & VT_SYM) {
			/* relocation case */

			// get add into A0, then start the jump B3

			greloc(tcc_state, tcc_state->cur_text_section, tcc_state->vtop->sym, tcc_state->ind, R_C60LO16);	// rem the inst need to be patched
			greloc(tcc_state, tcc_state->cur_text_section, tcc_state->vtop->sym, tcc_state->ind + 4, R_C60HI16);

			C67_MVKL(tcc_state, C67_A0, 0);	//r=reg to load, constant
			C67_MVKH(tcc_state, C67_A0, 0);	//r=reg to load, constant
			C67_IREG_B_REG(tcc_state, 0, C67_CREG_ZERO, C67_A0);	//  B.S2x  A0

			if (is_jmp) {
				C67_NOP(tcc_state, 5);	// simple jump, just put NOP
			}
			else {
				// Call, must load return address into B3 during delay slots

				sym = get_sym_ref(tcc_state, &tcc_state->char_pointer_type, tcc_state->cur_text_section, tcc_state->ind + 12, 0);	// symbol for return address
				greloc(tcc_state, tcc_state->cur_text_section, sym, tcc_state->ind, R_C60LO16);	// rem the inst need to be patched
				greloc(tcc_state, tcc_state->cur_text_section, sym, tcc_state->ind + 4, R_C60HI16);
				C67_MVKL(tcc_state, C67_B3, 0);	//r=reg to load, constant
				C67_MVKH(tcc_state, C67_B3, 0);	//r=reg to load, constant
				C67_NOP(tcc_state, 3);	// put remaining NOPs
			}
		}
		else {
			/* put an empty PC32 relocation */
			ALWAYS_ASSERT(tcc_state, FALSE);
		}
	}
	else {
		/* otherwise, indirect call */
		r = gv(tcc_state, RC_INT);
		C67_IREG_B_REG(tcc_state, 0, C67_CREG_ZERO, r);	//  B.S2x  r

		if (is_jmp) {
			C67_NOP(tcc_state, 5);		// simple jump, just put NOP
		}
		else {
			// Call, must load return address into B3 during delay slots

			sym = get_sym_ref(tcc_state, &tcc_state->char_pointer_type, tcc_state->cur_text_section, tcc_state->ind + 12, 0);	// symbol for return address
			greloc(tcc_state, tcc_state->cur_text_section, sym, tcc_state->ind, R_C60LO16);	// rem the inst need to be patched
			greloc(tcc_state, tcc_state->cur_text_section, sym, tcc_state->ind + 4, R_C60HI16);
			C67_MVKL(tcc_state, C67_B3, 0);	//r=reg to load, constant
			C67_MVKH(tcc_state, C67_B3, 0);	//r=reg to load, constant
			C67_NOP(tcc_state, 3);		// put remaining NOPs
		}
	}
}

/* Return the number of registers needed to return the struct, or 0 if
returning via struct pointer. */
ST_FUNC int gfunc_sret(TCCState *tcc_state, CType *vt, int variadic, CType *ret, int *ret_align) {
	*ret_align = 1; // Never have to re-align return values for x86-64
	return 0;
}

/* generate function call with address in (vtop->t, vtop->c) and free function
context. Stack entry is popped */
void gfunc_call(TCCState *tcc_state, int nb_args)
{
	int i, r, size = 0;
	int args_sizes[NoCallArgsPassedOnStack];

	if (nb_args > NoCallArgsPassedOnStack) {
		tcc_error(tcc_state, "more than 10 function params not currently supported");
		// handle more than 10, put some on the stack
	}

	for (i = 0; i < nb_args; i++) {
		if ((tcc_state->vtop->type.t & VT_BTYPE) == VT_STRUCT) {
			ALWAYS_ASSERT(tcc_state, FALSE);
		}
		else {
			/* simple type (currently always same size) */
			/* XXX: implicit cast ? */


			if ((tcc_state->vtop->type.t & VT_BTYPE) == VT_LLONG) {
				tcc_error(tcc_state, "long long not supported");
			}
			else if ((tcc_state->vtop->type.t & VT_BTYPE) == VT_LDOUBLE) {
				tcc_error(tcc_state, "long double not supported");
			}
			else if ((tcc_state->vtop->type.t & VT_BTYPE) == VT_DOUBLE) {
				size = 8;
			}
			else {
				size = 4;
			}

			// put the parameter into the corresponding reg (pair)

			r = gv(tcc_state, RC_C67_A4 << (2 * i));

			// must put on stack because with 1 pass compiler , no way to tell
			// if an up coming nested call might overwrite these regs

			C67_PUSH(tcc_state, r);

			if (size == 8) {
				C67_STW_PTR_PRE_INC(tcc_state, r + 1, C67_SP, 3);	// STW  r, *+SP[3] (go back and put the other)
			}
			args_sizes[i] = size;
		}
		tcc_state->vtop--;
	}
	// POP all the params on the stack into registers for the
	// immediate call (in reverse order)

	for (i = nb_args - 1; i >= 0; i--) {

		if (args_sizes[i] == 8)
			C67_POP_DW(tcc_state, TREG_C67_A4 + i * 2);
		else
			C67_POP(tcc_state, TREG_C67_A4 + i * 2);
	}
	gcall_or_jmp(tcc_state, 0);
	tcc_state->vtop--;
}


// to be compatible with Code Composer for the C67
// the first 10 parameters must be passed in registers
// (pairs for 64 bits) starting wit; A4:A5, then B4:B5 and
// ending with B12:B13.
//
// When a call is made, if the caller has its parameters
// in regs A4-B13 these must be saved before/as the call 
// parameters are loaded and restored upon return (or if/when needed).

/* generate function prolog of type 't' */
void gfunc_prolog(TCCState *tcc_state, CType * func_type)
{
	int addr, align, size, func_call, i;
	Sym *sym;
	CType *type;

	sym = func_type->ref;
	func_call = sym->r;
	addr = 8;
	/* if the function returns a structure, then add an
	implicit pointer parameter */
	tcc_state->func_vt = sym->type;
	tcc_state->func_var = (sym->c == FUNC_ELLIPSIS);
	if ((tcc_state->func_vt.t & VT_BTYPE) == VT_STRUCT) {
		tcc_state->func_vc = addr;
		addr += 4;
	}

	NoOfCurFuncArgs = 0;

	/* define parameters */
	while ((sym = sym->next) != NULL) {
		type = &sym->type;
		sym_push(tcc_state, sym->v & ~SYM_FIELD, type, VT_LOCAL | lvalue_type(type->t), addr);
		size = type_size(type, &align);
		size = (size + 3) & ~3;

		// keep track of size of arguments so
		// we can translate where tcc thinks they
		// are on the stack into the appropriate reg

		TranslateStackToReg[NoOfCurFuncArgs] = size;
		NoOfCurFuncArgs++;

#ifdef FUNC_STRUCT_PARAM_AS_PTR
		/* structs are passed as pointer */
		if ((type->t & VT_BTYPE) == VT_STRUCT) {
			size = 4;
		}
#endif
		addr += size;
	}
	func_ret_sub = 0;
	/* pascal type call ? */
	if (func_call == FUNC_STDCALL)
		func_ret_sub = addr - 8;

	C67_MV(tcc_state, C67_FP, C67_A0);	//  move FP -> A0
	C67_MV(tcc_state, C67_SP, C67_FP);	//  move SP -> FP

	// place all the args passed in regs onto the stack

	tcc_state->loc = 0;
	for (i = 0; i < NoOfCurFuncArgs; i++) {

		ParamLocOnStack[i] = tcc_state->loc;	// remember where the param is
		tcc_state->loc += -8;

		C67_PUSH(tcc_state, TREG_C67_A4 + i * 2);

		if (TranslateStackToReg[i] == 8) {
			C67_STW_PTR_PRE_INC(tcc_state, TREG_C67_A4 + i * 2 + 1, C67_SP, 3);	// STW  r, *+SP[1] (go back and put the other)
		}
	}

	TotalBytesPushedOnStack = -tcc_state->loc;

	func_sub_sp_offset = tcc_state->ind;	// remember where we put the stack instruction 
	C67_ADDK(tcc_state, 0, C67_SP);	//  ADDK.L2 loc,SP  (just put zero temporarily)

	C67_PUSH(tcc_state, C67_A0);
	C67_PUSH(tcc_state, C67_B3);
}

/* generate function epilog */
void gfunc_epilog(TCCState *tcc_state)
{
	{
		int local = (-tcc_state->loc + 7) & -8;	// stack must stay aligned to 8 bytes for LDDW instr
		C67_POP(tcc_state, C67_B3);
		C67_NOP(tcc_state, 4);		// NOP wait for load
		C67_IREG_B_REG(tcc_state, 0, C67_CREG_ZERO, C67_B3);	//  B.S2  B3
		C67_POP(tcc_state, C67_FP);
		C67_ADDK(tcc_state, local, C67_SP);	//  ADDK.L2 loc,SP  
		C67_Adjust_ADDK(tcc_state, 
			(int *)(tcc_state->cur_text_section->data +
			func_sub_sp_offset),
			-local + TotalBytesPushedOnStack);
		C67_NOP(tcc_state, 3);		// NOP 
	}
}

/* generate a jump to a label */
int gjmp(TCCState *tcc_state, int t)
{
	int ind1 = tcc_state->ind;

	C67_MVKL(tcc_state, C67_A0, t);	//r=reg to load,  constant
	C67_MVKH(tcc_state, C67_A0, t);	//r=reg to load,  constant
	C67_IREG_B_REG(tcc_state, 0, C67_CREG_ZERO, C67_A0);	// [!R] B.S2x  A0
	C67_NOP(tcc_state, 5);
	return ind1;
}

/* generate a jump to a fixed address */
void gjmp_addr(TCCState *tcc_state, int a)
{
	Sym *sym;
	// I guess this routine is used for relative short
	// local jumps, for now just handle it as the general
	// case

	// define a label that will be relocated

	sym = get_sym_ref(tcc_state, &tcc_state->char_pointer_type, tcc_state->cur_text_section, a, 0);
	greloc(tcc_state, tcc_state->cur_text_section, sym, tcc_state->ind, R_C60LO16);
	greloc(tcc_state, tcc_state->cur_text_section, sym, tcc_state->ind + 4, R_C60HI16);

	gjmp(tcc_state, 0);			// place a zero there later the symbol will be added to it
}

/* generate a test. set 'inv' to invert test. Stack entry is popped */
int gtst(TCCState *tcc_state, int inv, int t)
{
	int ind1, n;
	int v, *p;

	v = tcc_state->vtop->r & VT_VALMASK;
	if (v == VT_CMP) {
		/* fast case : can jump directly since flags are set */
		// C67 uses B2 sort of as flags register
		ind1 = tcc_state->ind;
		C67_MVKL(tcc_state, C67_A0, t);	//r=reg to load, constant
		C67_MVKH(tcc_state, C67_A0, t);	//r=reg to load, constant

		if (C67_compare_reg != TREG_EAX &&	// check if not already in a conditional test reg
			C67_compare_reg != TREG_EDX &&
			C67_compare_reg != TREG_ST0 && C67_compare_reg != C67_B2) {
			C67_MV(tcc_state, C67_compare_reg, C67_B2);
			C67_compare_reg = C67_B2;
		}

		C67_IREG_B_REG(tcc_state, C67_invert_test ^ inv, C67_compare_reg, C67_A0);	// [!R] B.S2x  A0
		C67_NOP(tcc_state, 5);
		t = ind1;		//return where we need to patch

	}
	else { /* VT_JMP || VT_JMPI */
		/* && or || optimization */
		if ((v & 1) == inv) {
			/* insert vtop->c jump list in t */
			p = &tcc_state->vtop->c.i;

			// I guess the idea is to traverse to the
			// null at the end of the list and store t
			// there

			n = *p;
			while (n != 0) {
				p = (int *)(tcc_state->cur_text_section->data + n);

				// extract 32 bit address from MVKH/MVKL
				n = ((*p >> 7) & 0xffff);
				n |= ((*(p + 1) >> 7) & 0xffff) << 16;
			}
			*p |= (t & 0xffff) << 7;
			*(p + 1) |= ((t >> 16) & 0xffff) << 7;
			t = tcc_state->vtop->c.i;

		}
		else {
			t = gjmp(tcc_state, t);
			gsym(tcc_state, tcc_state->vtop->c.i);
		}
	}
	tcc_state->vtop--;
	return t;
}

/* generate an integer binary operation */
void gen_opi(TCCState *tcc_state, int op)
{
	int r, fr, opc, t;

	switch (op) {
	case '+':
	case TOK_ADDC1:		/* add with carry generation */
		opc = 0;
	gen_op8:


		// C67 can't do const compares, must load into a reg
		// so just go to gv2 directly - tktk



		if (op >= TOK_ULT && op <= TOK_GT)
			gv2(tcc_state, RC_INT_BSIDE, RC_INT);	// make sure r (src1) is on the B Side of CPU
		else
			gv2(tcc_state, RC_INT, RC_INT);

		r = tcc_state->vtop[-1].r;
		fr = tcc_state->vtop[0].r;

		C67_compare_reg = C67_B2;


		if (op == TOK_LT) {
			C67_CMPLT(tcc_state, r, fr, C67_B2);
			C67_invert_test = FALSE;
		}
		else if (op == TOK_GE) {
			C67_CMPLT(tcc_state, r, fr, C67_B2);
			C67_invert_test = TRUE;
		}
		else if (op == TOK_GT) {
			C67_CMPGT(tcc_state, r, fr, C67_B2);
			C67_invert_test = FALSE;
		}
		else if (op == TOK_LE) {
			C67_CMPGT(tcc_state, r, fr, C67_B2);
			C67_invert_test = TRUE;
		}
		else if (op == TOK_EQ) {
			C67_CMPEQ(tcc_state, r, fr, C67_B2);
			C67_invert_test = FALSE;
		}
		else if (op == TOK_NE) {
			C67_CMPEQ(tcc_state, r, fr, C67_B2);
			C67_invert_test = TRUE;
		}
		else if (op == TOK_ULT) {
			C67_CMPLTU(tcc_state, r, fr, C67_B2);
			C67_invert_test = FALSE;
		}
		else if (op == TOK_UGE) {
			C67_CMPLTU(tcc_state, r, fr, C67_B2);
			C67_invert_test = TRUE;
		}
		else if (op == TOK_UGT) {
			C67_CMPGTU(tcc_state, r, fr, C67_B2);
			C67_invert_test = FALSE;
		}
		else if (op == TOK_ULE) {
			C67_CMPGTU(tcc_state, r, fr, C67_B2);
			C67_invert_test = TRUE;
		}
		else if (op == '+')
			C67_ADD(tcc_state, fr, r);	// ADD  r,fr,r
		else if (op == '-')
			C67_SUB(tcc_state, fr, r);	// SUB  r,fr,r
		else if (op == '&')
			C67_AND(tcc_state, fr, r);	// AND  r,fr,r
		else if (op == '|')
			C67_OR(tcc_state, fr, r);	// OR  r,fr,r
		else if (op == '^')
			C67_XOR(tcc_state, fr, r);	// XOR  r,fr,r
		else
			ALWAYS_ASSERT(tcc_state, FALSE);

		tcc_state->vtop--;
		if (op >= TOK_ULT && op <= TOK_GT) {
			tcc_state->vtop->r = VT_CMP;
			tcc_state->vtop->c.i = op;
		}
		break;
	case '-':
	case TOK_SUBC1:		/* sub with carry generation */
		opc = 5;
		goto gen_op8;
	case TOK_ADDC2:		/* add with carry use */
		opc = 2;
		goto gen_op8;
	case TOK_SUBC2:		/* sub with carry use */
		opc = 3;
		goto gen_op8;
	case '&':
		opc = 4;
		goto gen_op8;
	case '^':
		opc = 6;
		goto gen_op8;
	case '|':
		opc = 1;
		goto gen_op8;
	case '*':
	case TOK_UMULL:
		gv2(tcc_state, RC_INT, RC_INT);
		r = tcc_state->vtop[-1].r;
		fr = tcc_state->vtop[0].r;
		tcc_state->vtop--;
		C67_MPYI(tcc_state, fr, r);	// 32 bit bultiply  fr,r,fr
		C67_NOP(tcc_state, 8);		// NOP 8 for worst case
		break;
	case TOK_SHL:
		gv2(tcc_state, RC_INT_BSIDE, RC_INT_BSIDE);	// shift amount must be on same side as dst
		r = tcc_state->vtop[-1].r;
		fr = tcc_state->vtop[0].r;
		tcc_state->vtop--;
		C67_SHL(tcc_state, fr, r);		// arithmetic/logical shift
		break;

	case TOK_SHR:
		gv2(tcc_state, RC_INT_BSIDE, RC_INT_BSIDE);	// shift amount must be on same side as dst
		r = tcc_state->vtop[-1].r;
		fr = tcc_state->vtop[0].r;
		tcc_state->vtop--;
		C67_SHRU(tcc_state, fr, r);	// logical shift
		break;

	case TOK_SAR:
		gv2(tcc_state, RC_INT_BSIDE, RC_INT_BSIDE);	// shift amount must be on same side as dst
		r = tcc_state->vtop[-1].r;
		fr = tcc_state->vtop[0].r;
		tcc_state->vtop--;
		C67_SHR(tcc_state, fr, r);		// arithmetic shift
		break;

	case '/':
		t = TOK__divi;
	call_func:
		vswap(tcc_state);
		/* call generic idiv function */
		vpush_global_sym(tcc_state, &tcc_state->func_old_type, t);
		vrott(tcc_state, 3);
		gfunc_call(tcc_state, 2);
		vpushi(tcc_state, 0);
		tcc_state->vtop->r = REG_IRET;
		tcc_state->vtop->r2 = VT_CONST;
		break;
	case TOK_UDIV:
	case TOK_PDIV:
		t = TOK__divu;
		goto call_func;
	case '%':
		t = TOK__remi;
		goto call_func;
	case TOK_UMOD:
		t = TOK__remu;
		goto call_func;

	default:
		opc = 7;
		goto gen_op8;
	}
}

/* generate a floating point operation 'v = t1 op t2' instruction. The
two operands are guaranted to have the same floating point type */
/* XXX: need to use ST1 too */
void gen_opf(TCCState *tcc_state, int op)
{
	int ft, fc, fr, r;

	if (op >= TOK_ULT && op <= TOK_GT)
		gv2(tcc_state, RC_EDX, RC_EAX);	// make sure src2 is on b side
	else
		gv2(tcc_state, RC_FLOAT, RC_FLOAT);	// make sure src2 is on b side

	ft = tcc_state->vtop->type.t;
	fc = tcc_state->vtop->c.ul;
	r = tcc_state->vtop->r;
	fr = tcc_state->vtop[-1].r;


	if ((ft & VT_BTYPE) == VT_LDOUBLE)
		tcc_error(tcc_state, "long doubles not supported");

	if (op >= TOK_ULT && op <= TOK_GT) {

		r = tcc_state->vtop[-1].r;
		fr = tcc_state->vtop[0].r;

		C67_compare_reg = C67_B2;

		if (op == TOK_LT) {
			if ((ft & VT_BTYPE) == VT_DOUBLE)
				C67_CMPLTDP(tcc_state, r, fr, C67_B2);
			else
				C67_CMPLTSP(tcc_state, r, fr, C67_B2);

			C67_invert_test = FALSE;
		}
		else if (op == TOK_GE) {
			if ((ft & VT_BTYPE) == VT_DOUBLE)
				C67_CMPLTDP(tcc_state, r, fr, C67_B2);
			else
				C67_CMPLTSP(tcc_state, r, fr, C67_B2);

			C67_invert_test = TRUE;
		}
		else if (op == TOK_GT) {
			if ((ft & VT_BTYPE) == VT_DOUBLE)
				C67_CMPGTDP(tcc_state, r, fr, C67_B2);
			else
				C67_CMPGTSP(tcc_state, r, fr, C67_B2);

			C67_invert_test = FALSE;
		}
		else if (op == TOK_LE) {
			if ((ft & VT_BTYPE) == VT_DOUBLE)
				C67_CMPGTDP(tcc_state, r, fr, C67_B2);
			else
				C67_CMPGTSP(tcc_state, r, fr, C67_B2);

			C67_invert_test = TRUE;
		}
		else if (op == TOK_EQ) {
			if ((ft & VT_BTYPE) == VT_DOUBLE)
				C67_CMPEQDP(tcc_state, r, fr, C67_B2);
			else
				C67_CMPEQSP(tcc_state, r, fr, C67_B2);

			C67_invert_test = FALSE;
		}
		else if (op == TOK_NE) {
			if ((ft & VT_BTYPE) == VT_DOUBLE)
				C67_CMPEQDP(tcc_state, r, fr, C67_B2);
			else
				C67_CMPEQSP(tcc_state, r, fr, C67_B2);

			C67_invert_test = TRUE;
		}
		else {
			ALWAYS_ASSERT(tcc_state, FALSE);
		}
		tcc_state->vtop->r = VT_CMP;	// tell TCC that result is in "flags" actually B2
	}
	else {
		if (op == '+') {
			if ((ft & VT_BTYPE) == VT_DOUBLE) {
				C67_ADDDP(tcc_state, r, fr);	// ADD  fr,r,fr
				C67_NOP(tcc_state, 6);
			}
			else {
				C67_ADDSP(tcc_state, r, fr);	// ADD  fr,r,fr
				C67_NOP(tcc_state, 3);
			}
			tcc_state->vtop--;
		}
		else if (op == '-') {
			if ((ft & VT_BTYPE) == VT_DOUBLE) {
				C67_SUBDP(tcc_state, r, fr);	// SUB  fr,r,fr
				C67_NOP(tcc_state, 6);
			}
			else {
				C67_SUBSP(tcc_state, r, fr);	// SUB  fr,r,fr
				C67_NOP(tcc_state, 3);
			}
			tcc_state->vtop--;
		}
		else if (op == '*') {
			if ((ft & VT_BTYPE) == VT_DOUBLE) {
				C67_MPYDP(tcc_state, r, fr);	// MPY  fr,r,fr
				C67_NOP(tcc_state, 9);
			}
			else {
				C67_MPYSP(tcc_state, r, fr);	// MPY  fr,r,fr
				C67_NOP(tcc_state, 3);
			}
			tcc_state->vtop--;
		}
		else if (op == '/') {
			if ((ft & VT_BTYPE) == VT_DOUBLE) {
				// must call intrinsic DP floating point divide
				vswap(tcc_state);
				/* call generic idiv function */
				vpush_global_sym(tcc_state, &tcc_state->func_old_type, TOK__divd);
				vrott(tcc_state, 3);
				gfunc_call(tcc_state, 2);
				vpushi(tcc_state, 0);
				tcc_state->vtop->r = REG_FRET;
				tcc_state->vtop->r2 = REG_LRET;

			}
			else {
				// must call intrinsic SP floating point divide
				vswap(tcc_state);
				/* call generic idiv function */
				vpush_global_sym(tcc_state, &tcc_state->func_old_type, TOK__divf);
				vrott(tcc_state, 3);
				gfunc_call(tcc_state, 2);
				vpushi(tcc_state, 0);
				tcc_state->vtop->r = REG_FRET;
				tcc_state->vtop->r2 = VT_CONST;
			}
		}
		else
			ALWAYS_ASSERT(tcc_state, FALSE);


	}
}


/* convert integers to fp 't' type. Must handle 'int', 'unsigned int'
and 'long long' cases. */
void gen_cvt_itof(TCCState *tcc_state, int t)
{
	int r;

	gv(tcc_state, RC_INT);
	r = tcc_state->vtop->r;

	if ((t & VT_BTYPE) == VT_DOUBLE) {
		if (t & VT_UNSIGNED)
			C67_INTDPU(tcc_state, r, r);
		else
			C67_INTDP(tcc_state, r, r);

		C67_NOP(tcc_state, 4);
		tcc_state->vtop->type.t = VT_DOUBLE;
	}
	else {
		if (t & VT_UNSIGNED)
			C67_INTSPU(tcc_state, r, r);
		else
			C67_INTSP(tcc_state, r, r);
		C67_NOP(tcc_state, 3);
		tcc_state->vtop->type.t = VT_FLOAT;
	}

}

/* convert fp to int 't' type */
/* XXX: handle long long case */
void gen_cvt_ftoi(TCCState *tcc_state, int t)
{
	int r;

	gv(tcc_state, RC_FLOAT);
	r = tcc_state->vtop->r;

	if (t != VT_INT)
		tcc_error(tcc_state, "long long not supported");
	else {
		if ((tcc_state->vtop->type.t & VT_BTYPE) == VT_DOUBLE) {
			C67_DPTRUNC(tcc_state, r, r);
			C67_NOP(tcc_state, 3);
		}
		else {
			C67_SPTRUNC(tcc_state, r, r);
			C67_NOP(tcc_state, 3);
		}

		tcc_state->vtop->type.t = VT_INT;

	}
}

/* convert from one floating point type to another */
void gen_cvt_ftof(TCCState *tcc_state, int t)
{
	int r, r2;

	if ((tcc_state->vtop->type.t & VT_BTYPE) == VT_DOUBLE &&
		(t & VT_BTYPE) == VT_FLOAT) {
		// convert double to float

		gv(tcc_state, RC_FLOAT);		// get it in a register pair

		r = tcc_state->vtop->r;

		C67_DPSP(tcc_state, r, r);		// convert it to SP same register
		C67_NOP(tcc_state, 3);

		tcc_state->vtop->type.t = VT_FLOAT;
		tcc_state->vtop->r2 = VT_CONST;	// set this as unused
	}
	else if ((tcc_state->vtop->type.t & VT_BTYPE) == VT_FLOAT &&
		(t & VT_BTYPE) == VT_DOUBLE) {
		// convert float to double

		gv(tcc_state, RC_FLOAT);		// get it in a register

		r = tcc_state->vtop->r;

		if (r == TREG_EAX) {	// make sure the paired reg is avail
			r2 = get_reg(tcc_state, RC_ECX);
		}
		else if (r == TREG_EDX) {
			r2 = get_reg(tcc_state, RC_ST0);
		}
		else {
			ALWAYS_ASSERT(tcc_state, FALSE);
			r2 = 0; /* avoid warning */
		}

		C67_SPDP(tcc_state, r, r);		// convert it to DP same register
		C67_NOP(tcc_state, 1);

		tcc_state->vtop->type.t = VT_DOUBLE;
		tcc_state->vtop->r2 = r2;		// set this as unused
	}
	else {
		ALWAYS_ASSERT(tcc_state, FALSE);
	}
}

/* computed goto support */
void ggoto(TCCState *tcc_state)
{
	gcall_or_jmp(tcc_state, 1);
	tcc_state->vtop--;
}

/* Save the stack pointer onto the stack and return the location of its address */
ST_FUNC void gen_vla_sp_save(TCCState *tcc_state, int addr) {
	tcc_error(tcc_state, "variable length arrays unsupported for this target");
}

/* Restore the SP from a location on the stack */
ST_FUNC void gen_vla_sp_restore(TCCState *tcc_state, int addr) {
	tcc_error(tcc_state, "variable length arrays unsupported for this target");
}

/* Subtract from the stack pointer, and push the resulting value onto the stack */
ST_FUNC void gen_vla_alloc(TCCState *tcc_state, CType *type, int align) {
	tcc_error(tcc_state, "variable length arrays unsupported for this target");
}

/* end of C67 code generator */
/*************************************************************/
#endif
/*************************************************************/
