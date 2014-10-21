#include "params.h"
#include <iostream>
#include <boost/variant.hpp> 

/*
  File used to test server-side of the Parameter system
*/

int main(int argc, char* argv[]) {


  // test data

  std::cout << "hello world" << std::endl;

  /* Used before ParamManager adapted to self-create root param
  std::string name = "root";
  Param rootParam(ParamType::OBJ, name);
  */

  //Testing accessing the root parameter
  ParamManager paramSystem();

  std::shared_ptr<Param> systemRoot = paramSystem.getRootParam();

  std::cout << systemRoot.getName() << std::endl; //Should just print "root"
  

  //Testing adding a new object to the root parameter
  std::string newParamName = "TestObject";

  systemRoot.createParam(newParamName, ParamType::OBJ);

  std::cout << systemRoot.getParam("TestObject").getName() << std::endl; //Should output "TestObject"



  //Test adding terminal to objects
  //Testing boolean
  systemRoot.createParam("TestBool", ParamType::BOOL, false);

  if (systemRoot.getParam("TestBool").getValue() == false) {
      std::cout << "TestBool is false" << std:endl; //Should be outputted
  }
  else std::cout << "TestBool is true(Something is wrong)" << std:endl; //Should NOT be outputted


  //Test adding an integer, locking it and trying to access it. Then unlocking & accessing
  systemRoot.createParam("TestInts", ParamType::INT64, 32); //How to set min/max values??

  systemRoot.getParam("TestInts").setIsLocked(true);

  systemRoot.getParam("TestInts").setValue(16); //Should throw a runtime error

  systemRoot.getParam("TestInts").setIsLocked(false);

  systemRoot.getParam("TestInts").setValue(64);

  //Should output 64
  std::cout << "value of testInts: " << systemRoot.getParam("TestInts").getValue() << std::endl;

  return 0;
}


