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
const unsigned int DEFAULTCACHESIZE = 1048576; // 1MB

template<class T>
class Writer {
private:
   const std::string fFileName = "";
   const gzFile fFilePtr = nullptr;
   const std::size_t fCacheMaxSize = 0;
   std::size_t fCacheSize = 0;
   std::vector<std::pair<T,int>> fCacheValues;
   const unsigned short int fElementSize = 0;
   int FlushCache();
public:
   Writer(const std::string& filename, std::size_t cacheSize = DEFAULTCACHESIZE):Writer(filename.c_str(),cacheSize){};
   Writer(const char* filename, std::size_t cacheSize = DEFAULTCACHESIZE);
   ~Writer();
   Writer(const Writer&) = delete;
   int Write(T x, unsigned int hash );
};

template<>
inline int Writer<float>::FlushCache() {
   gzbuffer(fFilePtr, DEFAULTCACHESIZE);
   auto ret = gzwrite(fFilePtr, (void*) &(fCacheValues[0]), fElementSize*fCacheSize);
   fCacheSize = 0;
   return ret;
}

template<>
inline int Writer<double>::FlushCache() {
   gzbuffer(fFilePtr, DEFAULTCACHESIZE);
   int ret(0);
   for (unsigned int i=0;i<fCacheSize;++i){
      auto& valHash = fCacheValues[i];
      ret += gzwrite(fFilePtr, (void*) (&(valHash.first)), 8);
      ret += gzwrite(fFilePtr, (void*) (&(valHash.second)), 4);
   }
   fCacheSize = 0;
   return ret;
}


template<class T>
inline int Writer<T>::Write(T x, unsigned int hash ) {
   int ret = 0;
   if (fCacheSize != fCacheMaxSize) {
      fCacheValues[fCacheSize++] = std::pair<T, unsigned int>(x, hash);
   } else {
      ret = FlushCache();
   }
   return ret;
}

template<class T>
Writer<T>::Writer(const char* filename, std::size_t cacheSize):
      fFileName(filename),
      fFilePtr(gzopen(filename,DEFAULTWRITEMODE)),      
      fCacheMaxSize(cacheSize),
      fCacheValues(cacheSize),
      fElementSize(sizeof(typename decltype(fCacheValues)::value_type)){
   };
   
template<class T>
Writer<T>::~Writer(){
   FlushCache();
   gzclose(fFilePtr);      
}



#endif
