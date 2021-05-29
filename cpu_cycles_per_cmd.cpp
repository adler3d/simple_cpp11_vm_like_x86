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
#if(defined(_WIN32)||defined(_WIN64))

  #include <intrin.h>
  uint64_t rdtsc(){
      return __rdtsc();
  }

  #define intrin_nop()__nop();

//  Linux/GCC
#else
  
  #define intrin_nop()asm("nop");

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
    intrin_nop();
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

typedef long long int int64;
static_assert(sizeof(int64)==8,"sizeof(int64)==8");
//typedef int64 t_val;
typedef int t_val;
struct t_cmd
{
  t_val id;
  t_val dest;
  t_val a;
  t_val b;
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
  void set(t_val id,t_val dest=0,t_val a=0,t_val b=0){this->id=id;this->dest=dest;this->a=a;this->b=b;}
  t_cmd(t_val id,t_val dest=0,t_val a=0,t_val b=0){this->id=id;this->dest=dest;this->a=a;this->b=b;}
};
enum t_register
{
  eip, cmd_counter, eax, ebx, ecx, edx, ebp, esp, err,
};

struct t_machine;

struct t_raw_const{t_val operator[](t_val value)const{return value;}};

struct t_ssd_mem{
  decltype(fopen(0,0)) f;
  t_val f_size;
  t_ssd_mem(){f=0;f_size=0;}
 ~t_ssd_mem(){fflush(f);fclose(f);}
  t_val size(){return f_size;}
  void resize(t_val size){
    if(!f)f=fopen("vm_mem.bin","wb+");
    if(size<=0)return;
    f_size=size;
    fseek(f,size*sizeof(t_val)-1,SEEK_SET);
    fputc('\0',f);
    fflush(f);
  }
  struct t_int{
    decltype(fopen(0,0)) f;
    t_val addr;
    t_val old;
    t_val value;
    operator t_val&(){return value;}
    operator const t_val&()const{return value;}
    void operator=(t_val v){value=v;};
   ~t_int(){
      if(old==value)return;
      fseek(f,addr*sizeof(t_val),SEEK_SET);
      fwrite(&value,1,sizeof(value),f);
    }
  };
  const t_val operator[](t_val addr)const{
    t_val out;
    fseek(f,addr*sizeof(t_val),SEEK_SET);
    if(fread(&out,1,sizeof(out),f)!=sizeof(out))out=0;
    return out;
  }
  t_int operator[](t_val addr){
    t_int tmp={f,addr};
    t_val&out=tmp.value;
    fseek(f,addr*sizeof(t_val),SEEK_SET);
    if(fread(&out,1,sizeof(out),f)!=sizeof(out))out=0;
    tmp.old=out;
    return tmp;
  }
};

//#define USE_SSD_MEM

#ifdef USE_SSD_MEM
  #define DECLARE_MEM(iter)vector<t_val> mem;mem.resize(iter+16);
  #define t_ssd_mem t_ssd_mem
#else
  #define DECLARE_MEM(iter)vector<t_val> mem;mem.resize(iter+16);//auto&mem=m.mem;
  #define t_ssd_mem vector<t_val>
#endif

struct t_machine
{
  vector<t_cmd> arr;
  t_ssd_mem mem;
  vector<t_val> reg;
  void DoReset(){}
  void operator=(t_machine&&ref){
    oper_set(std::move(ref));
  }
  t_machine(t_machine&&ref){
    DoReset();
    oper_set(std::move(ref));
  }
  void oper_set(t_machine&&ref){
    this->arr=std::move(ref.arr);
    this->mem=std::move(ref.mem);
    this->reg=std::move(ref.reg);
  }
  t_machine(){DoReset();}

  void jz(const t_val&dest,const t_val&src)
  {
    if (!src)jmp(dest);
  }
  void jnz(const t_val&dest,const t_val&src)
  {
    if (src)jmp(dest);
  }
  void mov(const t_val&,const t_val&)
  {
    QapNoWay();
  }
  void not_sukagcc(const t_val&,const t_val&)
  {
    QapNoWay();
  }
  void inv(const t_val&,const t_val&)
  {
    QapNoWay();
  }
  void mov(t_val&dest,const t_val&src)
  {
    dest=src;
  }
  void not_sukagcc(t_val&dest,const t_val&src)
  {
    dest=!bool(src);
  }
  void inv(t_val&dest,const t_val&src)
  {
    dest=-src;
  }
  void add(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a+b;
  }
  void sub(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a-b;
  }
  void mul(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a*b;
  }
  void div(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a/b;
  }
  void mod(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a%b;
  }
  void eq(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a==b;
  }
  void neq(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a!=b;
  }
  void less(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a<b;
  }
  void more(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a>b;
  }
  void or_sukagcc(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a||b;
  }
  void and_sukagcc(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a&&b;
  }
  void shr(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a>>b;
  }
  void shl(t_val&dest,const t_val&a,const t_val&b)
  {
    dest=a<<b;
  }
  void jmp(const t_val&dest)
  {
    reg[eip]=dest;
  }
  void inc(t_val&inout)
  {
    inout++;
  }
  void dec(t_val&inout)
  {
    inout--;
  }
  void push(const t_val&inp)
  {
    reg[esp]--;
    mem[reg[esp]]=inp;
  }
  void pop(t_val&dest)
  {
    dest=mem[reg[esp]];
    reg[esp]++;
  }
  void call(const t_val&addr)
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
    const int base_counter=0+1;

