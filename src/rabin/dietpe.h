#include "pe.h"

#ifndef DIETPE_H
#define DIETPE_H

typedef struct _dietpe_arm_addr_desc 
{
	unsigned int type;
} dietpe_arm_addr_desc; 

typedef struct _dietpe_arm_code_desc 
{
	unsigned int base;
	unsigned int size;
	dietpe_arm_addr_desc* desc;
} dietpe_arm_code_desc;


typedef struct _dietpe_branch_resolver
{
	unsigned int call_resolver;
	unsigned int b_al_resolver;
	unsigned int b_cond_resolver;
} dietpe_branch_resolver;


typedef struct _dietpe_reg_list
{
	char used[16];
} dietpe_reg_list;

typedef struct _dietpe_func_arm
{
	unsigned int start;
	unsigned int end;
} dietpe_func_arm;


#define PE_ANAL_DESC_NOTANALED 0x00
#define PE_ANAL_DESC_CODE 0x01
#define PE_ANAL_DESC_DATA 0x02

#endif
