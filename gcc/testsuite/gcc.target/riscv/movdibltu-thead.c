/* { dg-do compile { target { ! riscv_abi_e } } } */
/* { dg-require-effective-target rv64 } */
/* { dg-skip-if "" { *-*-* } { "-O0" "-Og" "-Os" "-Oz" } } */
/* { dg-options "-march=rv64gc_xtheadcondmov -mtune=thead-c906 -mbranch-cost=1 -fdump-rtl-ce1" } */

typedef unsigned int __attribute__ ((mode (DI))) int_t;

int_t
movdiltu (int_t w, int_t x, int_t y, int_t z)
{
  return w < x ? y : z;
}

/* Expect branched assembly like:

	bgeu	a0,a1,.L2
	mv	a3,a2
.L2:
	mv	a0,a3
 */

/* { dg-final { scan-rtl-dump-not "Conversion succeeded on pass \[0-9\]+\\." "ce1" } } */
/* { dg-final { scan-rtl-dump-not "if-conversion succeeded through" "ce1" } } */
/* { dg-final { scan-assembler-times "\\s(?:bgeu|bgtu|bleu|bltu)\\s" 1 } } */
/* { dg-final { scan-assembler-not "\\s(?:sgtu|sltu)\\s" } } */
/* { dg-final { scan-assembler-not "\\s(?:th\\.mveqz|th\\.mvnez)\\s" } } */
/* { dg-final { scan-assembler-not "\\s(?:seqz|snez)\\s" } } */
