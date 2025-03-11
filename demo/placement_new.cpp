#include <iostream>
#include <cstring>

class MyClass
{
private:
    int value_;
public:
    MyClass(int value);
    ~MyClass();
    void print() const;
};

MyClass::MyClass(int value):value_(value)
{
    std::cout << "Constructor called with value: " << value_ << std::endl;
}

MyClass::~MyClass()
{
    std::cout << "Destructor called for value: " << value_ << std::endl;
}

void MyClass::print() const{
    std::cout << "Value: " << value_ << std::endl;
}

int main(){
    // 1. 分配一块足够大的内存；
    // 2. 使用placement new在已分配的内存上构造对象；
    // 3. 使用对象；
    // 4. 显式调用析构函数；
    // 5. 释放内存；
    void* memory = malloc(sizeof(MyClass));
    MyClass* obj = new(memory) MyClass(42);
    obj->print();
    obj->~MyClass();
    free(memory);
    return 0;
}