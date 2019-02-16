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
void func_opcode_mnemonic(char *);

/* additional functions */
void add_Node_To_Queue(char *,Queue*);
void divide_str(char*,char*,int*,int *,int *,char *);
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


