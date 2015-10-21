#include <iostream>
#include <cmath>

void a(double x){
   for (unsigned long int i=0;i<50000;++i){
      double y = x+i;
      double s = log(y);
      double ss = logf(y);
      if (i%500 == 0)
      std::cout << "In prog log("<< y << ")" <<  s << " " << ss << std::endl;
   }
}

int main(int argc, char** argv){
   double x = argc >1 ? std::stod(argv[1]) : 1;
   a(x);
   std::cout << "Exit from prog\n";

}
