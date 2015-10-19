#include <cstring>
#include <iostream>
#include "reader.h"

int main(int argc, char** argv){
   
   if (argc < 2){
      std::cerr << "Usage: reader hookFile.gz" << std::endl;
      return 1;
   }

   auto filename = argv[1];
   
   if (strstr(filename,".dp.")){   
      Reader<double> r(filename);
      r.Print();
   }
   else{
      Reader<float> r(filename);
      r.Print();
   }
   
}