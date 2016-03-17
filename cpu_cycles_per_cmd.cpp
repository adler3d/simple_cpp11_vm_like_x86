#define $x$():i_instruction{  void operator()(t_machine&m,const t_cmd&cmd)const{static const t_raw_const raw;auto&mem=m.mem;auto&reg=m.reg;auto&dest=cmd.dest;auto&src=cmd.a;auto&a=cmd.a;auto&b=cmd.b;m.
typedef double real;
#include <vector>
#include <algorithm>
#include <vector>
#include <stdio.h>
using std::vector;
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

	return -1.0;		/* Failed. */
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
  
  struct i_instruction
  {
    virtual void operator()(t_machine&machine,const t_cmd&cmd)const {}
  };

struct t_machine
{
  vector<t_cmd> arr;
  vector<int> mem;
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
  static vector<i_instruction*>&get_all_instructions();
  void exec(const t_cmd&cmd)
  {
    static auto&funcs=get_all_instructions();
    QapAssert(qap_check_id(funcs,cmd.id));
    (*(funcs[cmd.id]))(*this,cmd);
    int gg=1;
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
  void def_app()
  {
    arr.push_back(t_cmd(20,2,65536*256,0));
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

  namespace t_instructions
  {
    static const int beg_counter=0;
template<int NUMBER>struct t_instruction:i_instruction {};
template<>struct t_instruction<1>$x$()jz(reg[dest],reg[src]);
      };
    };
template<>struct t_instruction<2>$x$()jz(reg[dest],mem[src]);
      };
    };
template<>struct t_instruction<3>$x$()jz(reg[dest],raw[src]);
      };
    };
template<>struct t_instruction<4>$x$()jz(mem[dest],reg[src]);
      };
    };
template<>struct t_instruction<5>$x$()jz(mem[dest],mem[src]);
      };
    };
template<>struct t_instruction<6>$x$()jz(mem[dest],raw[src]);
      };
    };
template<>struct t_instruction<7>$x$()jz(raw[dest],reg[src]);
      };
    };
template<>struct t_instruction<8>$x$()jz(raw[dest],mem[src]);
      };
    };
template<>struct t_instruction<9>$x$()jz(raw[dest],raw[src]);
      };
    };
template<>struct t_instruction<10>$x$()jnz(reg[dest],reg[src]);
      };
    };
template<>struct t_instruction<11>$x$()jnz(reg[dest],mem[src]);
      };
    };
template<>struct t_instruction<12>$x$()jnz(reg[dest],raw[src]);
      };
    };
template<>struct t_instruction<13>$x$()jnz(mem[dest],reg[src]);
      };
    };
template<>struct t_instruction<14>$x$()jnz(mem[dest],mem[src]);
      };
    };
template<>struct t_instruction<15>$x$()jnz(mem[dest],raw[src]);
      };
    };
template<>struct t_instruction<16>$x$()jnz(raw[dest],reg[src]);
      };
    };
template<>struct t_instruction<17>$x$()jnz(raw[dest],mem[src]);
      };
    };
template<>struct t_instruction<18>$x$()jnz(raw[dest],raw[src]);
      };
    };
template<>struct t_instruction<19>$x$()mov(reg[dest],reg[src]);
      };
    };
template<>struct t_instruction<20>$x$()mov(reg[dest],mem[src]);
      };
    };
template<>struct t_instruction<21>$x$()mov(reg[dest],raw[src]);
      };
    };
template<>struct t_instruction<22>$x$()mov(mem[dest],reg[src]);
      };
    };
template<>struct t_instruction<23>$x$()mov(mem[dest],mem[src]);
      };
    };
template<>struct t_instruction<24>$x$()mov(mem[dest],raw[src]);
      };
    };
template<>struct t_instruction<25>$x$()mov(raw[dest],reg[src]);
      };
    };
template<>struct t_instruction<26>$x$()mov(raw[dest],mem[src]);
      };
    };
template<>struct t_instruction<27>$x$()mov(raw[dest],raw[src]);
      };
    };
template<>struct t_instruction<28>$x$()not_sukagcc(reg[dest],reg[src]);
      };
    };
