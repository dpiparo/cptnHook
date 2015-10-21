/**
Library for intercepting arguments of mathematical functions. These 
are saved in compressed binary format.
*/
#include <execinfo.h> // for backtrace
#include <dlfcn.h> // for dladdr
#include <cxxabi.h> // for __cxa_demangle
#include <cstdio>
#include <cstdlib>
#include <cmath> //  W/O this the preload does not work!
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "writer.h"
#include "utils.h"

// ------------------------------------

//  Utils ----------------------------------------------------------------------

static std::string GetCwd(){
   const std::size_t len = 4096;
   char cwdAsPtr[len];
   if (!getcwd(cwdAsPtr, len)){
      logError("GetCwd","Could not get the name of the current directory.");
   }
   std::string cwdAsString(cwdAsPtr);
   return cwdAsString;
}

// Is directory existing
struct stat info;

static bool dirExists(const char* pathname) {
   if( stat( pathname, &info ) != 0 ){
      return false;
   }      
   return info.st_mode & S_IFDIR ;
}

// Get the pid
static std::string GetPID(){
  return std::to_string(getpid());
}

static std::string GetDefaultDir(){
  std::string dname("cptnHookProfile_");  
  return dname+GetPID();
}

static std::string GetReportDir(){
   auto pPath = getenv ("CPTNHOOKLITE_PROFILEDIR");
   auto reportDirName = pPath ? std::string(pPath) : GetDefaultDir();
   if (!dirExists(reportDirName.c_str())){
      mkdir (reportDirName.c_str(), 0777);
   }   
   return GetCwd()+"/"+reportDirName;
}

#ifndef USE_LIBUNWIND
// // Get the backtrace
static std::string Backtrace(int skip = 1){
   static void *callstack[1024];
   const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
   static char buf[1024];
   int nFrames = backtrace(callstack, nMaxFrames);
   char **symbols = backtrace_symbols(callstack, nFrames);

   std::ostringstream trace_buf;
   for (int i = skip; i < nFrames; i++) {
      Dl_info info;
      if (dladdr(callstack[i], &info) && info.dli_sname) {
         char *demangled = NULL;
         int status = -1;
         if (info.dli_sname[0] == '_'){
//             demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
//          snprintf(buf, sizeof( buf), "%s",
//             status == 0 ? demangled : info.dli_sname == 0 ? symbols[i] : info.dli_sname);
//          free(demangled);
            snprintf(buf, sizeof( buf), "%s", info.dli_sname);
         }
      } else {
      snprintf(buf, sizeof(buf), "%s",symbols[i]);
      }
      trace_buf << buf << "`";
   }

   free(symbols);
   if (nFrames == nMaxFrames)
      trace_buf << "[truncated]`";
   return trace_buf.str();
   }
#else

#define UNW_LOCAL_ONLY
#include <libunwind.h>
std::string Backtrace() {
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  // Unwind frames one by one, going up the frame stack.
  unw_word_t offset, pc;
  std::ostringstream trace_buf;
  while (unw_step(&cursor) > 0) {    
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    trace_buf << pc << "`";
//     static char sym[2048];
//     if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
// //       std::printf("%s\n", sym);
//        trace_buf << sym << "`";
//     } else {
//        trace_buf << "err`";
// //       std::printf("err\n");
//     }
  }
  return trace_buf.str();
}
#endif

// Here the hasher
class Hasher{
  unsigned int fHash = 0;
  std::unordered_map<std::string, unsigned int> fHashRegistry;
public:
   Hasher(){
      // Remove any preload library
      setenv("LD_PRELOAD","", true);
   };
   ~Hasher(){ //Here the dump on disk
      logInfo("Hasher","Dumping hash-backtrace map");
      std::string ofileName(GetReportDir()+"/backtraceMap.xml");
      std::ofstream ofile(ofileName);
      ofile << "<cptnHookBackTraces>\n";
      for (auto&& stackHashPair : fHashRegistry){
         ofile << " <bt hash=\"" << stackHashPair.second << "\" val=\"" << stackHashPair.first << "\" />" << std::endl;
      }
      ofile << "</cptnHookBackTraces>\n";
      ofile.close();
   }
   unsigned int Hash(const std::string& s){
      auto stringPos = fHashRegistry.find(s);
      if (stringPos == fHashRegistry.end()) {
         fHashRegistry.insert(std::make_pair(s,fHash));
         return fHash++;
      }
      return stringPos->second;
   };
};
Hasher gHasher; // singleton

//  Interceptors utils ---------------------------------------------

template<class T>
std::string GetPrecision(){return "ERROR";};

template<>
std::string GetPrecision<double>(){return "dp";};

template<>
std::string GetPrecision<float>(){return "sp";};

