/* GCC backend definitions for the TI MSP430 Processor
   Copyright (C) 2012-2025 Free Software Foundation, Inc.
   Contributed by Red Hat.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */


/* Run-time Target Specification */

/* True if the MSP430x extensions are enabled.  */
#ifndef IN_LIBGCC2
extern bool msp430x;
#endif

#define TARGET_CPU_CPP_BUILTINS()		\
  do						\
    {						\
      builtin_define ("NO_TRAMPOLINES");	\
      builtin_define ("__MSP430__"); 		\
      builtin_define (msp430_mcu_name ());	\
      if (msp430x)				\
	{					\
	  builtin_define ("__MSP430X__");	\
	  builtin_assert ("cpu=MSP430X");	\
	  if (TARGET_LARGE)			\
	    builtin_define ("__MSP430X_LARGE__");	\
	}					\
      else					\
	builtin_assert ("cpu=MSP430"); 		\
    }						\
  while (0)

/* For the "c" language where exceptions are implicitly disabled, use
   crt*_no_eh.o unless -fexceptions is passed.  For other languages, only use
   crt*_no_eh.o if -fno-exceptions is explicitly passed.  */
#undef  STARTFILE_SPEC
#define STARTFILE_SPEC "%{pg:gcrt0.o%s}" \
  "%{!pg:%{minrt:crt0-minrt.o%s}%{!minrt:crt0.o%s}} " \
  "%{!minrt:%{,c:%{!fexceptions:crtbegin_no_eh.o%s; :crtbegin.o%s}; " \
    ":%{fno-exceptions:crtbegin_no_eh.o%s; :crtbegin.o%s}}}"

/* -lgcc is included because crtend.o needs __mspabi_func_epilog_1.  */
#undef  ENDFILE_SPEC
#define ENDFILE_SPEC \
  "%{!minrt:%{,c:%{!fexceptions:crtend_no_eh.o%s; :crtend.o%s}; " \
    ":%{fno-exceptions:crtend_no_eh.o%s; :crtend.o%s}}} " \
  "%{minrt:%:if-exists(crtn-minrt.o%s)}%{!minrt:%:if-exists(crtn.o%s)} -lgcc"

#define ASM_SPEC "-mP " /* Enable polymorphic instructions.  */ \
  "%{mcpu=*:-mcpu=%*} " /* Pass the CPU type on to the assembler.  */ \
  "%{mrelax=-mQ} " /* Pass the relax option on to the assembler.  */ \
  /* Tell the assembler if we are building for the LARGE pointer model.  */ \
  "%{mlarge:-ml} " \
  "%{msilicon-errata=*:-msilicon-errata=%*} " \
  "%{msilicon-errata-warn=*:-msilicon-errata-warn=%*} " \
  /* Create DWARF line number sections for -ffunction-sections.  */ \
  "%{ffunction-sections:-gdwarf-sections} " \
  "%{mdata-region=*:-mdata-region=%*} "

/* Enable linker section garbage collection by default, unless we
   are creating a relocatable binary (gc does not work) or debugging
   is enabled  (the GDB testsuite relies upon unused entities not being
   deleted).  */
#define LINK_SPEC "%{mrelax:--relax} %{mlarge:%{!r:%{!g:--gc-sections}}} " \
  "%{mcode-region=*:--code-region=%:" \
    "msp430_propagate_region_opt(%* %{muse-lower-region-prefix})} " \
  "%{mdata-region=*:--data-region=%:" \
    "msp430_propagate_region_opt(%* %{muse-lower-region-prefix})} " \
  "%:msp430_get_linker_devices_include_path() " \
  "%{mtiny-printf:--wrap puts --wrap printf} "

#define DRIVER_SELF_SPECS \
  " %{!mlarge:%{mcode-region=*:%{mdata-region=*:%e-mcode-region and "	\
    "-mdata-region require the large memory model (-mlarge)}}}" \
  " %{!mlarge:%{mcode-region=*:"	\
    "%e-mcode-region requires the large memory model (-mlarge)}}"	\
  " %{!mlarge:%{mdata-region=*:"	\
    "%e-mdata-region requires the large memory model (-mlarge)}}"	\
  " %{mno-warn-devices-csv:%:msp430_set_driver_var(msp430_warn_devices_csv 0)}"\
  " %{mdevices-csv-loc=*:%:msp430_set_driver_var(msp430_devices_csv_loc %*)}"\
  " %{I*:%:msp430_check_path_for_devices(%{I*:%*})}"       \
  " %{L*:%:msp430_check_path_for_devices(%{L*:%*})}"       \
  " %{!mcpu=*:%{mmcu=*:%:msp430_select_cpu(%{mmcu=*:%*})}}"

