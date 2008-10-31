#ifndef _INCLUDE_AASM_H_
#define _INCLUDE_AASM_H_


#define TOKEN_ANDX  0x02000000       /* Just to aid readability below */
#define TOKEN_EORX  0x02200000
#define TOKEN_SUBX  0x02400000
#define TOKEN_RSBX  0x02600000
#define TOKEN_ADDX  0x02800000
#define TOKEN_ADCX  0x02A00000
#define TOKEN_SBCX  0x02C00000
#define TOKEN_RSCX  0x02E00000
#define TOKEN_ORRX  0x03800000
#define TOKEN_MOVX  0x03A00000
#define TOKEN_BICX  0x03C00000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRUE               (0 == 0)
#define FALSE              (0 != 0)

#define MAX_PASSES               20    /* No of reiterations before giving up */
#define SHRINK_STOP (MAX_PASSES - 3)  /* First pass where shrinkage forbidden */

#define IF_STACK_SIZE            10          /* Maximum nesting of IF clauses */

#define SYM_TAB_HASH_BITS         4
#define SYM_TAB_LIST_COUNT       (1 << SYM_TAB_HASH_BITS)
#define SYM_TAB_LIST_MASK        (SYM_TAB_LIST_COUNT - 1)

#define SYM_NAME_MAX             32
#define LINE_LENGTH             256

#define SYM_TAB_CASE_FLAG         1     /* Bit mask for case insensitive flag */
#define SYM_TAB_EXPORT_FLAG       2                       /* Keep whole table */

#define SYM_REC_DEF_FLAG     0x0100     /* Bit mask for `symbol defined' flag */
#define SYM_REC_EXPORT_FLAG  0x0200             /* Bit mask for `export' flag */
#define SYM_REC_EQU_FLAG     0x0400           /* Indicate `type' as EQU (abs) */
#define SYM_REC_THUMB_FLAG   0x0800         /* Indicate label in `Thumb' area */
/* Lowest 8 bits used for pass count */
#define SYM_REC_DATA_FLAG    0x1000                      /* Data space offset */
#define SYM_USER_VARIABLE    0x2000             /* Assembler control variable */

#define ALLOW_ON_FIRST_PASS   0x00010000       /* Bit masks to prevent errors */
#define ALLOW_ON_INTER_PASS   0x00020000       /*  occurring when not wanted. */
#define WARNING_ONLY          0x00040000
#define ALL_EXCEPT_LAST_PASS (ALLOW_ON_FIRST_PASS | ALLOW_ON_INTER_PASS)

#define SYM_NO_ERROR               0
#define SYM_ERR_SYNTAX        0x0100
#define SYM_ERR_NO_MNEM       0x0200
#define SYM_ERR_NO_EQU        0x0300
#define SYM_BAD_REG           0x0400
#define SYM_BAD_REG_COMB      0x0500
#define SYM_NO_REGLIST        0x0600
#define SYM_NO_RSQUIGGLE      0x0700
#define SYM_OORANGE          (0x0800 | ALL_EXCEPT_LAST_PASS)
#define SYM_ENDLESS_STRING    0x0900
#define SYM_DEF_TWICE         0x0A00
#define SYM_NO_COMMA          0x0B00
#define SYM_NO_TABLE          0x0C00
#define SYM_GARBAGE           0x0D00
#define SYM_ERR_NO_EXPORT    (0x0E00 | WARNING_ONLY)
#define SYM_INCONSISTENT      0x0F00
#define SYM_ERR_NO_FILENAME   0x1000
#define SYM_NO_LBR            0x1100
#define SYM_NO_RBR            0x1200
#define SYM_ADDR_MODE_ERR     0x1300
#define SYM_ADDR_MODE_BAD     0x1400
#define SYM_NO_LSQUIGGLE      0x1500
#define SYM_OFFSET_TOO_BIG   (0x1600 | ALL_EXCEPT_LAST_PASS)
#define SYM_BAD_COPRO         0x1700
#define SYM_BAD_VARIANT       0x1800
#define SYM_NO_COND           0x1900
#define SYM_BAD_CP_OP         0x1A00
#define SYM_NO_LABELS        (0x1B00 | WARNING_ONLY)
#define SYM_DOUBLE_ENTRY      0x1C00
#define SYM_NO_INCLUDE        0x1D00
#define SYM_NO_BANG           0x1E00
#define SYM_MISALIGNED       (0x1F00 | ALL_EXCEPT_LAST_PASS)
#define SYM_OORANGE_BRANCH   (0x2000 | ALL_EXCEPT_LAST_PASS)
#define SYM_UNALIGNED_BRANCH (0x2100 | ALL_EXCEPT_LAST_PASS)
#define SYM_VAR_INCONSISTENT  0x2200
#define SYM_NO_IDENTIFIER     0x2300
#define SYM_MANY_IFS          0x2400
#define SYM_MANY_FIS          0x2500
#define SYM_LOST_ELSE         0x2600
#define SYM_NO_HASH           0x2700

