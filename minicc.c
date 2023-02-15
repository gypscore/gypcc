#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークンの種類
typedef enum{
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

//なぜ？
typedef struct Token Token;

struct Token{
  TokenKind kind;
  Token *next;
  int val;
  char *str;
};

Token *token;
char *code_head;
//エラー文
void error(Token *token, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  int pos = token->str - code_head;
  fprintf(stderr,"%s\n", code_head);
  fprintf(stderr, "%*s",pos, " ");
  fprintf(stderr,"^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

//トークンの解析
bool consume(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

void expect(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)
    error(token,"'%c'ではありません",op);
  token = token->next;
}

int expect_number(){
  if(token->kind != TK_NUM)
    error(token,"数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;
  while(*p){
    if (isspace(*p)){
      p++;
      continue;
    }

    if(*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED,cur,p++);
      continue;
    }
    if(isdigit(*p)){
      cur = new_token(TK_NUM,cur,p);
      cur->val=strtol(p,&p,10);
      continue;
    }
    error(cur,"トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}


int main(int argc, char **argv){
  if(argc !=2){
    fprintf(stderr, "引数の個数が正しくありません。");
    return 1;
  }
  code_head= argv[1];
  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  //最初のmov命令
  printf("  mov rax, %d\n", expect_number());

  while(!at_eof()){
    if (consume('+')){
      printf("  add rax, %d\n", expect_number());
      continue;
    }
    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }
  printf("  ret\n");
  return 0;
}