template<>struct t_instruction<29>$x$()not_sukagcc(reg[dest],mem[src]);
      };
    };
template<>struct t_instruction<30>$x$()not_sukagcc(reg[dest],raw[src]);
      };
    };
template<>struct t_instruction<31>$x$()not_sukagcc(mem[dest],reg[src]);
      };
    };
template<>struct t_instruction<32>$x$()not_sukagcc(mem[dest],mem[src]);
      };
    };
template<>struct t_instruction<33>$x$()not_sukagcc(mem[dest],raw[src]);
      };
    };
template<>struct t_instruction<34>$x$()not_sukagcc(raw[dest],reg[src]);
      };
    };
template<>struct t_instruction<35>$x$()not_sukagcc(raw[dest],mem[src]);
      };
    };
template<>struct t_instruction<36>$x$()not_sukagcc(raw[dest],raw[src]);
      };
    };
template<>struct t_instruction<37>$x$()inv(reg[dest],reg[src]);
      };
    };
template<>struct t_instruction<38>$x$()inv(reg[dest],mem[src]);
      };
    };
template<>struct t_instruction<39>$x$()inv(reg[dest],raw[src]);
      };
    };
template<>struct t_instruction<40>$x$()inv(mem[dest],reg[src]);
      };
    };
template<>struct t_instruction<41>$x$()inv(mem[dest],mem[src]);
      };
    };
template<>struct t_instruction<42>$x$()inv(mem[dest],raw[src]);
      };
    };
template<>struct t_instruction<43>$x$()inv(raw[dest],reg[src]);
      };
    };
template<>struct t_instruction<44>$x$()inv(raw[dest],mem[src]);
      };
    };
template<>struct t_instruction<45>$x$()inv(raw[dest],raw[src]);
      };
    };