extern const char * msp430_select_hwmult_lib (int, const char **);
extern const char * msp430_select_cpu (int, const char **);
extern const char * msp430_set_driver_var (int, const char **);
extern const char * msp430_check_path_for_devices (int, const char **);
extern const char *msp430_propagate_region_opt (int, const char **);
extern const char *msp430_get_linker_devices_include_path (int, const char **);

/* There must be a trailing comma after the last item, see gcc.cc
   "static_spec_functions".  */
# define EXTRA_SPEC_FUNCTIONS				\
  { "msp430_hwmult_lib", msp430_select_hwmult_lib },	\
  { "msp430_select_cpu", msp430_select_cpu },		\
  { "msp430_set_driver_var", msp430_set_driver_var },		\
  { "msp430_check_path_for_devices", msp430_check_path_for_devices }, \
  { "msp430_propagate_region_opt", msp430_propagate_region_opt }, \
  { "msp430_get_linker_devices_include_path", \
    msp430_get_linker_devices_include_path },

/* Specify the libraries to include on the linker command line.

   Selecting the hardware multiply library to use is quite complex.
   If the user has specified -mhwmult=FOO then the mapping is quite
   easy (and could be handled here in the SPEC string), unless FOO
   is set to AUTO.  In this case the -mmcu= option must be consulted
   instead.  If the -mhwmult= option is not specified then the -mmcu=
   option must then be examined.  If neither -mhwmult= nor -mmcu= are
   specified then a default hardware multiply library is used.

   Examining the -mmcu=FOO option is difficult, and it is so this
   reason that a spec function is used.  There are so many possible
   values of FOO that a table is used to look up the name and map
   it to a hardware multiply library.  This table (in device-msp430.c)
   must be kept in sync with the same table in msp430.cc.  */
#undef  LIB_SPEC
#define LIB_SPEC "					\
--start-group						\
%{mhwmult=auto:%{mmcu=*:%:msp430_hwmult_lib(mcu %{mmcu=*:%*});\
  :%:msp430_hwmult_lib(default)};			\
  mhwmult=*:%:msp430_hwmult_lib(hwmult %{mhwmult=*:%*}); \
  mmcu=*:%:msp430_hwmult_lib(mcu %{mmcu=*:%*});		\
  :%:msp430_hwmult_lib(default)}			\
-lc							\
-lgcc							\
-lcrt							\
%{msim:-lsim}						\
%{!msim:-lnosys}					\
--end-group					   	\
%{!T*:%{!msim:%{mmcu=*:--script=%*.ld}}}		\
%{!T*:%{msim:%{mlarge:%Tmsp430xl-sim.ld}%{!mlarge:%Tmsp430-sim.ld}}} \
"

/* Storage Layout */

#define BITS_BIG_ENDIAN 		0
#define BYTES_BIG_ENDIAN 		0
#define WORDS_BIG_ENDIAN 		0


#ifdef IN_LIBGCC2
/* This is to get correct SI and DI modes in libgcc2.c (32 and 64 bits).  */
#define	UNITS_PER_WORD			4
/* We have a problem with libgcc2.  It only defines two versions of
   each function, one for "int" and one for "long long".  Ie it assumes
   that "sizeof (int) == sizeof (long)".  For the MSP430 this is not true
   and we need a third set of functions.  We explicitly define
   LIBGCC2_UNITS_PER_WORD here so that it is clear that we are expecting
   to get the SI and DI versions from the libgcc2.c sources, and we
   provide our own set of HI functions, which is why this
   definition is surrounded by #ifndef..#endif.  */
#ifndef LIBGCC2_UNITS_PER_WORD
#define LIBGCC2_UNITS_PER_WORD 		4
#endif
#else
/* Actual width of a word, in units (bytes).  */
#define	UNITS_PER_WORD 			2
#endif

#define SHORT_TYPE_SIZE			16
#define INT_TYPE_SIZE			16
#define LONG_TYPE_SIZE			32
#define LONG_LONG_TYPE_SIZE		64

#define DEFAULT_SIGNED_CHAR		0

#define STRICT_ALIGNMENT 		1
#define FUNCTION_BOUNDARY 		16
#define BIGGEST_ALIGNMENT 		16
#define STACK_BOUNDARY 			16
#define PARM_BOUNDARY 			8
#define PCC_BITFIELD_TYPE_MATTERS	1

#define STACK_GROWS_DOWNWARD		1
#define FRAME_GROWS_DOWNWARD		1
#define FIRST_PARM_OFFSET(FNDECL) 	0

