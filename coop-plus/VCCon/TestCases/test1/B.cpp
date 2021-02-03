#include "header.h"

int B::getA(){
  return A+B;
}

void B::setA(int a){
  A = a + B;
}

int* B::getPtrA(){
  return PtrA;
}

void B::setPtrA(int* a){
  PtrA = a;
}

int B::read(){
  return *PtrA + *PtrB;
}

void B::write(int a){
  *PtrA = a + *getPtrB();
}


int B::getB(){
  return B;
}

void B::setB(int a){
  B = a;
}

int* B::getPtrB(){
  return PtrB;
}

void B::setPtrB(int* a){
  PtrB = a;
}