template<>struct t_instruction<46>$x$()add(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<47>$x$()add(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<48>$x$()add(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<49>$x$()add(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<50>$x$()add(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<51>$x$()add(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<52>$x$()add(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<53>$x$()add(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<54>$x$()add(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<55>$x$()add(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<56>$x$()add(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<57>$x$()add(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<58>$x$()sub(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<59>$x$()sub(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<60>$x$()sub(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<61>$x$()sub(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<62>$x$()sub(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<63>$x$()sub(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<64>$x$()sub(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<65>$x$()sub(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<66>$x$()sub(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<67>$x$()sub(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<68>$x$()sub(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<69>$x$()sub(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<70>$x$()mul(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<71>$x$()mul(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<72>$x$()mul(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<73>$x$()mul(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<74>$x$()mul(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<75>$x$()mul(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<76>$x$()mul(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<77>$x$()mul(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<78>$x$()mul(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<79>$x$()mul(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<80>$x$()mul(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<81>$x$()mul(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<82>$x$()div(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<83>$x$()div(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<84>$x$()div(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<85>$x$()div(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<86>$x$()div(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<87>$x$()div(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<88>$x$()div(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<89>$x$()div(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<90>$x$()div(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<91>$x$()div(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<92>$x$()div(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<93>$x$()div(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<94>$x$()mod(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<95>$x$()mod(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<96>$x$()mod(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<97>$x$()mod(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<98>$x$()mod(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<99>$x$()mod(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<100>$x$()mod(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<101>$x$()mod(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<102>$x$()mod(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<103>$x$()mod(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<104>$x$()mod(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<105>$x$()mod(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<106>$x$()eq(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<107>$x$()eq(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<108>$x$()eq(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<109>$x$()eq(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<110>$x$()eq(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<111>$x$()eq(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<112>$x$()eq(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<113>$x$()eq(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<114>$x$()eq(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<115>$x$()eq(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<116>$x$()eq(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<117>$x$()eq(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<118>$x$()neq(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<119>$x$()neq(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<120>$x$()neq(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<121>$x$()neq(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<122>$x$()neq(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<123>$x$()neq(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<124>$x$()neq(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<125>$x$()neq(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<126>$x$()neq(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<127>$x$()neq(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<128>$x$()neq(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<129>$x$()neq(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<130>$x$()less(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<131>$x$()less(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<132>$x$()less(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<133>$x$()less(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<134>$x$()less(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<135>$x$()less(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<136>$x$()less(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<137>$x$()less(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<138>$x$()less(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<139>$x$()less(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<140>$x$()less(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<141>$x$()less(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<142>$x$()more(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<143>$x$()more(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<144>$x$()more(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<145>$x$()more(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<146>$x$()more(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<147>$x$()more(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<148>$x$()more(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<149>$x$()more(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<150>$x$()more(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<151>$x$()more(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<152>$x$()more(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<153>$x$()more(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<154>$x$()or_sukagcc(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<155>$x$()or_sukagcc(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<156>$x$()or_sukagcc(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<157>$x$()or_sukagcc(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<158>$x$()or_sukagcc(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<159>$x$()or_sukagcc(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<160>$x$()or_sukagcc(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<161>$x$()or_sukagcc(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<162>$x$()or_sukagcc(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<163>$x$()or_sukagcc(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<164>$x$()or_sukagcc(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<165>$x$()or_sukagcc(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<166>$x$()and_sukagcc(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<167>$x$()and_sukagcc(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<168>$x$()and_sukagcc(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<169>$x$()and_sukagcc(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<170>$x$()and_sukagcc(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<171>$x$()and_sukagcc(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<172>$x$()and_sukagcc(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<173>$x$()and_sukagcc(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<174>$x$()and_sukagcc(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<175>$x$()and_sukagcc(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<176>$x$()and_sukagcc(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<177>$x$()and_sukagcc(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<178>$x$()shr(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<179>$x$()shr(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<180>$x$()shr(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<181>$x$()shr(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<182>$x$()shr(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<183>$x$()shr(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<184>$x$()shr(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<185>$x$()shr(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<186>$x$()shr(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<187>$x$()shr(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<188>$x$()shr(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<189>$x$()shr(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<190>$x$()shl(reg[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<191>$x$()shl(reg[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<192>$x$()shl(reg[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<193>$x$()shl(reg[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<194>$x$()shl(reg[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<195>$x$()shl(reg[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<196>$x$()shl(mem[dest],reg[a],reg[b]);
      };
    };
template<>struct t_instruction<197>$x$()shl(mem[dest],reg[a],mem[b]);
      };
    };
template<>struct t_instruction<198>$x$()shl(mem[dest],reg[a],raw[b]);
      };
    };
template<>struct t_instruction<199>$x$()shl(mem[dest],mem[a],reg[b]);
      };
    };
template<>struct t_instruction<200>$x$()shl(mem[dest],mem[a],mem[b]);
      };
    };
template<>struct t_instruction<201>$x$()shl(mem[dest],mem[a],raw[b]);
      };
    };
template<>struct t_instruction<202>$x$()jmp(reg[dest]);
      };
    };
template<>struct t_instruction<203>$x$()jmp(mem[dest]);
      };
    };
template<>struct t_instruction<204>$x$()jmp(raw[dest]);
      };
    };
template<>struct t_instruction<205>$x$()call(reg[dest]);
      };
    };
template<>struct t_instruction<206>$x$()call(mem[dest]);
      };
    };
template<>struct t_instruction<207>$x$()call(raw[dest]);
      };
    };
template<>struct t_instruction<208>$x$()push(reg[dest]);
      };
    };
template<>struct t_instruction<209>$x$()push(mem[dest]);
      };
    };
template<>struct t_instruction<210>$x$()push(raw[dest]);
      };
    };
template<>struct t_instruction<211>$x$()pop(reg[dest]);
      };
    };
template<>struct t_instruction<212>$x$()pop(mem[dest]);
      };
    };
template<>struct t_instruction<213>$x$()inc(reg[dest]);
      };
    };
template<>struct t_instruction<214>$x$()inc(mem[dest]);
      };
    };
template<>struct t_instruction<215>$x$()dec(reg[dest]);
      };
    };
template<>struct t_instruction<216>$x$()dec(mem[dest]);
      };
    };
template<>struct t_instruction<217>$x$()ret();
      };
    };
template<>struct t_instruction<218>$x$()nop();
      };
    };
template<>struct t_instruction<219>$x$()label();
      };
    };
template<>struct t_instruction<220>$x$()mov(mem[reg[dest]],reg[src]);
      };
    };
template<>struct t_instruction<221>$x$()mov(mem[reg[dest]],mem[src]);
      };
    };
template<>struct t_instruction<222>$x$()mov(mem[reg[dest]],raw[src]);
      };
    };
template<>struct t_instruction<223>$x$()mov(reg[dest],mem[reg[src]]);
      };
    };
template<>struct t_instruction<224>$x$()mov(mem[dest],mem[reg[src]]);
      };
    };
    static const int end_counter=225;
    static const int size=end_counter-beg_counter-1;
    template<int N>static i_instruction&get()
    {
      static t_instruction<N> tmp;
      return tmp;
    };
  };

  std::vector<i_instruction*>& t_machine::get_all_instructions()
  {
    static vector<i_instruction*> out;
    if (!out.empty())return out;
    #define GG t_instructions
    {
      qap_add_back(out)=&GG::get<226-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<227-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<228-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<229-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<230-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<231-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<232-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<233-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<234-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<235-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<236-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<237-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<238-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<239-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<240-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<241-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<242-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<243-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<244-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<245-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<246-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<247-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<248-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<249-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<250-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<251-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<252-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<253-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<254-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<255-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<256-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<257-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<258-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<259-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<260-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<261-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<262-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<263-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<264-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<265-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<266-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<267-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<268-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<269-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<270-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<271-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<272-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<273-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<274-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<275-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<276-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<277-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<278-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<279-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<280-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<281-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<282-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<283-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<284-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<285-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<286-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<287-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<288-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<289-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<290-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<291-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<292-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<293-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<294-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<295-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<296-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<297-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<298-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<299-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<300-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<301-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<302-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<303-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<304-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<305-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<306-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<307-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<308-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<309-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<310-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<311-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<312-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<313-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<314-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<315-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<316-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<317-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<318-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<319-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<320-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<321-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<322-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<323-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<324-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<325-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<326-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<327-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<328-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<329-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<330-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<331-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<332-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<333-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<334-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<335-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<336-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<337-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<338-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<339-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<340-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<341-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<342-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<343-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<344-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<345-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<346-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<347-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<348-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<349-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<350-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<351-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<352-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<353-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<354-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<355-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<356-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<357-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<358-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<359-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<360-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<361-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<362-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<363-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<364-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<365-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<366-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<367-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<368-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<369-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<370-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<371-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<372-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<373-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<374-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<375-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<376-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<377-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<378-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<379-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<380-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<381-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<382-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<383-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<384-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<385-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<386-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<387-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<388-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<389-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<390-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<391-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<392-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<393-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<394-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<395-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<396-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<397-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<398-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<399-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<400-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<401-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<402-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<403-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<404-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<405-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<406-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<407-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<408-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<409-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<410-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<411-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<412-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<413-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<414-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<415-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<416-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<417-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<418-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<419-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<420-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<421-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<422-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<423-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<424-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<425-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<426-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<427-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<428-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<429-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<430-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<431-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<432-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<433-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<434-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<435-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<436-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<437-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<438-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<439-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<440-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<441-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<442-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<443-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<444-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<445-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<446-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<447-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<448-GG::end_counter>();
    };
    {
      qap_add_back(out)=&GG::get<449-GG::end_counter>();
    };
    return out;
  }
  
int main()
{
  t_machine m;
  m.mem.resize(64*1024*1024);
  m.reg.resize(1024);
  m.def_app();
  auto bef=getCPUTime();
  m.sim_till_err();
  static real t=1000.0*(getCPUTime()-bef);
  static real cpu_speed_ghz=get_cpu_speed()/1e9;
  static real cpu_speed=cpu_speed_ghz*1e6;
  static real cpu_cycles_per_cmd=t*cpu_speed/real(m.reg[cmd_counter]);
  printf("cpu_speed = %.2fGHz\n",cpu_speed_ghz);
  printf("t = %.3fms\n",t);
  printf("cpu_cycles_per_cmd = %.4f\n",cpu_cycles_per_cmd);
  int gg=1;
  return 0;
}