#define SYM_ERR_BROKEN        0xFF00                  /* TEMP uncommitted @@@ */

/* evaluate return error states */
/*
#define eval_okay             0x0000		// Rationalise @@@
 */
#define eval_okay             SYM_NO_ERROR
#define eval_no_operand       0x3000
#define eval_no_operator      0x3100
#define eval_not_closebr      0x3200
#define eval_not_openbr       0x3300
#define eval_mathstack_limit  0x3400
#define eval_no_label        (0x3500 | ALLOW_ON_FIRST_PASS)
#define eval_label_undef     (0x3600 | ALL_EXCEPT_LAST_PASS)
#define eval_out_of_radix     0x3700
#define eval_div_by_zero     (0x3800 | ALL_EXCEPT_LAST_PASS)
#define eval_operand_error    0x3900
#define eval_bad_loc_lab      0x3A00

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Literal pool flags                                                         */

#define LIT_DEFINED	0x01     /* Set when a literal pool entry has a value */
#define LIT_NO_DUMP	0x02            /* Set when record needn't be planted */
#define LIT_HALF	0x04                    /* Set when value is halfword */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Expression evaluator                                                       */

#define mathstack_size 20
/* Enumerations used for both unary and binary operators */
#define PLUS                 0
#define MINUS                1
#define NOT                  2
#define MULTIPLY             3
#define DIVIDE               4
#define MODULUS              5
#define CLOSEBR              6
#define LEFT_SHIFT           7
#define RIGHT_SHIFT          8
#define AND                  9
#define OR                  10
#define XOR                 11
#define LOG                 12
#define END                 13

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* List file formatting constants                                             */
#define LIST_LINE_LENGTH   120                           /* Total line length */
#define LIST_LINE_ADDRESS   10                         /* Address field width */
#define LIST_BYTE_COUNT      4                    /* Number of bytes per line */
#define LIST_BYTE_FIELD   (LIST_LINE_ADDRESS + 3 * LIST_BYTE_COUNT + 2)
#define LIST_LINE_LIST    (LIST_LINE_LENGTH  - 1 - LIST_BYTE_FIELD)

#define HEX_LINE_ADDRESS    10
#define HEX_BYTE_COUNT      16
#define HEX_LINE_LENGTH    (HEX_LINE_ADDRESS + 3 * HEX_BYTE_COUNT)

#define ELF_TEMP_LENGTH     20

#define ELF_MACHINE         40                                     /* ARM (?) */
#define ELF_EHSIZE          52                         /* Defined in standard */
#define ELF_PHENTSIZE      (4 * 8)                     /* Defined in standard */
#define ELF_SHENTSIZE      (4 * 10)                    /* Defined in standard */

