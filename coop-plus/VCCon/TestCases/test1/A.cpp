#include "header.h"

int A::getA(){
  return A;
}

void A::setA(int a){
  A = a;
}

int* A::getPtrA(){
  return PtrA;
}

void A::setPtrA(int* a){
  PtrA = a;
}

int A::read(){
  return *PtrA;
}

void A::write(int a){
  *PtrA = a;
}