    #define C(ID,CODE)case ID-base_counter:{CODE;break;}
    #define not$ not_sukagcc
    #define or$ or_sukagcc
    #define and$ and_sukagcc
    #define D dest
    #define S src
    #define R reg
    #define M mem
    #define W raw
    switch (cmd.id){
      C(1,jz(R[D],R[S]))
      C(2,jz(R[D],M[S]))
      C(3,jz(R[D],W[S]))
      C(4,jz(M[D],R[S]))
      C(5,jz(M[D],M[S]))
      C(6,jz(M[D],W[S]))
      C(7,jz(W[D],R[S]))
      C(8,jz(W[D],M[S]))
      C(9,jz(W[D],W[S]))
      C(10,jnz(R[D],R[S]))
      C(11,jnz(R[D],M[S]))
      C(12,jnz(R[D],W[S]))
      C(13,jnz(M[D],R[S]))
      C(14,jnz(M[D],M[S]))
      C(15,jnz(M[D],W[S]))
      C(16,jnz(W[D],R[S]))
      C(17,jnz(W[D],M[S]))
      C(18,jnz(W[D],W[S]))
      C(19,mov(R[D],R[S]))
      C(20,mov(R[D],M[S]))
      C(21,mov(R[D],W[S]))
      C(22,mov(M[D],R[S]))
      C(23,mov(M[D],M[S]))
      C(24,mov(M[D],W[S]))
      C(25,mov(W[D],R[S]))
      C(26,mov(W[D],M[S]))
      C(27,mov(W[D],W[S]))
      C(28,not$(R[D],R[S]))
      C(29,not$(R[D],M[S]))
      C(30,not$(R[D],W[S]))
      C(31,not$(M[D],R[S]))
      C(32,not$(M[D],M[S]))
      C(33,not$(M[D],W[S]))
      C(34,not$(W[D],R[S]))
      C(35,not$(W[D],M[S]))
      C(36,not$(W[D],W[S]))
      C(37,inv(R[D],R[S]))
      C(38,inv(R[D],M[S]))
      C(39,inv(R[D],W[S]))
      C(40,inv(M[D],R[S]))
      C(41,inv(M[D],M[S]))
      C(42,inv(M[D],W[S]))
      C(43,inv(W[D],R[S]))
      C(44,inv(W[D],M[S]))
      C(45,inv(W[D],W[S]))
      C(46,add(R[D],R[a],R[b]))
      C(47,add(R[D],R[a],M[b]))
      C(48,add(R[D],R[a],W[b]))
      C(49,add(R[D],M[a],R[b]))
      C(50,add(R[D],M[a],M[b]))
      C(51,add(R[D],M[a],W[b]))
      C(52,add(M[D],R[a],R[b]))
      C(53,add(M[D],R[a],M[b]))
      C(54,add(M[D],R[a],W[b]))
      C(55,add(M[D],M[a],R[b]))
      C(56,add(M[D],M[a],M[b]))
      C(57,add(M[D],M[a],W[b]))
      C(58,sub(R[D],R[a],R[b]))
      C(59,sub(R[D],R[a],M[b]))
      C(60,sub(R[D],R[a],W[b]))
      C(61,sub(R[D],M[a],R[b]))
      C(62,sub(R[D],M[a],M[b]))
      C(63,sub(R[D],M[a],W[b]))
      C(64,sub(M[D],R[a],R[b]))
      C(65,sub(M[D],R[a],M[b]))
      C(66,sub(M[D],R[a],W[b]))
      C(67,sub(M[D],M[a],R[b]))
      C(68,sub(M[D],M[a],M[b]))
      C(69,sub(M[D],M[a],W[b]))
      C(70,mul(R[D],R[a],R[b]))
      C(71,mul(R[D],R[a],M[b]))
      C(72,mul(R[D],R[a],W[b]))
      C(73,mul(R[D],M[a],R[b]))
      C(74,mul(R[D],M[a],M[b]))
      C(75,mul(R[D],M[a],W[b]))
      C(76,mul(M[D],R[a],R[b]))
      C(77,mul(M[D],R[a],M[b]))
      C(78,mul(M[D],R[a],W[b]))
      C(79,mul(M[D],M[a],R[b]))
      C(80,mul(M[D],M[a],M[b]))
      C(81,mul(M[D],M[a],W[b]))
      C(82,div(R[D],R[a],R[b]))
      C(83,div(R[D],R[a],M[b]))
      C(84,div(R[D],R[a],W[b]))
      C(85,div(R[D],M[a],R[b]))
      C(86,div(R[D],M[a],M[b]))
      C(87,div(R[D],M[a],W[b]))
      C(88,div(M[D],R[a],R[b]))
      C(89,div(M[D],R[a],M[b]))
      C(90,div(M[D],R[a],W[b]))
      C(91,div(M[D],M[a],R[b]))
      C(92,div(M[D],M[a],M[b]))
      C(93,div(M[D],M[a],W[b]))
      C(94,mod(R[D],R[a],R[b]))
      C(95,mod(R[D],R[a],M[b]))
      C(96,mod(R[D],R[a],W[b]))
      C(97,mod(R[D],M[a],R[b]))
      C(98,mod(R[D],M[a],M[b]))
      C(99,mod(R[D],M[a],W[b]))
      C(100,mod(M[D],R[a],R[b]))
      C(101,mod(M[D],R[a],M[b]))
      C(102,mod(M[D],R[a],W[b]))
      C(103,mod(M[D],M[a],R[b]))
      C(104,mod(M[D],M[a],M[b]))
      C(105,mod(M[D],M[a],W[b]))
      C(106,eq(R[D],R[a],R[b]))
      C(107,eq(R[D],R[a],M[b]))
      C(108,eq(R[D],R[a],W[b]))
      C(109,eq(R[D],M[a],R[b]))
      C(110,eq(R[D],M[a],M[b]))
      C(111,eq(R[D],M[a],W[b]))
      C(112,eq(M[D],R[a],R[b]))
      C(113,eq(M[D],R[a],M[b]))
      C(114,eq(M[D],R[a],W[b]))
      C(115,eq(M[D],M[a],R[b]))
      C(116,eq(M[D],M[a],M[b]))
      C(117,eq(M[D],M[a],W[b]))
      C(118,neq(R[D],R[a],R[b]))
      C(119,neq(R[D],R[a],M[b]))
      C(120,neq(R[D],R[a],W[b]))
      C(121,neq(R[D],M[a],R[b]))
      C(122,neq(R[D],M[a],M[b]))
      C(123,neq(R[D],M[a],W[b]))
      C(124,neq(M[D],R[a],R[b]))
      C(125,neq(M[D],R[a],M[b]))
      C(126,neq(M[D],R[a],W[b]))
      C(127,neq(M[D],M[a],R[b]))
      C(128,neq(M[D],M[a],M[b]))
      C(129,neq(M[D],M[a],W[b]))
      C(130,less(R[D],R[a],R[b]))
      C(131,less(R[D],R[a],M[b]))
      C(132,less(R[D],R[a],W[b]))
      C(133,less(R[D],M[a],R[b]))
      C(134,less(R[D],M[a],M[b]))
      C(135,less(R[D],M[a],W[b]))
      C(136,less(M[D],R[a],R[b]))
      C(137,less(M[D],R[a],M[b]))
      C(138,less(M[D],R[a],W[b]))
      C(139,less(M[D],M[a],R[b]))
      C(140,less(M[D],M[a],M[b]))
      C(141,less(M[D],M[a],W[b]))
      C(142,more(R[D],R[a],R[b]))
      C(143,more(R[D],R[a],M[b]))
      C(144,more(R[D],R[a],W[b]))
      C(145,more(R[D],M[a],R[b]))
      C(146,more(R[D],M[a],M[b]))
      C(147,more(R[D],M[a],W[b]))
      C(148,more(M[D],R[a],R[b]))
      C(149,more(M[D],R[a],M[b]))
      C(150,more(M[D],R[a],W[b]))
      C(151,more(M[D],M[a],R[b]))
      C(152,more(M[D],M[a],M[b]))
      C(153,more(M[D],M[a],W[b]))
      C(154,or$(R[D],R[a],R[b]))
      C(155,or$(R[D],R[a],M[b]))
      C(156,or$(R[D],R[a],W[b]))
      C(157,or$(R[D],M[a],R[b]))
      C(158,or$(R[D],M[a],M[b]))
      C(159,or$(R[D],M[a],W[b]))
      C(160,or$(M[D],R[a],R[b]))
      C(161,or$(M[D],R[a],M[b]))
      C(162,or$(M[D],R[a],W[b]))
      C(163,or$(M[D],M[a],R[b]))
      C(164,or$(M[D],M[a],M[b]))
      C(165,or$(M[D],M[a],W[b]))
      C(166,and$(R[D],R[a],R[b]))
      C(167,and$(R[D],R[a],M[b]))
      C(168,and$(R[D],R[a],W[b]))
      C(169,and$(R[D],M[a],R[b]))
      C(170,and$(R[D],M[a],M[b]))
      C(171,and$(R[D],M[a],W[b]))
      C(172,and$(M[D],R[a],R[b]))
      C(173,and$(M[D],R[a],M[b]))
      C(174,and$(M[D],R[a],W[b]))
      C(175,and$(M[D],M[a],R[b]))
      C(176,and$(M[D],M[a],M[b]))
      C(177,and$(M[D],M[a],W[b]))
      C(178,shr(R[D],R[a],R[b]))
      C(179,shr(R[D],R[a],M[b]))
      C(180,shr(R[D],R[a],W[b]))
      C(181,shr(R[D],M[a],R[b]))
      C(182,shr(R[D],M[a],M[b]))
      C(183,shr(R[D],M[a],W[b]))
      C(184,shr(M[D],R[a],R[b]))
      C(185,shr(M[D],R[a],M[b]))
      C(186,shr(M[D],R[a],W[b]))
      C(187,shr(M[D],M[a],R[b]))
      C(188,shr(M[D],M[a],M[b]))
      C(189,shr(M[D],M[a],W[b]))
      C(190,shl(R[D],R[a],R[b]))
      C(191,shl(R[D],R[a],M[b]))
      C(192,shl(R[D],R[a],W[b]))
      C(193,shl(R[D],M[a],R[b]))
      C(194,shl(R[D],M[a],M[b]))
      C(195,shl(R[D],M[a],W[b]))
      C(196,shl(M[D],R[a],R[b]))
      C(197,shl(M[D],R[a],M[b]))
      C(198,shl(M[D],R[a],W[b]))
      C(199,shl(M[D],M[a],R[b]))
      C(200,shl(M[D],M[a],M[b]))
      C(201,shl(M[D],M[a],W[b]))
      C(202,jmp(R[D]))
      C(203,jmp(M[D]))
      C(204,jmp(W[D]))
      C(205,call(R[D]))
      C(206,call(M[D]))
      C(207,call(W[D]))
      C(208,push(R[D]))
      C(209,push(M[D]))
      C(210,push(W[D]))
      C(211,pop(R[D]))
      C(212,pop(M[D]))
      C(213,inc(R[D]))
      C(214,inc(M[D]))
      C(215,dec(R[D]))
      C(216,dec(M[D]))
      C(217,ret())
      C(218,nop())
      C(219,label())
      C(220,mov(M[R[D]],R[S]))
      C(221,mov(M[R[D]],M[S]))
      C(222,mov(M[R[D]],W[S]))
      C(223,mov(R[D],M[R[S]]))
      C(224,mov(M[D],M[R[S]]))
    }
    #undef C
    #undef D
    #undef W
    #undef M
    #undef R
    #undef S
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
  void def_app(t_val iter)
  {
    arr.push_back(t_cmd(20,2,iter,0));
    arr.push_back(t_cmd(20,3,0,0));
    arr.push_back(t_cmd(203,5,0,0));
    arr.push_back(t_cmd(218,0,0,0));
    arr.push_back(t_cmd(212,3,0,0));
    arr.push_back(t_cmd(218,0,0,0));
    arr.push_back(t_cmd(47,5,3,4));
    arr.push_back(t_cmd(212,5,0,0));
    arr.push_back(t_cmd(219,3,5,0));
    arr.push_back(t_cmd(129,4,3,2));
    arr.push_back(t_cmd(15,3,4,0));
    arr.push_back(t_cmd(20,8,1,0));
  }
};

inline void mod(t_val&dest,t_val a,t_val b){dest=a%b;}
inline void less(t_val&dest,t_val a,t_val b){dest=a<b;}
inline void mov(t_val&dest,t_val src){dest=src;}
inline void inc(t_val&inout){inout++;}

// like sequential fill: for(int i=0;i<iter;i++)ptr[i]=(i%4)+1;
int native_func(t_val*ptr,t_val iter){
  t_val eax,ebx,ecx,edx;
  mov(eax,iter);
  mov(ebx,0);
  goto body;
  loop:
  inc(ebx);
  body:
  //mod(edx,ebx,4);
  edx=ebx&2;
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
  bool no_ssd=std::is_same<t_ssd_mem,vector<t_val>>::value;
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
    auto ver_ext="t_val("+std::to_string(sizeof(t_val)*8)+"bit)";
    ver_ext+=" size_t("+std::to_string(sizeof(size_t)*8)+"bit)";
    printf("%s\n",(jq("version(k==cmd_n/cmd)")+":"+jq("1.0.3    "+ver_ext)).c_str());
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
