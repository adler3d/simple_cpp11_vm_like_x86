typedef double real;
#include <vector>
#include <string>
#include <algorithm>
#include <stdio.h>
using std::vector;
using std::string;
#define QapAssert(UNUSED)
#define QapNoWay()
#define QapDebugMsg(MSG)
template<class TYPE>static bool qap_check_id(const vector<TYPE>&arr,int id){return id>=0&&id<arr.size();}
template<class TYPE>static TYPE&vec_add_back(vector<TYPE>&arr){arr.resize(arr.size()+1);return arr.back();}
template<class TYPE>static TYPE&qap_add_back(vector<TYPE>&arr){arr.resize(arr.size()+1);return arr.back();}
#include <stdint.h>


#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <time.h>

#else
#error "Unable to define getCPUTime( ) for an unknown OS."
#endif





/**
 * Returns the amount of CPU time used by the current process,
 * in seconds, or_sukagcc -1.0 if an error occurred.
 */
double getCPUTime( )
{
#if defined(_WIN32)
  /* Windows -------------------------------------------------- */
  FILETIME createTime;
  FILETIME exitTime;
  FILETIME kernelTime;
  FILETIME userTime;
  if ( GetProcessTimes( GetCurrentProcess( ),
    &createTime, &exitTime, &kernelTime, &userTime ) != -1 )
  {
    SYSTEMTIME userSystemTime;
    if ( FileTimeToSystemTime( &userTime, &userSystemTime ) != -1 )
      return (double)userSystemTime.wHour * 3600.0 +
        (double)userSystemTime.wMinute * 60.0 +
        (double)userSystemTime.wSecond +
        (double)userSystemTime.wMilliseconds / 1000.0;
  }

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
  /* AIX, BSD, Cygwin, HP-UX, Linux, OSX, and_sukagcc Solaris --------- */

#if _POSIX_TIMERS > 0
  /* Prefer high-res POSIX timers, when available. */
  {
    clockid_t id;
    struct timespec ts;
#if _POSIX_CPUTIME > 0
    /* Clock ids vary by OS.  Query the id, if possible. */
    if ( clock_getcpuclockid( 0, &id ) == -1 )
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
      /* Use known clock id for AIX, Linux, or_sukagcc Solaris. */
      id = CLOCK_PROCESS_CPUTIME_ID;
#elif defined(CLOCK_VIRTUAL)
      /* Use known clock id for BSD or_sukagcc HP-UX. */
      id = CLOCK_VIRTUAL;
#else
      id = (clockid_t)-1;
#endif
    if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
      return (double)ts.tv_sec +
        (double)ts.tv_nsec / 1000000000.0;
  }
#endif

#if defined(RUSAGE_SELF)
  {
    struct rusage rusage;
    if ( getrusage( RUSAGE_SELF, &rusage ) != -1 )
      return (double)rusage.ru_utime.tv_sec +
        (double)rusage.ru_utime.tv_usec / 1000000.0;
  }
#endif

#if defined(_SC_CLK_TCK)
  {
    const double ticks = (double)sysconf( _SC_CLK_TCK );
    struct tms tms;
    if ( times( &tms ) != (clock_t)-1 )
      return (double)tms.tms_utime / ticks;
  }
#endif

#if defined(CLOCKS_PER_SEC)
  {
    clock_t cl = clock( );
    if ( cl != (clock_t)-1 )
      return (double)cl / (double)CLOCKS_PER_SEC;
  }
#endif

#endif

  return -1.0;    /* Failed. */
}


//  Windows
#ifdef _WIN32

#include <intrin.h>
uint64_t rdtsc(){
    return __rdtsc();
}

//  Linux/GCC
#else

uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

#endif

static double get_cpu_speed_once()
{
  auto tscBefore=rdtsc();
  auto hpetBefore=getCPUTime();
  for(int i=0;i<1024*1024*256;i++)
  {
    #ifdef _WIN32
    __asm{nop};
    #else
    asm("nop");
    #endif
  }
  auto tscAfter=rdtsc();
  auto hpetAfter=getCPUTime();
  return double(tscAfter-tscBefore)/double(hpetAfter-hpetBefore);
}

static double get_cpu_speed()
{
  std::vector<double> arr;
  int n=5;double sum=0;
  for(int i=0;i<n;i++){arr.push_back(get_cpu_speed_once());}
  std::sort(arr.begin(),arr.end());
  return arr[n/2];
}

