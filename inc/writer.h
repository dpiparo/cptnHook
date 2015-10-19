#ifndef _WRITER__H
#define _WRITER__H

#include <vector>
#include <string>

#include <zlib.h>

#include "utils.h"

/**
Possible optimisations:
- column format: save 2 files, one for the hashes and one for the args
- truncate the arguments. Use float or even less.
 */


const char* DEFAULTWRITEMODE="wb9h";

template<class T>
class Writer {
private:
   std::string fFileName = "";
   gzFile fFilePtr = nullptr;
   std::size_t fCacheSize = 0;
   std::vector<std::pair<T,int>> fCacheValues; // not used yet
public:
   Writer(const std::string& filename, std::size_t cacheSize = 4096):Writer(filename.c_str(),cacheSize){};
   Writer(const char* filename, std::size_t cacheSize = 4096);
   ~Writer();
   Writer(const Writer&) = delete;
   int Write(T x, unsigned int hash );
};

template<class T>
inline int Writer<T>::Write(T x, unsigned int hash ) {
   // LAME
   auto ret = gzwrite(fFilePtr, (void*) (&x), sizeof(T));
   ret += gzwrite(fFilePtr, (void*) (&hash), sizeof(decltype(hash)));
   return ret;
}

template<class T>
Writer<T>::Writer(const char* filename, std::size_t cacheSize):
      fFileName(filename),
      fCacheSize(cacheSize){
   
   fFilePtr = gzopen(filename,DEFAULTWRITEMODE);
   fCacheValues.reserve(cacheSize);
   };
   
template<class T>
Writer<T>::~Writer(){
   gzclose(fFilePtr);      
}



#endif
