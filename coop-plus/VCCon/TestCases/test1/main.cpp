#include "header.h"

void foo(A* a)
{
  a->read();
}

int main(){
  int i=1, j=101;

  B* b = new B();
  
  b->setB(j);
  b->setPtrB(&j);

  b->setA(i);
  b->setPtrA(&i);

  cout << "b->getA(): " << b->getA() << endl;
  cout << "b->getPtrA(): " << b->getPtrA() << endl;
  cout << "b->getb(): " << b->getB() << endl;
  cout << "b->getPtrb(): " << b->getPtrB() << endl;
  
  cout << "b->read(): " << b->read() << endl;

  return 0;
}