#define MAX_REGS_PER_ADDRESS 		1

#define Pmode 				(TARGET_LARGE ? PSImode : HImode)
#define POINTER_SIZE			(TARGET_LARGE ? 20 : 16)
/* This is just for .eh_frame, to match bfd.  */
#define PTR_SIZE			(TARGET_LARGE ? 4 : 2)
#define	POINTERS_EXTEND_UNSIGNED	1

/* TARGET_VTABLE_ENTRY_ALIGN defaults to POINTER_SIZE, which is 20 for
   TARGET_LARGE.  Pointer alignment is always 16 for MSP430, so set explicitly
   here.  */
#define TARGET_VTABLE_ENTRY_ALIGN 16

#define ADDR_SPACE_NEAR	1
#define ADDR_SPACE_FAR	2

#define REGISTER_TARGET_PRAGMAS() msp430_register_pragmas()

#if 1 /* XXX */
/* Define this macro if it is advisable to hold scalars in registers
   in a wider mode than that declared by the program.  In such cases,
   the value is constrained to be within the bounds of the declared
   type, but kept valid in the wider mode.  The signedness of the
   extension may differ from that of the type.  */

#define PROMOTE_MODE(MODE, UNSIGNEDP, TYPE)	\
  if (GET_MODE_CLASS (MODE) == MODE_INT		\
      && GET_MODE_SIZE (MODE) < 2)      	\
    (MODE) = HImode;
#endif

/* Layout of Source Language Data Types */

#undef  SIZE_TYPE
#define SIZE_TYPE			(TARGET_LARGE \
					 ? "__int20__ unsigned" \
					 : "unsigned int")
#undef  PTRDIFF_TYPE
#define PTRDIFF_TYPE			(TARGET_LARGE ? "__int20__" : "int")
#undef  WCHAR_TYPE
#define WCHAR_TYPE			"long int"
#undef  WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE			BITS_PER_WORD
#define FUNCTION_MODE 			HImode
#define CASE_VECTOR_MODE		Pmode
#define HAS_LONG_COND_BRANCH		0
#define HAS_LONG_UNCOND_BRANCH		0

/* The cost of a branch sequence is roughly 3 "cheap" instructions.  */
#define BRANCH_COST(speed_p, predictable_p) 3

/* Override the default BRANCH_COST heuristic to indicate that it is preferable
   to retain short-circuit operations, this results in significantly better
   codesize and performance.  */
#define LOGICAL_OP_NON_SHORT_CIRCUIT 0

#define LOAD_EXTEND_OP(M)		ZERO_EXTEND
#define WORD_REGISTER_OPERATIONS	1

#define MOVE_MAX 			8

#define INCOMING_RETURN_ADDR_RTX \
  msp430_incoming_return_addr_rtx ()

#define RETURN_ADDR_RTX(COUNT, FA)		\
  msp430_return_addr_rtx (COUNT)

#define SLOW_BYTE_ACCESS		0

/* Calling a constant function address costs the same number of clock
   cycles as calling an address stored in a register. However, in terms of
   instruction length, calling a constant address is more expensive.  */
#define NO_FUNCTION_CSE (optimize >= 2 && !optimize_size)


/* Register Usage */

/* gas doesn't recognize PC (R0), SP (R1), and SR (R2) as register
   names.  */
#define REGISTER_NAMES						\
{								\
  "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",		\
    "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",	\
  "argptr"							\
}

/* Allow lowercase "r" to be used in register names instead of upper
   case "R".  */
#define ADDITIONAL_REGISTER_NAMES	\
{					\
    { "r0",  0 },			\
    { "r1",  1 },			\
    { "r2",  2 },			\
    { "r3",  3 },			\
    { "r4",  4 },			\
    { "r5",  5 },			\
    { "r6",  6 },			\
    { "r7",  7 },			\
    { "r8",  8 },			\
    { "r9",  9 },			\
    { "r10", 10 },			\
    { "r11", 11 },			\
    { "r12", 12 },			\
    { "r13", 13 },			\
    { "r14", 14 },			\
    { "r15", 15 }			\
}

enum reg_class
{
  NO_REGS,
  R12_REGS,
  R13_REGS,
  GEN_REGS,
  ALL_REGS,
  LIM_REG_CLASSES
};

#define REG_CLASS_NAMES \
{			\
  "NO_REGS",		\
  "R12_REGS",		\
  "R13_REGS",		\
  "GEN_REGS",		\
  "ALL_REGS"		\
}

