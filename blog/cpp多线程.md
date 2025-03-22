---
title: cpp多线程
---

# 基础概念

进程：是操作系统进行资源分配和调度的一个独立单位，是应用程序运行的实例，每个进程都有自己独立的内存空间；

线程：是进程中的执行单元，多个线程共享同一个进程的空间和资源，但是每个线程有自己的栈、程序计数器等，线程是CPU调度的基本单元；

并发：是指两个或多个事件在同一个时间间隔发生，并发是针对单核CPU提出的，在同一个CPU上处理多个事件；

并行：是指两个或多个事件在同一时刻发生，并行是针对多核CPU提出的，在不同的CPU上处理多个事件；

多线程：多线程是实现并发/并行的手段，即多个线程同时执行，通常多线程是把执行一件事情的完整步骤拆分成多个子步骤，然后使得这多个步骤同时执行；

# 线程的创建

1. 无参构造
2. 有参构造
3. 移动构造
4. ps.thread对象不能使用拷贝构造

### 无参构造

通过无参构造创建出来的线程对象没有关联任何线程函数，也就是它没有启动任何线程。

```cpp
thread t1;
```

上述 `t1`实际没有任何操作系统中实际的线程，由于thread提供了移动赋值函数，当后续需要让该线程对象与线程函数关联的时候，可以以带参的方式创建一个匿名对象，然后调用移动赋值将该匿名对象关联线程的状态转移给 `t1`;

```cpp
thread t1;
t1 = thread(func, 10);
t1.join();
```



### 带可变参数包的构造

C++中支持函数模板的可变参数，这里的thread的构造函数是一个模板函数：

```cpp
template <class Fn, class ... Args>
explicit thread(Fn&& fn, Args&&...args);
```

Fn表示可调用对象：

- 函数指针；
- 仿函数（函数对象）
- lambda表达式

```cpp
#include <iostream>
#include <thread>
#include <unistd.h>
using namespace std;

void func(int start, int end){
    for(int i = start; i <= end; i++){
        cout << i << " ";
    }
    cout << endl;
}

struct My_class{
    void operator()(int start, int end){
        for(int i = start; i <= end; i++){
            cout << i << " ";
        }
        cout << endl;
    }
};

int main(){
    // 函数指针
    thread t1(func, 1, 10);
    sleep(1);

    // 仿函数
    thread t2(My_class(), 11, 20);
    sleep(1);
  
    // lambda表达式
    thread t3([](const string &str) -> void{
        cout << str << endl;
    }, "Hello World");
    sleep(1);


    t1.join();
    t2.join();
    t3.join();
  
    return 0;
}
```



### 移动构造

thread提供了移动构造函数，用一个右值线程对象来构造一个线程对象：

```cpp
thread t3 = thread(func, 4, 20);
thread t4(std::move(thread(func, 10, 20)));
```

thread类禁止拷贝，不允许使用拷贝构造和拷贝赋值，但是可以移动构造和移动赋值，可以将一个线程对象关联线程所关联的状态转移给其他线程对象，并且转移期间不影响线程的执行；


# thread类的成员函数

- join：等待一个线程完成，如果该线程还未执行完毕，则当前线程将为阻塞，直到该线程执行完成，主线程才会继续执行；
- joinable：判断线程是否可以执行join()函数，如果可以返回true，否则返回false；
- detach：将当前线程与创建的线程分离，使他们分别运行，当分离的线程执行完毕后，系统会自动回收其资源，如果一个线程被分离了，就不能再使用join()函数了；
- get_id：获取该线程的id；
- swap：将两个线程对象关联线程的状态进行交换；