#define ELF_SHN_ABS        0xFFF1                      /* Defined in standard */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#define	v3	0xFF
#define	v3M	0xFE
#define	v4	0xFC
#define	v4xM	0xFD
#define	v4T	0xF8
#define	v4TxM	0xF9
#define	v5	0xF0
#define	v5xM	0xF1
#define	v5T	0xF0
#define	v5TxM	0xF1
#define	v5TE	0xC0
#define	v5TExP	0xE0

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

typedef int boolean;
typedef enum { ARM, THUMB } instr_set;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

typedef enum { TYPE_BYTE, TYPE_HALF, TYPE_WORD, TYPE_CPRO }  type_size;
typedef enum { NO_LABEL, MAYBE_SYMBOL, SYMBOL, LOCAL_LABEL } label_type;
typedef enum { ALL, EXPORTED, DEFINED, UNDEFINED }           label_category;
typedef enum { ALPHABETIC, VALUE, DEFINITION, FOR_ELF }      label_sort;

typedef enum
{
	SYM_REC_ADDED,      SYM_REC_DEFINED,    SYM_REC_REDEFINED,
	SYM_REC_UNCHANGED,  SYM_REC_ERROR
}
defn_return;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

typedef struct sym_record_name                    /* Symbol record definition */
{
	struct sym_record_name *pNext;                    /* Pointer to next record */
	unsigned int            count;              /* Number of characters in name */
	unsigned int             hash;
	unsigned int            flags;
	unsigned int       identifier;  /* Record identifier, also definition order */
	unsigned int      elf_section;    /* Section number - purely for ELF driver */
	int                     value;
	char       name[SYM_NAME_MAX];              /* Fixed field for name (quick) */
}
sym_record;

typedef struct                              /* Symbol table header definition */
{
	char             *name;
	unsigned int     symbol_number;
	unsigned int     flags;
	sym_record      *pList[SYM_TAB_LIST_COUNT];
}
sym_table;

typedef struct sym_table_item_name   /* So we can make lists of symbol tables */
{
	sym_table                  *pTable;    /* Pointer to symbol table (or NULL) */
	struct sym_table_item_name *pNext;     /* Pointer to next record  (or NULL) */
}
sym_table_item;

typedef struct local_label_name     /* Literal pool element holder definition */
{
	struct local_label_name *pNext;                   /* Pointer to next record */
	struct local_label_name *pPrev;               /* Pointer to previous record */
	unsigned int             label;          /* The value of the label (number) */
	unsigned int             value;                     /* The label word value */
	unsigned int             flags;
}
local_label;

typedef struct own_label_name          /* Definition of label on current line */
{
	label_type                 sort;            /* What `sort' of label, if any */
	struct sym_record_name  *symbol;    /* Pointer to symbol record, if present */
	struct local_label_name  *local;      /* Pointer to local label, if present */
}
own_label;

typedef struct literal_record_name  /* Literal pool element holder definition */
{
	struct literal_record_name *pNext;                /* Pointer to next record */
	unsigned int              address;       /* The address of the literal word */
	unsigned int                value;                /* The literal word value */
	unsigned int                flags;
}
literal_record;

typedef struct elf_temp_name
{
	struct elf_temp_name *pNext;
	boolean               continuation;
	unsigned int          section;
	unsigned int          address;
	unsigned int          count;
	char                  data[ELF_TEMP_LENGTH];
}
elf_temp;

typedef struct elf_info_name                 /* Section info collecting point */
{                                  /* Just the bits I think need collecting */
	struct elf_info_name *pNext;
	unsigned int          name;
	unsigned int          address;
	unsigned int          position;
	unsigned int          size;
}
elf_info;

typedef struct size_record_name          /* Size of variable length operation */
{                                                /*  (form an ordered list) */
	struct size_record_name *pNext;
	unsigned int             size;
}
size_record;


/* functions */

boolean      set_options(int argc, char *argv[]);