#define REG_CLASS_CONTENTS \
{			   \
  0x00000000,		   \
  0x00001000,		   \
  0x00002000,		   \
  0x0000fff3,		   \
  0x0001ffff		   \
}

/* GENERAL_REGS just means that the "g" and "r" constraints can use these
   registers.
   Even though R0 (PC) and R1 (SP) are not "general" in that they can be used
   for any purpose by the register allocator, they are general in that they can
   be used by any instruction in any addressing mode.  */
#define GENERAL_REGS			GEN_REGS
#define BASE_REG_CLASS  		GEN_REGS
#define INDEX_REG_CLASS			GEN_REGS
#define N_REG_CLASSES			(int) LIM_REG_CLASSES

#define PC_REGNUM			0
#define STACK_POINTER_REGNUM		1
#define CC_REGNUM 			2
#define FRAME_POINTER_REGNUM 		4 /* not usually used, call preserved */
#define ARG_POINTER_REGNUM 		16
#define STATIC_CHAIN_REGNUM 		5 /* FIXME */

#define FIRST_PSEUDO_REGISTER 		17

#define REGNO_REG_CLASS(REGNO)		(REGNO != 2 \
					 && REGNO != 3 \
					 && REGNO < 17 \
					 ? GEN_REGS : NO_REGS)

#define TRAMPOLINE_SIZE			4 /* FIXME */
#define TRAMPOLINE_ALIGNMENT		16 /* FIXME */

#define ELIMINABLE_REGS					\
{{ ARG_POINTER_REGNUM,   STACK_POINTER_REGNUM },	\
 { ARG_POINTER_REGNUM,   FRAME_POINTER_REGNUM },	\
 { FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM }}

#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET)	\
  (OFFSET) = msp430_initial_elimination_offset ((FROM), (TO))


#define FUNCTION_ARG_REGNO_P(N)	  	((N) >= 8 && (N) < ARG_POINTER_REGNUM)
#define DEFAULT_PCC_STRUCT_RETURN	0

/* 1 == register can't be used by gcc, in general
   0 == register can be used by gcc, in general */
#define FIXED_REGISTERS					\
{							\
  1,0,1,1, 0,0,0,0,					\
  0,0,0,0, 0,0,0,0,					\
  1,							\
}

/* 1 == value changes across function calls
   0 == value is the same after a call      */
/* R4 through R10 are callee-saved */
#define CALL_USED_REGISTERS				\
{							\
  1,0,1,1, 0,0,0,0,					\
  0,0,0,1, 1,1,1,1,					\
  1,						\
}

#define REG_ALLOC_ORDER					\
  { 12, 13, 14, 15, 10, 9, 8, 7, 6, 5, 4, 11, 0, 1, 2, 3, 16 }
/*  { 11, 15, 14, 13, 12, 10, 9, 8, 7, 6, 5, 4, 0, 1, 2, 3, 16 }*/

#define REGNO_OK_FOR_BASE_P(regno)	1
#define REGNO_OK_FOR_INDEX_P(regno)	1



typedef struct
{
  /* These two are the current argument status.  */
  char reg_used[4];
#define CA_FIRST_REG 12
  char can_split;
  /* These two are temporaries used internally.  */
  char start_reg;
  char reg_count;
  char mem_count;
  char special_p;
} CUMULATIVE_ARGS;

#define INIT_CUMULATIVE_ARGS(CA, FNTYPE, LIBNAME, INDIRECT, N_NAMED_ARGS) \
  msp430_init_cumulative_args (&CA, FNTYPE, LIBNAME, INDIRECT, N_NAMED_ARGS)


/* FIXME */
#define NO_PROFILE_COUNTERS     1
#define PROFILE_BEFORE_PROLOGUE 1

#define FUNCTION_PROFILER(FILE, LABELNO)	\
    fprintf (FILE, "\tcall\t__mcount\n");

/* Exception Handling */

/* R12,R13,R14 - EH data
   R15 - stack adjustment */

#define EH_RETURN_DATA_REGNO(N) \
  (((N) < 3) ? ((N) + 12) : INVALID_REGNUM)

#define EH_RETURN_HANDLER_RTX \
  gen_rtx_MEM (Pmode, gen_rtx_PLUS (Pmode, gen_rtx_REG (Pmode, SP_REGNO), \
				   gen_rtx_REG (Pmode, 15)))

#define EH_RETURN_STACKADJ_RTX gen_rtx_REG (Pmode, 15)

#define ASM_PREFERRED_EH_DATA_FORMAT(CODE,GLOBAL) DW_EH_PE_udata4


/* Stack Layout and Calling Conventions */


/* Addressing Modes */