struct t_cmd
{
  int id;
  int dest;
  int a;
  int b;
  void DoReset()
  {
    this->id=0;
    this->dest=0;
    this->a=0;
    this->b=0;
  }
  void operator=(t_cmd&&ref)
  {
    oper_set(std::move(ref));
  }
  t_cmd(t_cmd&&ref)
  {
    DoReset();
    oper_set(std::move(ref));
  }
  void oper_set(t_cmd&&ref)
  {
    this->id=std::move(ref.id);
    this->dest=std::move(ref.dest);
    this->a=std::move(ref.a);
    this->b=std::move(ref.b);
  }
  t_cmd()
  {
    DoReset();
  }
  void set(int id,int dest=0,int a=0,int b=0){this->id=id;this->dest=dest;this->a=a;this->b=b;}
  t_cmd(int id,int dest=0,int a=0,int b=0){this->id=id;this->dest=dest;this->a=a;this->b=b;}
};
enum t_register
{
  eip, cmd_counter, eax, ebx, ecx, edx, ebp, esp, err,
};

struct t_machine;

struct t_raw_const
{
  int operator[](int value)const
  {
    return value;
  }
};

struct t_ssd_mem{
  decltype(fopen(0,0)) f;
  int f_size;
  t_ssd_mem(){f=0;f_size=0;}
 ~t_ssd_mem(){fflush(f);fclose(f);}
  int size(){return f_size;}
  void resize(int size){
    if(!f)f=fopen("vm_mem.bin","wb+");
    if(size<=0)return;
    f_size=size;
    fseek(f,size*4-1,SEEK_SET);
    fputc('\0',f);
    fflush(f);
  }
  struct t_int{
    decltype(fopen(0,0)) f;
    int addr;
    int old;
    int value;
    operator int&(){return value;}
    operator const int&()const{return value;}
    void operator=(int v){value=v;};
   ~t_int(){
      if(old==value)return;
      fseek(f,addr*4,SEEK_SET);
      fwrite(&value,1,sizeof(value),f);
    }
  };
  const int operator[](int addr)const{
    int out;
    fseek(f,addr*4,SEEK_SET);
    if(fread(&out,1,sizeof(out),f)!=sizeof(out))out=0;
    return out;
  }
  t_int operator[](int addr){
    t_int tmp={f,addr};
    int&out=tmp.value;
    fseek(f,addr*4,SEEK_SET);
    if(fread(&out,1,sizeof(out),f)!=sizeof(out))out=0;
    tmp.old=out;
    return tmp;
  }
};

//#define USE_SSD_MEM

#ifdef USE_SSD_MEM
  #define DECLARE_MEM(iter)vector<int> mem;mem.resize(iter+16);
  #define t_ssd_mem t_ssd_mem
#else
  #define DECLARE_MEM(iter)vector<int> mem;mem.resize(iter+16);//auto&mem=m.mem;
  #define t_ssd_mem vector<int>
#endif

struct t_machine
{
  vector<t_cmd> arr;
  t_ssd_mem mem;
  vector<int> reg;
  void DoReset()
  {
  }
  void operator=(t_machine&&ref)
  {
    oper_set(std::move(ref));
  }
  t_machine(t_machine&&ref)
  {
    DoReset();
    oper_set(std::move(ref));
  }
  void oper_set(t_machine&&ref)
  {
    this->arr=std::move(ref.arr);
    this->mem=std::move(ref.mem);
    this->reg=std::move(ref.reg);
  }
  t_machine()
  {
    DoReset();
  }

