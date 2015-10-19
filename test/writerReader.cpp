#include "writer.h"
#include "reader.h"

template<class T>
void test(const char* filename, int N=10){
   {
      Writer<T> w(filename);
      for (int i=0;i<N;i++)
         w.Write(i,i);
   }
   Reader<T> r(filename);
   r.Print();
}

int main(){
   
   test<float>("testFloat.gz");
   test<double>("testDouble.gz");

   return 0;
}