#define TEXT_SECTION_ASM_OP ".text"
#define DATA_SECTION_ASM_OP ".data"
#define BSS_SECTION_ASM_OP   "\t.section .bss"

#define ASM_COMMENT_START	" ;"
#define ASM_APP_ON		""
#define ASM_APP_OFF 		""
#define LOCAL_LABEL_PREFIX	".L"
#undef  USER_LABEL_PREFIX
#define USER_LABEL_PREFIX	""

#define GLOBAL_ASM_OP 		"\t.global\t"

#define ASM_OUTPUT_LABELREF(FILE, SYM) msp430_output_labelref ((FILE), (SYM))

#define ASM_OUTPUT_ADDR_VEC_ELT(FILE, VALUE) \
  fprintf (FILE, "\t.long .L%d\n", VALUE)

/* This is how to output an element of a case-vector that is relative.
   Note: The local label referenced by the "3b" below is emitted by
   the tablejump insn.  */

#define ASM_OUTPUT_ADDR_DIFF_ELT(FILE, BODY, VALUE, REL) \
  fprintf (FILE, "\t.long .L%d - 1b\n", VALUE)


#define ASM_OUTPUT_ALIGN(STREAM, LOG)		\
  do						\
    {						\
      if ((LOG) == 0)				\
	break;					\
      fprintf (STREAM, "\t.balign %d\n", 1 << (LOG));	\
    }						\
  while (0)

#define JUMP_TABLES_IN_TEXT_SECTION	1

#undef	DWARF2_ADDR_SIZE
#define	DWARF2_ADDR_SIZE			4

#define INCOMING_FRAME_SP_OFFSET		(TARGET_LARGE ? 4 : 2)

#undef  PREFERRED_DEBUGGING_TYPE
#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG

#define DWARF2_ASM_LINE_DEBUG_INFO		1

/* Prevent reload (and others) from choosing HImode stack slots
   when spilling hard registers when they may contain PSImode values.  */
#define HARD_REGNO_CALLER_SAVE_MODE(REGNO,NREGS,MODE) \
  ((TARGET_LARGE && ((NREGS) <= 2)) ? PSImode \
   : choose_hard_reg_mode ((REGNO), (NREGS), NULL))

#define ACCUMULATE_OUTGOING_ARGS 1

#define HAVE_POST_INCREMENT 1

/* This (unsurprisingly) improves code size in the vast majority of cases, we
   want to prevent any instructions using a "store post increment" from being
   generated.  These will have to later be reloaded since msp430 does not
   support post inc for the destination operand.  */
#define USE_STORE_POST_INCREMENT(MODE)  0

/* Many other targets set USE_LOAD_POST_INCREMENT to 0.  For msp430-elf
   the benefit of disabling it is not clear.  When looking at code size, on
   average, there is a slight advantage to leaving it enabled.  */

#undef  ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL) \
  msp430_start_function ((FILE), (NAME), (DECL))

#define TARGET_HAS_NO_HW_DIVIDE (! TARGET_HWMULT)

void msp430_register_pre_includes (const char *sysroot ATTRIBUTE_UNUSED,
				   const char *iprefix ATTRIBUTE_UNUSED,
				   int stdinc ATTRIBUTE_UNUSED);
#undef TARGET_EXTRA_PRE_INCLUDES
#define TARGET_EXTRA_PRE_INCLUDES msp430_register_pre_includes

#undef  USE_SELECT_SECTION_FOR_FUNCTIONS
#define USE_SELECT_SECTION_FOR_FUNCTIONS 1

#undef ASM_OUTPUT_ALIGNED_DECL_COMMON
#define ASM_OUTPUT_ALIGNED_DECL_COMMON(FILE, DECL, NAME, SIZE, ALIGN)	\
  msp430_output_aligned_decl_common ((FILE), (DECL), (NAME), (SIZE), (ALIGN), 0)

#undef  ASM_OUTPUT_ALIGNED_DECL_LOCAL
#define ASM_OUTPUT_ALIGNED_DECL_LOCAL(FILE, DECL, NAME, SIZE, ALIGN)	\
  msp430_output_aligned_decl_common ((FILE), (DECL), (NAME), (SIZE), (ALIGN), 1)


#define SYMBOL_FLAG_LOW_MEM (SYMBOL_FLAG_MACH_DEP << 0)

#define ADJUST_INSN_LENGTH(insn, length) \
  do	\
    {	\
      if (recog_memoized (insn) >= 0)			\
	{						\
	  length += get_attr_extra_length (insn);	\
	  length *= get_attr_length_multiplier (insn);	\
	}						\
    } while (0)