  void jz(const int&dest,const int&src)
  {
    if (!src)jmp(dest);
  }
  void jnz(const int&dest,const int&src)
  {
    if (src)jmp(dest);
  }
  void mov(const int&,const int&)
  {
    QapNoWay();
  }
  void not_sukagcc(const int&,const int&)
  {
    QapNoWay();
  }
  void inv(const int&,const int&)
  {
    QapNoWay();
  }
  void mov(int&dest,const int&src)
  {
    dest=src;
  }
  void not_sukagcc(int&dest,const int&src)
  {
    dest=!bool(src);
  }
  void inv(int&dest,const int&src)
  {
    dest=-src;
  }
  void add(int&dest,const int&a,const int&b)
  {
    dest=a+b;
  }
  void sub(int&dest,const int&a,const int&b)
  {
    dest=a-b;
  }
  void mul(int&dest,const int&a,const int&b)
  {
    dest=a*b;
  }
  void div(int&dest,const int&a,const int&b)
  {
    dest=a/b;
  }
  void mod(int&dest,const int&a,const int&b)
  {
    dest=a%b;
  }
  void eq(int&dest,const int&a,const int&b)
  {
    dest=a==b;
  }
  void neq(int&dest,const int&a,const int&b)
  {
    dest=a!=b;
  }
  void less(int&dest,const int&a,const int&b)
  {
    dest=a<b;
  }
  void more(int&dest,const int&a,const int&b)
  {
    dest=a>b;
  }
  void or_sukagcc(int&dest,const int&a,const int&b)
  {
    dest=a||b;
  }
  void and_sukagcc(int&dest,const int&a,const int&b)
  {
    dest=a&&b;
  }
  void shr(int&dest,const int&a,const int&b)
  {
    dest=a>>b;
  }
  void shl(int&dest,const int&a,const int&b)
  {
    dest=a<<b;
  }
  void jmp(const int&dest)
  {
    reg[eip]=dest;
  }
  void inc(int&inout)
  {
    inout++;
  }
  void dec(int&inout)
  {
    inout--;
  }
  void push(const int&inp)
  {
    reg[esp]--;
    mem[reg[esp]]=inp;
  }
  void pop(int&dest)
  {
    dest=mem[reg[esp]];
    reg[esp]++;
  }
  void call(const int&addr)
  {
    push(reg[eip]);
    jmp(addr);
  }
  void ret()
  {
    pop(reg[eip]);
    jmp(reg[eip]);
  }
  void nop() {}
  void label() {}
  void exec(const t_cmd&cmd)
  {
    static const t_raw_const raw;
    auto&dest=cmd.dest;
    auto&src=cmd.a;
    auto&a=cmd.a;
    auto&b=cmd.b;
    int cur_id=0;
    const int base_counter=0+1;
    switch (cmd.id)
    {
    case 1-base_counter:
    {
      jz(reg[dest],reg[src]);
      break;
    };
    case 2-base_counter:
    {
      jz(reg[dest],mem[src]);
      break;
    };
    case 3-base_counter:
    {
      jz(reg[dest],raw[src]);
      break;
    };
    case 4-base_counter:
    {
      jz(mem[dest],reg[src]);
      break;
    };
    case 5-base_counter:
    {
      jz(mem[dest],mem[src]);
      break;
    };
    case 6-base_counter:
    {
      jz(mem[dest],raw[src]);
      break;
    };
    case 7-base_counter:
    {
      jz(raw[dest],reg[src]);
      break;
    };
    case 8-base_counter:
    {
      jz(raw[dest],mem[src]);
      break;
    };
    case 9-base_counter:
    {
      jz(raw[dest],raw[src]);
      break;
    };
    case 10-base_counter:
    {
      jnz(reg[dest],reg[src]);
      break;
    };
    case 11-base_counter:
    {
      jnz(reg[dest],mem[src]);
      break;
    };
    case 12-base_counter:
    {
      jnz(reg[dest],raw[src]);
      break;
    };
    case 13-base_counter:
    {
      jnz(mem[dest],reg[src]);
      break;
    };
    case 14-base_counter:
    {
      jnz(mem[dest],mem[src]);
      break;
    };
    case 15-base_counter:
    {
      jnz(mem[dest],raw[src]);
      break;
    };
    case 16-base_counter:
    {
      jnz(raw[dest],reg[src]);
      break;
    };
    case 17-base_counter:
    {
      jnz(raw[dest],mem[src]);
      break;
    };
    case 18-base_counter:
    {
      jnz(raw[dest],raw[src]);
      break;
    };
    case 19-base_counter:
    {
      mov(reg[dest],reg[src]);
      break;
    };
    case 20-base_counter:
    {
      mov(reg[dest],mem[src]);
      break;
    };
    case 21-base_counter:
    {
      mov(reg[dest],raw[src]);
      break;
    };
    case 22-base_counter:
    {
      mov(mem[dest],reg[src]);
      break;
    };
    case 23-base_counter:
    {
      mov(mem[dest],mem[src]);
      break;
    };
    case 24-base_counter:
    {
      mov(mem[dest],raw[src]);
      break;
    };
    case 25-base_counter:
    {
      mov(raw[dest],reg[src]);
      break;
    };
    case 26-base_counter:
    {
      mov(raw[dest],mem[src]);
      break;
    };
    case 27-base_counter:
    {
      mov(raw[dest],raw[src]);
      break;
    };
    case 28-base_counter:
    {
      not_sukagcc(reg[dest],reg[src]);
      break;
    };
    case 29-base_counter:
    {
      not_sukagcc(reg[dest],mem[src]);
      break;
    };
    case 30-base_counter:
    {
      not_sukagcc(reg[dest],raw[src]);
      break;
    };
    case 31-base_counter:
    {
      not_sukagcc(mem[dest],reg[src]);
      break;
    };
    case 32-base_counter:
    {
      not_sukagcc(mem[dest],mem[src]);
      break;
    };
    case 33-base_counter:
    {
      not_sukagcc(mem[dest],raw[src]);
      break;
    };
    case 34-base_counter:
    {
      not_sukagcc(raw[dest],reg[src]);
      break;
    };
    case 35-base_counter:
    {
      not_sukagcc(raw[dest],mem[src]);
      break;
    };
    case 36-base_counter:
    {
      not_sukagcc(raw[dest],raw[src]);
      break;
    };
    case 37-base_counter:
    {
      inv(reg[dest],reg[src]);
      break;
    };
    case 38-base_counter:
    {
      inv(reg[dest],mem[src]);
      break;
    };
    case 39-base_counter:
    {
      inv(reg[dest],raw[src]);
      break;
    };
    case 40-base_counter:
    {
      inv(mem[dest],reg[src]);
      break;
    };
    case 41-base_counter:
    {
      inv(mem[dest],mem[src]);
      break;
    };
    case 42-base_counter:
    {
      inv(mem[dest],raw[src]);
      break;
    };
    case 43-base_counter:
    {
      inv(raw[dest],reg[src]);
      break;
    };
    case 44-base_counter:
    {
      inv(raw[dest],mem[src]);
      break;
    };
    case 45-base_counter:
    {
      inv(raw[dest],raw[src]);
      break;
    };
    case 46-base_counter:
    {
      add(reg[dest],reg[a],reg[b]);
      break;
    };
    case 47-base_counter:
    {
      add(reg[dest],reg[a],mem[b]);
      break;
    };
    case 48-base_counter:
    {
      add(reg[dest],reg[a],raw[b]);
      break;
    };
    case 49-base_counter:
    {
      add(reg[dest],mem[a],reg[b]);
      break;
    };
    case 50-base_counter:
    {
      add(reg[dest],mem[a],mem[b]);
      break;
    };
    case 51-base_counter:
    {
      add(reg[dest],mem[a],raw[b]);
      break;
    };
    case 52-base_counter:
    {
      add(mem[dest],reg[a],reg[b]);
      break;
    };
    case 53-base_counter:
    {
      add(mem[dest],reg[a],mem[b]);
      break;
    };
    case 54-base_counter:
    {
      add(mem[dest],reg[a],raw[b]);
      break;
    };
    case 55-base_counter:
    {
      add(mem[dest],mem[a],reg[b]);
      break;
    };
    case 56-base_counter:
    {
      add(mem[dest],mem[a],mem[b]);
      break;
    };
    case 57-base_counter:
    {
      add(mem[dest],mem[a],raw[b]);
      break;
    };
    case 58-base_counter:
    {
      sub(reg[dest],reg[a],reg[b]);
      break;
    };
    case 59-base_counter:
    {
      sub(reg[dest],reg[a],mem[b]);
      break;
    };
    case 60-base_counter:
    {
      sub(reg[dest],reg[a],raw[b]);
      break;
    };
    case 61-base_counter:
    {
      sub(reg[dest],mem[a],reg[b]);
      break;
    };
    case 62-base_counter:
    {
      sub(reg[dest],mem[a],mem[b]);
      break;
    };
    case 63-base_counter:
    {
      sub(reg[dest],mem[a],raw[b]);
      break;
    };
    case 64-base_counter:
    {
      sub(mem[dest],reg[a],reg[b]);
      break;
    };
    case 65-base_counter:
    {
      sub(mem[dest],reg[a],mem[b]);
      break;
    };
    case 66-base_counter:
    {
      sub(mem[dest],reg[a],raw[b]);
      break;
    };
    case 67-base_counter:
    {
      sub(mem[dest],mem[a],reg[b]);
      break;
    };
    case 68-base_counter:
    {
      sub(mem[dest],mem[a],mem[b]);
      break;
    };
    case 69-base_counter:
    {
      sub(mem[dest],mem[a],raw[b]);
      break;
    };
    case 70-base_counter:
    {
      mul(reg[dest],reg[a],reg[b]);
      break;
    };
    case 71-base_counter:
    {
      mul(reg[dest],reg[a],mem[b]);
      break;
    };
    case 72-base_counter:
    {
      mul(reg[dest],reg[a],raw[b]);
      break;
    };
    case 73-base_counter:
    {
      mul(reg[dest],mem[a],reg[b]);
      break;
    };
    case 74-base_counter:
    {
      mul(reg[dest],mem[a],mem[b]);
      break;
    };
    case 75-base_counter:
    {
      mul(reg[dest],mem[a],raw[b]);
      break;
    };
    case 76-base_counter:
    {
      mul(mem[dest],reg[a],reg[b]);
      break;
    };
    case 77-base_counter:
    {
      mul(mem[dest],reg[a],mem[b]);
      break;
    };
    case 78-base_counter:
    {
      mul(mem[dest],reg[a],raw[b]);
      break;
    };
    case 79-base_counter:
    {
      mul(mem[dest],mem[a],reg[b]);
      break;
    };
    case 80-base_counter:
    {
      mul(mem[dest],mem[a],mem[b]);
      break;
    };
    case 81-base_counter:
    {
      mul(mem[dest],mem[a],raw[b]);
      break;
    };
    case 82-base_counter:
    {
      div(reg[dest],reg[a],reg[b]);
      break;
    };
    case 83-base_counter:
    {
      div(reg[dest],reg[a],mem[b]);
      break;
    };
    case 84-base_counter:
    {
      div(reg[dest],reg[a],raw[b]);
      break;
    };
    case 85-base_counter:
    {
      div(reg[dest],mem[a],reg[b]);
      break;
    };
    case 86-base_counter:
    {
      div(reg[dest],mem[a],mem[b]);
      break;
    };
    case 87-base_counter:
    {
      div(reg[dest],mem[a],raw[b]);
      break;
    };
    case 88-base_counter:
    {
      div(mem[dest],reg[a],reg[b]);
      break;
    };
    case 89-base_counter:
    {
      div(mem[dest],reg[a],mem[b]);
      break;
    };
    case 90-base_counter:
    {
      div(mem[dest],reg[a],raw[b]);
      break;
    };
    case 91-base_counter:
    {
      div(mem[dest],mem[a],reg[b]);
      break;
    };
    case 92-base_counter:
    {
      div(mem[dest],mem[a],mem[b]);
      break;
    };
    case 93-base_counter:
    {
      div(mem[dest],mem[a],raw[b]);
      break;
    };
    case 94-base_counter:
    {
      mod(reg[dest],reg[a],reg[b]);
      break;
    };
    case 95-base_counter:
    {
      mod(reg[dest],reg[a],mem[b]);
      break;
    };
    case 96-base_counter:
    {
      mod(reg[dest],reg[a],raw[b]);
      break;
    };
    case 97-base_counter:
    {
      mod(reg[dest],mem[a],reg[b]);
      break;
    };
    case 98-base_counter:
    {
      mod(reg[dest],mem[a],mem[b]);
      break;
    };
    case 99-base_counter:
    {
      mod(reg[dest],mem[a],raw[b]);
      break;
    };
    case 100-base_counter:
    {
      mod(mem[dest],reg[a],reg[b]);
      break;
    };
    case 101-base_counter:
    {
      mod(mem[dest],reg[a],mem[b]);
      break;
    };
    case 102-base_counter:
    {
      mod(mem[dest],reg[a],raw[b]);
      break;
    };
    case 103-base_counter:
    {
      mod(mem[dest],mem[a],reg[b]);
      break;
    };
    case 104-base_counter:
    {
      mod(mem[dest],mem[a],mem[b]);
      break;
    };
    case 105-base_counter:
    {
      mod(mem[dest],mem[a],raw[b]);
      break;
    };
    case 106-base_counter:
    {
      eq(reg[dest],reg[a],reg[b]);
      break;
    };
    case 107-base_counter:
    {
      eq(reg[dest],reg[a],mem[b]);
      break;
    };
    case 108-base_counter:
    {
      eq(reg[dest],reg[a],raw[b]);
      break;
    };
    case 109-base_counter:
    {
      eq(reg[dest],mem[a],reg[b]);
      break;
    };
    case 110-base_counter:
    {
      eq(reg[dest],mem[a],mem[b]);
      break;
    };
    case 111-base_counter:
    {
      eq(reg[dest],mem[a],raw[b]);
      break;
    };
    case 112-base_counter:
    {
      eq(mem[dest],reg[a],reg[b]);
      break;
    };
    case 113-base_counter:
    {
      eq(mem[dest],reg[a],mem[b]);
      break;
    };
    case 114-base_counter:
    {
      eq(mem[dest],reg[a],raw[b]);
      break;
    };
    case 115-base_counter:
    {
      eq(mem[dest],mem[a],reg[b]);
      break;
    };
    case 116-base_counter:
    {
      eq(mem[dest],mem[a],mem[b]);
      break;
    };
    case 117-base_counter:
    {
      eq(mem[dest],mem[a],raw[b]);
      break;
    };
    case 118-base_counter:
    {
      neq(reg[dest],reg[a],reg[b]);
      break;
    };
    case 119-base_counter:
    {
      neq(reg[dest],reg[a],mem[b]);
      break;
    };
    case 120-base_counter:
    {
      neq(reg[dest],reg[a],raw[b]);
      break;
    };
    case 121-base_counter:
    {
      neq(reg[dest],mem[a],reg[b]);
      break;
    };
    case 122-base_counter:
    {
      neq(reg[dest],mem[a],mem[b]);
      break;
    };
    case 123-base_counter:
    {
      neq(reg[dest],mem[a],raw[b]);
      break;
    };
    case 124-base_counter:
    {
      neq(mem[dest],reg[a],reg[b]);
      break;
    };
    case 125-base_counter:
    {
      neq(mem[dest],reg[a],mem[b]);
      break;
    };
    case 126-base_counter:
    {
      neq(mem[dest],reg[a],raw[b]);
      break;
    };
    case 127-base_counter:
    {
      neq(mem[dest],mem[a],reg[b]);
      break;
    };
    case 128-base_counter:
    {
      neq(mem[dest],mem[a],mem[b]);
      break;
    };
    case 129-base_counter:
    {
      neq(mem[dest],mem[a],raw[b]);
      break;
    };
    case 130-base_counter:
    {
      less(reg[dest],reg[a],reg[b]);
      break;
    };
    case 131-base_counter:
    {
      less(reg[dest],reg[a],mem[b]);
      break;
    };
    case 132-base_counter:
    {
      less(reg[dest],reg[a],raw[b]);
      break;
    };
    case 133-base_counter:
    {
      less(reg[dest],mem[a],reg[b]);
      break;
    };
    case 134-base_counter:
    {
      less(reg[dest],mem[a],mem[b]);
      break;
    };
    case 135-base_counter:
    {
      less(reg[dest],mem[a],raw[b]);
      break;
    };
    case 136-base_counter:
    {
      less(mem[dest],reg[a],reg[b]);
      break;
    };
    case 137-base_counter:
    {
      less(mem[dest],reg[a],mem[b]);
      break;
    };
    case 138-base_counter:
    {
      less(mem[dest],reg[a],raw[b]);
      break;
    };
    case 139-base_counter:
    {
      less(mem[dest],mem[a],reg[b]);
      break;
    };
    case 140-base_counter:
    {
      less(mem[dest],mem[a],mem[b]);
      break;
    };
    case 141-base_counter:
    {
      less(mem[dest],mem[a],raw[b]);
      break;
    };
    case 142-base_counter:
    {
      more(reg[dest],reg[a],reg[b]);
      break;
    };
    case 143-base_counter:
    {
      more(reg[dest],reg[a],mem[b]);
      break;
    };
    case 144-base_counter:
    {
      more(reg[dest],reg[a],raw[b]);
      break;
    };
    case 145-base_counter:
    {
      more(reg[dest],mem[a],reg[b]);
      break;
    };
    case 146-base_counter:
    {
      more(reg[dest],mem[a],mem[b]);
      break;
    };
    case 147-base_counter:
    {
      more(reg[dest],mem[a],raw[b]);
      break;
    };
    case 148-base_counter:
    {
      more(mem[dest],reg[a],reg[b]);
      break;
    };
    case 149-base_counter:
    {
      more(mem[dest],reg[a],mem[b]);
      break;
    };
    case 150-base_counter:
    {
      more(mem[dest],reg[a],raw[b]);
      break;
    };
    case 151-base_counter:
    {
      more(mem[dest],mem[a],reg[b]);
      break;
    };
    case 152-base_counter:
    {
      more(mem[dest],mem[a],mem[b]);
      break;
    };
    case 153-base_counter:
    {
      more(mem[dest],mem[a],raw[b]);
      break;
    };
    case 154-base_counter:
    {
      or_sukagcc(reg[dest],reg[a],reg[b]);
      break;
    };
    case 155-base_counter:
    {
      or_sukagcc(reg[dest],reg[a],mem[b]);
      break;
    };
    case 156-base_counter:
    {
      or_sukagcc(reg[dest],reg[a],raw[b]);
      break;
    };
    case 157-base_counter:
    {
      or_sukagcc(reg[dest],mem[a],reg[b]);
      break;
    };
    case 158-base_counter:
    {
      or_sukagcc(reg[dest],mem[a],mem[b]);
      break;
    };
    case 159-base_counter:
    {
      or_sukagcc(reg[dest],mem[a],raw[b]);
      break;
    };
    case 160-base_counter:
    {
      or_sukagcc(mem[dest],reg[a],reg[b]);
      break;
    };
    case 161-base_counter:
    {
      or_sukagcc(mem[dest],reg[a],mem[b]);
      break;
    };
    case 162-base_counter:
    {
      or_sukagcc(mem[dest],reg[a],raw[b]);
      break;
    };
    case 163-base_counter:
    {
      or_sukagcc(mem[dest],mem[a],reg[b]);
      break;
    };
    case 164-base_counter:
    {
      or_sukagcc(mem[dest],mem[a],mem[b]);
      break;
    };
    case 165-base_counter:
    {
      or_sukagcc(mem[dest],mem[a],raw[b]);
      break;
    };
    case 166-base_counter:
    {
      and_sukagcc(reg[dest],reg[a],reg[b]);
      break;
    };
    case 167-base_counter:
    {
      and_sukagcc(reg[dest],reg[a],mem[b]);
      break;
    };
    case 168-base_counter:
    {
      and_sukagcc(reg[dest],reg[a],raw[b]);
      break;
    };
    case 169-base_counter:
    {
      and_sukagcc(reg[dest],mem[a],reg[b]);
      break;
    };
    case 170-base_counter:
    {
      and_sukagcc(reg[dest],mem[a],mem[b]);
      break;
    };
    case 171-base_counter:
    {
      and_sukagcc(reg[dest],mem[a],raw[b]);
      break;
    };
    case 172-base_counter:
    {
      and_sukagcc(mem[dest],reg[a],reg[b]);
      break;
    };
    case 173-base_counter:
    {
      and_sukagcc(mem[dest],reg[a],mem[b]);
      break;
    };
    case 174-base_counter:
    {
      and_sukagcc(mem[dest],reg[a],raw[b]);
      break;
    };
    case 175-base_counter:
    {
      and_sukagcc(mem[dest],mem[a],reg[b]);
      break;
    };
    case 176-base_counter:
    {
      and_sukagcc(mem[dest],mem[a],mem[b]);
      break;
    };
    case 177-base_counter:
    {
      and_sukagcc(mem[dest],mem[a],raw[b]);
      break;
    };
    case 178-base_counter:
    {
      shr(reg[dest],reg[a],reg[b]);
      break;
    };
    case 179-base_counter:
    {
      shr(reg[dest],reg[a],mem[b]);
      break;
    };
    case 180-base_counter:
    {
      shr(reg[dest],reg[a],raw[b]);
      break;
    };
    case 181-base_counter:
    {
      shr(reg[dest],mem[a],reg[b]);
      break;
    };
    case 182-base_counter:
    {
      shr(reg[dest],mem[a],mem[b]);
      break;
    };
    case 183-base_counter:
    {
      shr(reg[dest],mem[a],raw[b]);
      break;
    };
    case 184-base_counter:
    {
      shr(mem[dest],reg[a],reg[b]);
      break;
    };
    case 185-base_counter:
    {
      shr(mem[dest],reg[a],mem[b]);
      break;
    };
    case 186-base_counter:
    {
      shr(mem[dest],reg[a],raw[b]);
      break;
    };
    case 187-base_counter:
    {
      shr(mem[dest],mem[a],reg[b]);
      break;
    };
    case 188-base_counter:
    {
      shr(mem[dest],mem[a],mem[b]);
      break;
    };
    case 189-base_counter:
    {
      shr(mem[dest],mem[a],raw[b]);
      break;
    };
    case 190-base_counter:
    {
      shl(reg[dest],reg[a],reg[b]);
      break;
    };
    case 191-base_counter:
    {
      shl(reg[dest],reg[a],mem[b]);
      break;
    };
    case 192-base_counter:
    {
      shl(reg[dest],reg[a],raw[b]);
      break;
    };
    case 193-base_counter:
    {
      shl(reg[dest],mem[a],reg[b]);
      break;
    };
    case 194-base_counter:
    {
      shl(reg[dest],mem[a],mem[b]);
      break;
    };
    case 195-base_counter:
    {
      shl(reg[dest],mem[a],raw[b]);
      break;
    };
    case 196-base_counter:
    {
      shl(mem[dest],reg[a],reg[b]);
      break;
    };
    case 197-base_counter:
    {
      shl(mem[dest],reg[a],mem[b]);
      break;
    };
    case 198-base_counter:
    {
      shl(mem[dest],reg[a],raw[b]);
      break;
    };
    case 199-base_counter:
    {
      shl(mem[dest],mem[a],reg[b]);
      break;
    };
    case 200-base_counter:
    {
      shl(mem[dest],mem[a],mem[b]);
      break;
    };
    case 201-base_counter:
    {
      shl(mem[dest],mem[a],raw[b]);
      break;
    };
    case 202-base_counter:
    {
      jmp(reg[dest]);
      break;
    };
    case 203-base_counter:
    {
      jmp(mem[dest]);
      break;
    };
    case 204-base_counter:
    {
      jmp(raw[dest]);
      break;
    };
    case 205-base_counter:
    {
      call(reg[dest]);
      break;
    };
    case 206-base_counter:
    {
      call(mem[dest]);
      break;
    };
    case 207-base_counter:
    {
      call(raw[dest]);
      break;
    };
    case 208-base_counter:
    {
      push(reg[dest]);
      break;
    };
    case 209-base_counter:
    {
      push(mem[dest]);
      break;
    };
    case 210-base_counter:
    {
      push(raw[dest]);
      break;
    };
    case 211-base_counter:
    {
      pop(reg[dest]);
      break;
    };
    case 212-base_counter:
    {
      pop(mem[dest]);
      break;
    };
    case 213-base_counter:
    {
      inc(reg[dest]);
      break;
    };
    case 214-base_counter:
    {
      inc(mem[dest]);
      break;
    };
    case 215-base_counter:
    {
      dec(reg[dest]);
      break;
    };
    case 216-base_counter:
    {
      dec(mem[dest]);
      break;
    };
    case 217-base_counter:
    {
      ret();
      break;
    };
    case 218-base_counter:
    {
      nop();
      break;
    };
    case 219-base_counter:
    {
      label();
      break;
    };
    case 220-base_counter:
    {
      mov(mem[reg[dest]],reg[src]);
      break;
    };
    case 221-base_counter:
    {
      mov(mem[reg[dest]],mem[src]);
      break;
    };
    case 222-base_counter:
    {
      mov(mem[reg[dest]],raw[src]);
      break;
    };
    case 223-base_counter:
    {
      mov(reg[dest],mem[reg[src]]);
      break;
    };
    case 224-base_counter:
    {
      mov(mem[dest],mem[reg[src]]);
      break;
    };
    }
  }
  void sim_n(int n)
  {
    for (int i=0;i<n;i++)
    {
      auto ip=reg[eip]++;
      exec(arr[ip]);
      reg[cmd_counter]++;
    }
  }
  void sim_till_err()
  {
    for (;!reg[err];)sim_n(1);
    int gg=1;
  }
  void def_app(int iter)
  {
    arr.push_back(t_cmd(20,2,iter,0));
    arr.push_back(t_cmd(20,3,0,0));
    arr.push_back(t_cmd(203,5,0,0));
    arr.push_back(t_cmd(218,0,0,0));
    arr.push_back(t_cmd(212,3,0,0));
    arr.push_back(t_cmd(218,0,0,0));
    arr.push_back(t_cmd(95,5,3,4));
    arr.push_back(t_cmd(212,5,0,0));
    arr.push_back(t_cmd(219,3,5,0));
    arr.push_back(t_cmd(129,4,3,2));
    arr.push_back(t_cmd(15,3,4,0));
    arr.push_back(t_cmd(20,8,1,0));
  }
};