template<class T>
std::string GetWriterName(const char* funcName){
   return GetReportDir()+"/"+funcName+"."+GetPrecision<T>()+".hook.gz";
}

typedef double (*dpdp) (double);
typedef float (*spsp) (float);

template<class T>
class FuncType{};

template<>
class FuncType<double>{
public:
   using val = dpdp;
};

template<>
class FuncType<float>{
public:
   using val = spsp;
};


template<class T>
auto  getOriginalSym(const char* funcName) -> typename FuncType<T>::val {
   return (typename FuncType<T>::val) dlsym(RTLD_NEXT,funcName);
};

template<class T>
inline void recordInput(T x, Writer<T>& writer) {
   auto s = Backtrace();
   writer.Write(x, gHasher.Hash(s));
}

/*
writerTemplate = 'Writer<%s> %s%sWriter(GetWriterName<%s>("%s"));'
interceptorTemplate = \
'''
auto %s(%s x) -> decltype(x){
   recordInput(x, %s%sWriter);   
   static auto origf = getOriginalSym<%s>("%s");
   return origf(x);
}'''
functions =["exp",
            "log",
            "sin",
            "asin",
            "cos",
            "acos",
            "tan",
            "atan",
            "sinh",
            "asinh",
            "cosh",
            "acosh",
            "tanh",
            "atanh",
            cbrt] 

# Print Writers -------------------------------------------
for function in functions:
    print writerTemplate %("double","dp",function.title(),"double",function)

for function in functions:
    functionsp = function+"f"
    print writerTemplate %("float","sp",function.title()+"","float",functionsp)
    
# Print interceptors --------------------------------------
for function in functions:
   print interceptorTemplate %(function, "double", "dp", function.title(), "double", function)

for function in functions:
   functionsp = function+"f"
   print interceptorTemplate %(functionsp, "float", "sp", function.title(), "float", functionsp)
*/

Writer<double> dpExpWriter(GetWriterName<double>("exp"));
Writer<double> dpLogWriter(GetWriterName<double>("log"));
Writer<double> dpSinWriter(GetWriterName<double>("sin"));
Writer<double> dpAsinWriter(GetWriterName<double>("asin"));
Writer<double> dpCosWriter(GetWriterName<double>("cos"));
Writer<double> dpAcosWriter(GetWriterName<double>("acos"));
Writer<double> dpTanWriter(GetWriterName<double>("tan"));
Writer<double> dpAtanWriter(GetWriterName<double>("atan"));
Writer<double> dpSinhWriter(GetWriterName<double>("sinh"));
Writer<double> dpAsinhWriter(GetWriterName<double>("asinh"));
Writer<double> dpCoshWriter(GetWriterName<double>("cosh"));
Writer<double> dpAcoshWriter(GetWriterName<double>("acosh"));
Writer<double> dpTanhWriter(GetWriterName<double>("tanh"));
Writer<double> dpAtanhWriter(GetWriterName<double>("atanh"));
Writer<double> dpCbrtWriter(GetWriterName<double>("cbrt"));
Writer<float> spExpWriter(GetWriterName<float>("expf"));
Writer<float> spLogWriter(GetWriterName<float>("logf"));
Writer<float> spSinWriter(GetWriterName<float>("sinf"));
Writer<float> spAsinWriter(GetWriterName<float>("asinf"));
Writer<float> spCosWriter(GetWriterName<float>("cosf"));
Writer<float> spAcosWriter(GetWriterName<float>("acosf"));
Writer<float> spTanWriter(GetWriterName<float>("tanf"));
Writer<float> spAtanWriter(GetWriterName<float>("atanf"));
Writer<float> spSinhWriter(GetWriterName<float>("sinhf"));
Writer<float> spAsinhWriter(GetWriterName<float>("asinhf"));
Writer<float> spCoshWriter(GetWriterName<float>("coshf"));
Writer<float> spAcoshWriter(GetWriterName<float>("acoshf"));
Writer<float> spTanhWriter(GetWriterName<float>("tanhf"));
Writer<float> spAtanhWriter(GetWriterName<float>("atanhf"));
Writer<float> spCbrtWriter(GetWriterName<float>("cbrtf"));

auto exp(double x) -> decltype(x){
   recordInput(x, dpExpWriter);   
   static auto origf = getOriginalSym<double>("exp");
   return origf(x);
}

auto log(double x) -> decltype(x){
   recordInput(x, dpLogWriter);   
   static auto origf = getOriginalSym<double>("log");
   return origf(x);
}

auto sin(double x) -> decltype(x){
   recordInput(x, dpSinWriter);   
   static auto origf = getOriginalSym<double>("sin");
   return origf(x);
}