boolean      input_line(FILE*, char*, unsigned int);
boolean      parse_mnemonic_line(char*, sym_table*, sym_table*, sym_table*);
unsigned int parse_source_line(char*,sym_table_item*,sym_table*,int,int,char**);
void         print_error(char*, unsigned int, unsigned int, char*, int);
unsigned int assemble_line(char*, unsigned int, unsigned int,
		own_label*, sym_table*, int, int, char**);

int          do_literal(instr_set, type_size, int*, boolean, unsigned int*);
unsigned int find_partials(unsigned int, unsigned int*);
unsigned int variable_item_size(int, unsigned int);

int          get_thing(char*, unsigned int*, sym_table*);
int          get_reg(char*, unsigned int*);
int          get_thumb_reg(char*, unsigned int*, unsigned int);
int          get_creg(char*, unsigned int*);
int          get_copro(char*, unsigned int*);
int          get_psr(char*, unsigned int*);
int          get_shift(char*, unsigned int*);
int          data_op_imm(unsigned int);
unsigned int thumb_pc_load(unsigned int, unsigned int, unsigned int, int, int,
		unsigned int*);

void         redefine_symbol(char*, sym_record*, sym_table*);
void         assemble_redef_label(unsigned int,  int, own_label*,
		unsigned int*, int, int, int, char*);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

unsigned int evaluate(char*, unsigned int*, int*, sym_table*);
int          get_variable(char*, unsigned int*, int*, int*, boolean*, sym_table*);
int          get_operator(char*, unsigned int*, int*, int*);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

sym_table   *sym_create_table(char*, unsigned int);
int          sym_delete_table(sym_table*, boolean);
defn_return  sym_define_label(char*, unsigned int, unsigned int,
		sym_table*, sym_record**);
int          sym_locate_label(char*, unsigned int, sym_table*, sym_record**);
sym_record  *sym_find_label_list(char*, sym_table_item*);
sym_record  *sym_find_label(char*, sym_table*);
sym_record  *sym_create_record(char*, unsigned int, unsigned int, unsigned int);
void         sym_delete_record(sym_record*);
int          sym_delete_record_list(sym_record**, int);
int          sym_add_to_table(sym_table*, sym_record*);
sym_record  *sym_find_record(sym_table*, sym_record*);
void         sym_string_copy(char*, sym_record*, unsigned int);
char        *sym_strtab(sym_record*, unsigned int, unsigned int*);
sym_record  *sym_sort_symbols(sym_table*, label_category, label_sort);
unsigned int sym_count_symbols(sym_table*, label_category);
void         sym_print_table(sym_table*,label_category,label_sort,int,char*);
void         local_label_dump(local_label*, FILE*);
void         lit_print_table(literal_record*, FILE*);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void byte_dump(unsigned int, unsigned int, char*, int);

void literal_dump(int, char*, unsigned int);

FILE *open_output_file(int, char*);
void close_output_file(FILE*, char*, int);
void hex_dump(unsigned int, char);
void hex_dump_flush(void);
void hexpairs_dump(unsigned int address, char value);

void elf_dump(unsigned int, char);
void elf_new_section_maybe(void);
void elf_dump_out(FILE*, sym_table*);

void list_file_out(void);
void list_start_line(unsigned int, int);
void list_mid_line(unsigned int, char*, int);
void list_end_line(char*);
void list_symbols(FILE*, sym_table*);
void list_buffer_init(char*, unsigned int, int);
void list_hex(unsigned int, unsigned int, char*);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

int          skip_spc(char*, int);
char         *file_path(char*);
char         *pathname(char*, char*);
boolean      cmp_next_non_space(char*, int*, int, char);
boolean      test_eol(char);
unsigned int get_identifier(char*, unsigned int, char*, unsigned int);
boolean      alpha_numeric(char);
boolean      alphabetic(char);
int          get_num(char*, unsigned int*, unsigned int*, unsigned int);
int          allow_error(unsigned int, boolean, boolean);