inline void mod(int&dest,int a,int b){dest=a%b;}
inline void less(int&dest,int a,int b){dest=a<b;}
inline void mov(int&dest,int src){dest=src;}
inline void inc(int&inout){inout++;}

// like sequential fill: for(int i=0;i<iter;i++)ptr[i]=(i%4)+1;
int native_func(int*ptr,int iter){
  int eax,ebx,ecx,edx;
  mov(eax,iter);
  mov(ebx,0);
  goto body;
  loop:
  inc(ebx);
  body:
  mod(edx,ebx,4);
  inc(edx);
  mov(ptr[ebx],edx);
  less(ecx,ebx,eax);
  if(ecx)goto loop;
  int gg=1;
  return 0;
}

#include <atomic>

#ifdef _WIN32
  #define FREQ_INIT()QueryPerformanceFrequency(&frequency);
  LARGE_INTEGER frequency;
  real get_time_in_sec(){LARGE_INTEGER tmp;QueryPerformanceCounter(&tmp);return real(tmp.QuadPart)/frequency.QuadPart;}
#else
  #define FREQ_INIT()
  #include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
  real get_time_in_sec(){return (real(clock())*1.0)/CLOCKS_PER_SEC;}
#endif

string jq(const string&s){string q="\"";return q+s+q;}

