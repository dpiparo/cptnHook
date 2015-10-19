#include <iostream>
#include <cmath>

void a(double x){
   for (int i=0;i<30000;++i){
      double y = x+i;
      double s = log(y);
      std::cout << "In prog log("<< y << ")" <<  s << std::endl;
   }
}

int main(int argc, char** argv){
   double x = argc >1 ? std::stod(argv[1]) : 1;
   a(x);
   std::cout << "Exit from prog\n";

}
