#include <iostream>

using namespace std;

class A{
  protected:
    int A;
    int* PtrA;

  public:
    virtual int getA();

    virtual void setA(int a);

    virtual int* getPtrA();

    virtual void setPtrA(int* a);

    virtual int read();

    virtual void write(int a);
};

class B: public A{
  protected:
    int B;
    int* PtrB;

  public:
    virtual int getA();

    virtual void setA(int a);

    virtual int* getPtrA();

    virtual void setPtrA(int* a);

    virtual int read();

    virtual void write(int a);

    virtual int getB();

    virtual void setB(int a);

    virtual int* getPtrB();

    virtual void setPtrB(int* a);
};