/* GLOBAL WTFU */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern int SYM_RECORD_SIZE;
extern int SYM_TABLE_SIZE;
extern int SYM_TABLE_ITEM_SIZE;
extern int LIT_RECORD_SIZE;
extern int LOCAL_LABEL_SIZE;
extern int ELF_TEMP_SIZE;
extern int ELF_INFO_SIZE;
extern int SIZE_RECORD_SIZE;

/*----------------------------------------------------------------------------*/
/* Global variables                                                           */

extern instr_set  instruction_set;                                   /* ARM or Thumb */
extern char      *input_file_name;
extern char      *symbols_file_name;
extern char      *list_file_name;
extern char      *hex_file_name;
extern char      *elf_file_name;
extern char      *hexpairs_file_name;
extern FILE      *fList, *fHex, *fElf;
extern int        symbols_stdout, list_stdout, hex_stdout, hexpairs_stdout, elf_stdout;   /* Booleans */
extern label_sort symbols_order;
extern int        list_sym, list_kmd;

extern unsigned int sym_print_extras;        /* <0> set for locals, <1> for literals */

extern unsigned int arm_variant;

extern unsigned int assembly_pointer;       /* Address at which to plant instruction */
extern unsigned int def_increment;           /* Offset reached from assembly_pointer */
extern boolean      assembly_pointer_defined;
extern unsigned int entry_address;
extern boolean      entry_address_defined;
extern unsigned int data_pointer;           /* Used for creating record offsets etc. */
extern unsigned int undefined_count;  /* Number of references to undefined variables */
extern unsigned int   defined_count;        /* Number of variables defined this pass */
extern unsigned int redefined_count;      /* Number of variables redefined this pass */
extern unsigned int pass_count;                          /* Pass number, starts at 0 */
extern unsigned int pass_errors;                     /* Errors occurred in this pass */
extern boolean      div_zero_this_pass;  /* Indicates /0 in pass, prevents code dump */
extern boolean      dump_code;         /* Allow output (FALSE on last pass if error) */

extern own_label *evaluate_own_label;	                                /* Yuk! @@@@@ */
/* Because evaluate needs to know if there is a local label on -current- line */

extern literal_record *literal_list;   /* Start of the list of literals from "LDR =" */
extern literal_record *literal_head;           /* The next literal record `expected' */
extern literal_record *literal_tail;             /* The last literal record `dumped' */

extern local_label    *loc_lab_list;            /* Start of the list of local labels */
extern local_label    *loc_lab_position;         /* The current local label position */

extern size_record    *size_record_list;     /* Start of list of ADRL (etc.) lengths */
extern size_record    *size_record_current;          /* Current record in above list */
extern unsigned int    size_changed_count;   /* Number of `instruction' size changes */

extern boolean      if_stack[IF_STACK_SIZE + 1];      /* Used for nesting IF clauses */
extern int if_SP;

extern unsigned int list_address;
extern unsigned int list_byte;
extern unsigned int list_line_position;     /* Pos. in the src line copied to output */
extern char         list_buffer[LIST_LINE_LENGTH];

extern unsigned int hex_address;
extern boolean      hex_address_defined;
extern char         hex_buffer[HEX_LINE_LENGTH];

extern int          elf_section_valid;   /* Flag: true if code dumped in elf_section */
extern unsigned int elf_section;          /* Current elf section number (for labels) */
extern unsigned int elf_section_old;
extern boolean      elf_new_block;

extern elf_temp     *elf_record_list;
extern elf_temp     *current_elf_record;

extern sym_table    *variable_table;                      /* Assemble-time variables */
extern sym_table        *arch_table;    /* Table of possible processor architectures */
extern sym_table    *operator_table;
extern sym_table    *register_table;
extern sym_table   *cregister_table;
extern sym_table       *copro_table;
extern sym_table       *shift_table;

#endif

