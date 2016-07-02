#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include "Unit.h"
#include "StateVariableFilter.h"
#include "Oscillator.h"
#include "Circuit.h"
#include <iostream>
#include <sstream>
#include <memory>

int main() 
{

  syn::Circuit* circ = new syn::Circuit("main");
  syn::Unit* svfUnit = new syn::StateVariableFilter("svfUnit");
  syn::Unit* oscUnit = new syn::BasicOscillator("oscUnit");
  circ->addUnit(svfUnit);
  circ->addUnit(oscUnit);
  circ->connectInternal("oscUnit",0,"svfUnit",0);
  std::stringstream ss;
  {
    cereal::XMLOutputArchive oarchive(ss);
    shared_ptr<syn::Unit> tmpUnit(circ->clone());
    oarchive(tmpUnit);
  }
  std::cout << ss.str() << std::endl;
  std::cout << "----------" << std::endl;

  syn::Unit* readUnit;
  {
    cereal::XMLInputArchive iarchive(ss);
    shared_ptr<syn::Unit> tmpUnit;
    iarchive(tmpUnit);
    readUnit = tmpUnit->clone();
  }

  std::stringstream ss2;
  {
    cereal::XMLOutputArchive oarchive(ss2);
    shared_ptr<syn::Unit> tmpUnit(circ->clone());
    oarchive(tmpUnit);
  }
  std::cout << ss2.str() << std::endl;
  
  return 0;
}
