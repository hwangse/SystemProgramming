#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<error.h>

#define True 1
#define False 0
#define MAX_ADDR 1048576

typedef struct node{
	char h_order[81];
	struct node* next;
}Node;

typedef struct queue{
	Node* head;
	Node* tail;
	int num;
}Queue;

typedef struct hashnode {
	unsigned int hexa;
	char mnemonic[8];
	char option[5];
	struct hashnode* next;
}hashNode;

/* initialization functions */
void init_queue(Queue*);
void init_memory(char a[][16]);
void create_hashTable();

/* valid command functions */
void func_help(void);
void func_history(Queue *);
 int func_dir(void);
void func_dump(char a[][16],int start,int end,int *,int *);
void func_fill(char a[][16],int start,int end,int val);
void func_quit(Queue*);
void func_opcodelist();
unsigned int func_opcode_mnemonic(char *, int,char *);

/* additional functions[ project1 ] */
void add_Node_To_Queue(char *,Queue*);
void divide_str(char*,char*,int*,int *,int *,char *,char *,char *);
 int dec_to_hexa_digit(int a);
 int correct_Addr(char a);
 int find_group(char *);
 int get_string(char *,char *,int *,int);
 int test_tail(char *,int );
 int string_to_number(char *,int );
 int func_hash(char *);
 int IS_VALID(char a);
 int IS_NUMBER(char a);
 int IS_UPPER(char a);

/***************** project2 에 추가된 함수와 자료구조 ******************/
typedef enum type{BYTE,WORD,RESB,RESW,END,OP1,OP2,OP3,OP_PLUS,
COMMENT,BASE,NOBASE,BLANK}what_type;

typedef enum r{A,X,L,B,S,T,F,PC=8,SW=9}Register;

typedef enum err{get,tail,directive,mnemonic,boundary,estab,record,obj,notError,already,idx}ERROR;

/* additional functions */
int func_type(char *);
int Pow10(int a);
int pass1(char *filename);
int if_directive(FILE *fp,char* origin,char *order,int *locctr,int *start);
int if_mnemonic(FILE* fp,char *origin,char *mnemo,int *locctr,int start);
int string_to_number2(char *tmp);
void add_node_to_symTable(char *,int );
void func_symbol();
void delete_symbolTable();
int pass2(char *filename);
void loc_write_to_file(FILE*fp,int locctr,int flag);
int OP2_choose_reg(char* reg,char *objcode);
void decInt_to_hexStr(int a,char *str,int flag);
int find_symbol(char *);

/* additional data structures */
typedef struct sym_node{
	char symbol[20];
	int line_addr;
	struct sym_node *next;
}symbolNode;

typedef struct mod{
	int addr;
	struct mod *next;
}modeNode;

typedef struct m{
	int num;
	struct mod* head;
}modHead;
/* linking loader기능에서 새로 추가된 함수*/
typedef struct symbol{
	char symbol[10];
	int address;
	struct symbol* next;
}estab_node;		//load map의 노드 (link loader pass1에 사용)

typedef struct rr{
	int index;
	int addr;
}R_node;			//Rrecord에서 배열 생성시 사용됨

typedef struct bp{
	int addr;
	struct bp* next;
}bp_node;

typedef struct b{
	bp_node* head;
	int num;
}bp_head;

int loader_pass1(int num, char *file1,char *file2,char *file3);
int loader_pass2(char memory[][16],int num, char* file1,char *file2,char *file3);
void add_node_to_estable(estab_node*, char*,int address);
int find_node_in_estab(char*, int ,int ,int );
int get_string_len6(char*origin,char*tmp,int *mark,int Hbyte,int type);
void delete_ESTAB();
void twos_complement(char *);
int func_bp(bp_head* bp,int a);
int delete_bp_list(bp_head* bp);
int func_run(char* str,int *reg_list);
void init_reg_list(int *a);
int find_opcode(int opcode,char *);
