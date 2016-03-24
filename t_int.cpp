// t_mem & t_int is two classes for working with bit-addressable
#include <random>
#include <vector>
#include <bitset>
#include <stdio.h>
#define QapAssert(UNUSED)
#define QapNoWay()
#define QapDebugMsg(MSG)
using std::vector; using std::string; using std::bitset;
template<typename TYPE>TYPE Sign(TYPE value){return TYPE(value>0?1:(value<0?-1:0));}
inline string IToS(const int&val){char c[16];sprintf(c,"%i",val);return string(c);}

struct t_int;
struct t_mem{
  vector<unsigned char> arr;
  t_int operator[](int bitptr);
  unsigned size()const{
    return arr.size()*sizeof(arr[0])*8;
  }
  void resize(size_t size){arr.resize(Sign(size%8)+size/8);}
};

struct t_int{
  struct t_mem*ptr;
  int bitptr;
  bool asBool()const{return !ptr?bitptr:unsafe_get();}
  int asInt()const{return !ptr?bitptr:unsafe_get();}
  void operator=(t_int src){
    if(!ptr)QapNoWay();unsafe_set(src.asInt());
  }
  void operator=(bool src){QapNoWay();}
  void operator=(int src){if(!ptr)QapNoWay();unsafe_set(src);}
  void operator=(unsigned src){if(!ptr)QapNoWay();unsafe_set(src);}
  void operator++(int){if(!ptr)QapNoWay();unsafe_set(unsafe_get()+1);}
  void operator--(int){if(!ptr)QapNoWay();unsafe_set(unsafe_get()-1);}
  operator const int(){return !ptr?bitptr:unsafe_get();}
  t_int(int value):ptr(nullptr),bitptr(value){}
  t_int(unsigned value):ptr(nullptr),bitptr(value){}
  t_int():ptr(nullptr),bitptr(0){}
  t_int(struct t_mem*ptr,int bitptr):ptr(ptr),bitptr(bitptr){}
  const int slow_get()const{
    auto m=bitptr%8;
    auto id=bitptr/8;
    auto a=(unsigned int&)ptr->arr[id];
    auto b=ptr->arr[id+4];
    std::bitset<32+8> s;
    (int&)s=a;
    ((unsigned char*)&s)[4]=b;
    unsigned out=0;
    for(int i=0;i<32;i++){out>>=1;out=out|unsigned(s[i+m]<<31);}
    return out;
  }
  void slow_set(int v)const{
    auto m=bitptr%8;
    auto id=bitptr/8;
    auto&a=(unsigned int&)ptr->arr[id];
    auto&b=ptr->arr[id+4];
    std::bitset<32+8> s;
    (int&)s=a;
    ((unsigned char*)&s)[4]=b;
    unsigned inp=v;
    for(int i=0;i<32;i++){s[i+m]=v%2;v>>=1;}
    a=(int&)s;
    b=((unsigned char*)&s)[4];
  }
  const int unsafe_get()const{
    typedef unsigned char u8;
    auto m=bitptr%8;
    auto id=bitptr/8;
    auto a=(unsigned int&)ptr->arr[id];
    if(!m)return a;
    auto b=ptr->arr[id+4];
    auto fp=a>>m;
    u8 boff=u8(0xff)>>(8-m);
    auto sp=b&boff;
    return fp|(sp<<32-m);
  }
  void unsafe_set(int v){
    typedef unsigned char u8;
    auto m=bitptr%8;
    auto id=bitptr/8;
    auto&a=(unsigned int&)ptr->arr[id];
    if(!m){a=v;return;}
    auto&b=ptr->arr[id+4];
    auto off=1<<m;off--;
    a=(a&off)|(v<<m);
    u8 boff=~((unsigned(1)<<m)-1);
    auto fp=(b&boff);
    auto sp=u8(unsigned(v)>>(32-m));
    b=fp|sp;
  }
};
t_int t_mem::operator[](int bitptr){return t_int(this,bitptr);}

template<unsigned N>
string tostr(const std::bitset<N>&bs){string out;out.resize(N);for(int i=0;i<N;i++)out[i]=bs[i]?'1':'0';return out;}

static unsigned rnd(){
  static std::ranlux24 generator;
  static std::uniform_int_distribution<unsigned> distribution(0,0xffff);
  return distribution(generator);
}
void init_mem(t_mem&mem){mem.resize(8*64);for(int i=0;i<64/8;i++)mem.arr[i]=rnd();}
string mem2str(const t_mem&mem){auto&bs=(std::bitset<64>&)mem.arr[0];return tostr(bs);}

int main()
{
  t_mem mem;init_mem(mem);
  string out;
  for(int i=0;i<256*1024;i++){
    t_mem mem;init_mem(mem);
    auto addr=rnd()%32;
    unsigned fv=mem[addr];
    unsigned sv=mem[addr].slow_get();
    out="";
    out+="addr:"+IToS(addr)+"\n";
    out+="mem:"+mem2str(mem)+"\n";
    out+="fv:"+tostr((std::bitset<32>&)fv)+"\n";
    out+="sv:"+tostr((std::bitset<32>&)sv)+"\n";
    auto st=mem2str(mem).substr(addr,32);
    out+="st:"+st+"\n";
    bool ok=st==tostr((std::bitset<32>&)fv);
    if(fv!=sv||!ok){printf("%s",out.c_str());return 1;}
    int gg=1;
  }
  printf("getter are tested!\n");
  for(int i=0;i<256*1024;i++){
    t_mem mem;init_mem(mem);
    auto dbg=mem;
    auto mbs=mem;
    auto addr=i%32;
    auto v=(rnd()<<16)|rnd();
    mem[addr]=v;
    mbs[addr].slow_set(v);
    out="";
    out+="addr:"+IToS(addr)+"\n";
    out+="v:"+tostr((std::bitset<32>&)v)+"\n";
    out+="dbg:"+mem2str(dbg)+"\n";
    out+="mem:"+mem2str(mem)+"\n";
    out+="mbs:"+mem2str(mbs)+"\n";
    auto tru=mem2str(dbg).substr(0,addr)+tostr((std::bitset<32>&)v)+mem2str(dbg).substr(addr+32);
    out+="tru:"+tru+"\n";
    out+="hlp:"+mem2str(dbg).substr(0,addr)+string(32,'V')+mem2str(dbg).substr(addr+32)+"\n";
    bool ok=tru==mem2str(mem);
    if(mbs.arr!=mem.arr||!ok){printf("%s",out.c_str());return 1;}
    int gg=1;
  }
  printf("setter are tested!\n");
  return 0;
}
