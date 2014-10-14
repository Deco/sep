#include "newParams.h"
#include <iostream>
#include <boost/variant.hpp> 



int main(int argc, char* argv[]) {


  // test data




std::cout << "hello world\n";
  std::vector<std::string> tempAddress;;
  tempAddress.push_back("root");
  ParamAddress pAdr(tempAddress);


  //std::map<std::string, std::shared_ptr<int>> mapper;
  //std::shared_ptr<int> testa(new int(10));// = testb;
  //mapper["Hello world"]= testa;
  //ParamValue v = 35.5f;




  Param::ParamType type = Param::ParamType::OBJ;
  Param param(pAdr, type);

  int whatthefuck = 5;

  ParamManager pManager(whatthefuck);



/*
  ParamAddress adrTest = param.getAddress();
  std::vector<std::string> blah = adrTest.getList();

  for (auto c : blah)
    std::cout << c;
  std::cout << '\n';



  atom<ParamValue> a1(v);
  ParamValue var1 =  a1.get();
*/

  return 0;
}


