#include <iostream>
using namespace std;

int stack[100];
int esp=100-1;
inline void mod(int&dest,int a,int b){dest=a%b;}
inline void mov(int&dest,int src){dest=src;}
inline void add(int&a,int b){a=a+b;}
inline void sub(int&a,int b){a=a-b;}
inline void div(int&a,int b){a=a/b;}
inline void mod(int&a,int b){a=a%b;}
inline void mul(int&a,int b){a=a*b;}
void push(int v){stack[--esp]=v;}
void pop(int&v){v=stack[esp++];}
#define VAR(unused,var)auto&var=stack[--esp];
int func(int param_a,int param_b)
{
  int eax,ebx,ecx,edx;
  VAR(int,var_c);
  mov(eax,20);
  add(eax,30);
  add(eax,40);
  {
    push(eax);
    mov(eax,50);
    mul(eax,2);
    mov(ebx,eax);
    pop(eax);
  }
  sub(eax,ebx);
  {
    push(eax);
    mov(eax,10);
    mul(eax,5);
    {
      push(eax);
      mov(eax,1);
      add(eax,1);
      mov(ebx,eax);
      pop(eax);
    }
    mul(eax,ebx);
    mod(eax,3);
    mov(ebx,eax);
    pop(eax);
  }
  add(eax,ebx);
  mov(var_c,eax);
  VAR(int,var_n);
  mov(var_n,4);
  mov(eax,var_c);
  {
    push(eax);
    mov(eax,param_a);
    mul(eax,param_b);
    mov(ebx,eax);
    pop(eax);
  }
  add(eax,ebx);
  sub(eax,var_n);
  mov(eax,eax);
  return eax;
}

int main() {
  cout<<func(100,200);
	return 0;
}
