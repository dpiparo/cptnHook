#ifndef _READER__H
#define _READER__H

#include <string>
#include <cstdio>
#include <zlib.h>

template<class T>
class Reader {
private:
   std::string fFileName = "";
   gzFile fFilePtr = nullptr;
public:
   Reader(const char* filename);
   ~Reader();
   Reader(const Reader&) = delete;
   std::size_t Print();
};

template<class T>
std::size_t Reader<T>::Print() {
   auto fpSize = sizeof(T)/sizeof(int);
   auto pairSizeInInts = fpSize+1;
   auto bufferLenghtInBytes = pairSizeInInts * sizeof(int);
   unsigned int buffer[pairSizeInInts];
   auto ended = false;
   int retcode;
   unsigned int hash = 0;
   std::size_t bytesRead = 0;
   unsigned int rowNumber = 0;
   while( (retcode = gzread (fFilePtr, buffer, bufferLenghtInBytes)) ){
      if (-1 == retcode) printf("Error while reading!\n");
      auto x = *((T*) buffer);
      hash = buffer[fpSize];
      bytesRead += retcode;
      printf("Entry %u - %f %u\n",rowNumber, x, hash);
      rowNumber++;
   }
   return bytesRead;

}

template<class T>
Reader<T>::Reader(const char* filename):
      fFileName(filename){
   fFilePtr = gzopen(filename,"rb");
   if (!fFilePtr){
      printf("Error opening %s\n.", filename);
      }

   };

template<class T>
Reader<T>::~Reader(){
   gzclose(fFilePtr);
}



#endif