auto asin(double x) -> decltype(x){
   recordInput(x, dpAsinWriter);   
   static auto origf = getOriginalSym<double>("asin");
   return origf(x);
}

auto cos(double x) -> decltype(x){
   recordInput(x, dpCosWriter);   
   static auto origf = getOriginalSym<double>("cos");
   return origf(x);
}

auto acos(double x) -> decltype(x){
   recordInput(x, dpAcosWriter);   
   static auto origf = getOriginalSym<double>("acos");
   return origf(x);
}

auto tan(double x) -> decltype(x){
   recordInput(x, dpTanWriter);   
   static auto origf = getOriginalSym<double>("tan");
   return origf(x);
}

auto atan(double x) -> decltype(x){
   recordInput(x, dpAtanWriter);   
   static auto origf = getOriginalSym<double>("atan");
   return origf(x);
}

auto sinh(double x) -> decltype(x){
   recordInput(x, dpSinhWriter);   
   static auto origf = getOriginalSym<double>("sinh");
   return origf(x);
}

auto asinh(double x) -> decltype(x){
   recordInput(x, dpAsinhWriter);   
   static auto origf = getOriginalSym<double>("asinh");
   return origf(x);
}

auto cosh(double x) -> decltype(x){
   recordInput(x, dpCoshWriter);   
   static auto origf = getOriginalSym<double>("cosh");
   return origf(x);
}

auto acosh(double x) -> decltype(x){
   recordInput(x, dpAcoshWriter);   
   static auto origf = getOriginalSym<double>("acosh");
   return origf(x);
}

auto tanh(double x) -> decltype(x){
   recordInput(x, dpTanhWriter);   
   static auto origf = getOriginalSym<double>("tanh");
   return origf(x);
}

auto atanh(double x) -> decltype(x){
   recordInput(x, dpAtanhWriter);   
   static auto origf = getOriginalSym<double>("atanh");
   return origf(x);
}

auto cbrt(double x) -> decltype(x){
   recordInput(x, dpCbrtWriter);   
   static auto origf = getOriginalSym<double>("cbrt");
   return origf(x);
}

auto expf(float x) -> decltype(x){
   recordInput(x, spExpWriter);   
   static auto origf = getOriginalSym<float>("expf");
   return origf(x);
}

auto logf(float x) -> decltype(x){
   recordInput(x, spLogWriter);   
   static auto origf = getOriginalSym<float>("logf");
   return origf(x);
}

auto sinf(float x) -> decltype(x){
   recordInput(x, spSinWriter);   
   static auto origf = getOriginalSym<float>("sinf");
   return origf(x);
}

auto asinf(float x) -> decltype(x){
   recordInput(x, spAsinWriter);   
   static auto origf = getOriginalSym<float>("asinf");
   return origf(x);
}

auto cosf(float x) -> decltype(x){
   recordInput(x, spCosWriter);   
   static auto origf = getOriginalSym<float>("cosf");
   return origf(x);
}

auto acosf(float x) -> decltype(x){
   recordInput(x, spAcosWriter);   
   static auto origf = getOriginalSym<float>("acosf");
   return origf(x);
}

auto tanf(float x) -> decltype(x){
   recordInput(x, spTanWriter);   
   static auto origf = getOriginalSym<float>("tanf");
   return origf(x);
}

auto atanf(float x) -> decltype(x){
   recordInput(x, spAtanWriter);   
   static auto origf = getOriginalSym<float>("atanf");
   return origf(x);
}

auto sinhf(float x) -> decltype(x){
   recordInput(x, spSinhWriter);   
   static auto origf = getOriginalSym<float>("sinhf");
   return origf(x);
}

auto asinhf(float x) -> decltype(x){
   recordInput(x, spAsinhWriter);   
   static auto origf = getOriginalSym<float>("asinhf");
   return origf(x);
}

auto coshf(float x) -> decltype(x){
   recordInput(x, spCoshWriter);   
   static auto origf = getOriginalSym<float>("coshf");
   return origf(x);
}

auto acoshf(float x) -> decltype(x){
   recordInput(x, spAcoshWriter);   
   static auto origf = getOriginalSym<float>("acoshf");
   return origf(x);
}

auto tanhf(float x) -> decltype(x){
   recordInput(x, spTanhWriter);   
   static auto origf = getOriginalSym<float>("tanhf");
   return origf(x);
}

auto atanhf(float x) -> decltype(x){
   recordInput(x, spAtanhWriter);   
   static auto origf = getOriginalSym<float>("atanhf");
   return origf(x);
}

auto cbrtf(float x) -> decltype(x){
   recordInput(x, spCbrtWriter);   
   static auto origf = getOriginalSym<float>("cbrtf");
   return origf(x);
}