string to_jk(string s,real t,int n){return jq(s)+":["+std::to_string(t)+","+std::to_string(n)+"]";}

int main(int argc,char**argv)
{
  FREQ_INIT();
  auto getCPUTime=[](){{std::atomic<int> i;}return get_time_in_sec();};
  bool no_ssd=std::is_same<t_ssd_mem,vector<int>>::value;
  int iter=!no_ssd?1024*160:1024*1024*32;
  t_machine m;
  m.mem.resize(iter+16);
  m.reg.resize(1024);
  m.def_app(iter);
  auto bef=getCPUTime();
  m.sim_till_err();
  static real t=1000.0*(getCPUTime()-bef);
  {
    auto native_iter=1024*1024*64;real k=real(native_iter)/iter;
    DECLARE_MEM(native_iter);
    for(int i=0;i<mem.size();i++)mem[i]=0;
    bef=getCPUTime();
    native_func(&mem[0],native_iter);
    static real tn=1000.0*(getCPUTime()-bef);
    static real cpu_speed_ghz=get_cpu_speed()/1e9;
    static real cpu_speed=cpu_speed_ghz*1e6;
    static real cpu_cycles_per_cmd  =t *cpu_speed/real(m.reg[cmd_counter]);
    static real cpu_cycles_per_cmd_n=tn*cpu_speed/real(m.reg[cmd_counter]*k);
    real cmd=real(m.reg[cmd_counter]);
    printf("{\n");
    printf("%s\n",(jq("version(k==cmd_n/cmd)")+":"+jq("1.0.2")).c_str());
    #define F(A,B,C)printf((","+jq(#A)+":"+jq(B)+"\n").c_str(),C);
    F(cpu_speed,"%.2f GHz",cpu_speed_ghz);
    F(cmd,"%i",int(cmd));
    F(cmd/iter,"%.3f",cmd/iter);
    F(t,"%.3f ms",t);
    F(tn,"%.3f ms",tn);
    F(cmd_n/cmd,"%.3f",k);
    F(t*k/tn,"%.3f",t*k/tn);
    F(cmd/t,"%.3f cmd/ms",cmd/t);
    F(cmd*k/tn,"%.3f cmd/ms",cmd*k/tn);
    F(cpu_cycles/cmd,"%.4f",cpu_cycles_per_cmd);
    F(cpu_cycles/cmd_n,"%.4f",cpu_cycles_per_cmd_n);
    #undef F
    string s=(","+jq("raw")+":{\n"+
      "  "+to_jk("VM_perf_"+string(no_ssd?"mem":"ssd"),t,iter)+",\n"+
      "  "+to_jk("O2_perf",tn,native_iter)+"\n"
      "}\n"
    );
    printf("%s",s.c_str());
    printf("}");
    int gg=1;
  }
  return 0;
}
