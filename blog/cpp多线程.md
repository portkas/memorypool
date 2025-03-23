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



# this_thread类成员函数

- get_id：获取当前线程ID
- sleep_for：当前线程休眠一个时间段
- sleep_until：当前线程休眠到一个具体的时间
- yield：当前线程放弃执行，让操作系统调度另一个线程继续执行


### yield

yield主要用于无锁编程，当原子操作不断尝试，时间过久的时候会降低整体效率，使用yield主动让出当前时间片，避免大量重复，把CPU资源让出去，从而提高整体效率；


# 线程参数

如果需要通过线程函数修改外部变量，可以使用以下方法：

1. 使用 `std::ref` 包装引用。
2. 使用指针传递变量地址。
3. 使用 Lambda 表达式捕获变量的引用。

```cpp
#include <iostream>
#include <thread>
using namespace std;

void threadFunction1(int &num)
{
    num += 10;
}

void threadFunction2(int *x)
{
    *x += 10;
}

int main()
{
    int num = 0;
    int x = 0;
    int y = 0;
    std::thread t1(threadFunction1, ref(num));
    std::thread t2(threadFunction2, &x);
    std::thread t3([&y]()
                   { y += 10; });
    t1.join();
    t2.join();
    t3.join();
    std::cout << num << std::endl;
    std::cout << x << std::endl;
    std::cout << y << std::endl;
    return 0;
}
```


# 互斥锁

在多线程编程中，多个线程同时访问和操作共享资源，可能会产生数据不一致或者冲突，这种情况称为竞态条件；

锁是一种机制，用来确保在同一时刻只有一个线程可以访问共享资源，通过使用锁来防止多个线程同时修改共享资源，从而保证数据的一致性和正确性；

1. std::mutex
2. std::recursive_mutex
3. std::timed_mutex
4. std::recursive_timed_mutex

### mutex

mutex是最基本的互斥锁，mutex对象之间不能进行拷贝，也不能进行移动；

- lock：对互斥量进行加锁
- try_lock：尝试对互斥量进行加锁
- unlock：对互斥量进行解锁，释放互斥量的所有权

### recursive_mutex

递归互斥锁，自己在持有锁资源的情况下，不再申请锁；

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
using namespace std;

recursive_mutex recur_mutex;

void func(int n)
{
    if(n == 0)
    {
        return;
    }

    recur_mutex.lock();
    n--;
    func(n);
    recur_mutex.unlock();
}

int main()
{
	int n = 1000;
	thread t1(func, n);
	thread t2(func, n);

	t1.join();
	t2.join();
	return 0;
}
```

这里如果用普通锁的话，就会出现死锁现象，因为再进入递归函数之前，申请了锁资源还没来得及释放锁资源，又再次申请了锁资源，出现了锁在我手里，但是我申请不到的现象；

### timed_mutex

时间互斥锁增加了定时解锁的功能，可以再程序运行指定时间后，自动解锁（如果还没有解锁的话）；

- try_lock_for：是按照相对时间进行解锁；
- try_lock_until：是按照绝对时间进行自动解锁；

### std::recursive_timed_mutex

递归时间互斥锁，具备递归和定时的功能，是上述两个锁的结合；


# RAII风格的锁

### lock_guard

std::lock_guard是C++标准库中的一个模板类，用于实现资源的自动加锁和解锁，它是基于RAII（资源获取即初始化）的设计理念，能够确保再作用域结束的时候自动释放锁资源，避免了手动管理锁的复杂性和可能出现的错误

- 自动加锁：在创建std::lock_guard对象的时候，会立即对指定的互斥量进行加锁操作，确保在进入作用域后，互斥量已经被锁定；
- 自动解锁：std::lock_guard对象在作用域结束之后，会自动调用析构函数释放锁资源，避免资源泄露和死锁；
- 适用于局部锁定：std::lock_guard是通过栈上对象实现的，适用于局部分为内锁定互斥量；

### unique_lock

提供了比std::lock_guard更多的功能和控制能力：

- 自动管理锁的生命周期：std::unique_lock在对象被销毁时会自动释放互斥量；
- 支持手动加锁和解锁：可以在构造时选择不立即加锁，稍后手动调用lock/unlock自行加锁和解锁；
- 支持延迟加锁：即支持手动加锁和解锁；
- 与条件变量配合使用：方便和条件变量配合使用；

自动加锁和解锁：

```cpp
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;

void print_block(int n, char c) {
    // 创建 std::unique_lock 对象时自动加锁
    std::unique_lock<std::mutex> lock(mtx);
    for (int i = 0; i < n; ++i) {
        std::cout << c;
    }
    std::cout << '\n';
    // 对象销毁时自动解锁
}

int main() {
    std::thread t1(print_block, 50, '*');
    std::thread t2(print_block, 50, '$');

    t1.join();
    t2.join();

    return 0;
}
```

手动加锁和解锁：

```cpp
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;

void print_block(int n, char c) {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock); // 延迟加锁
    // 手动加锁
    lock.lock();
    for (int i = 0; i < n; ++i) {
        std::cout << c;
    }
    std::cout << '\n';
    // 手动解锁
    lock.unlock();
}

int main() {
    std::thread t1(print_block, 50, '*');
    std::thread t2(print_block, 50, '$');

    t1.join();
    t2.join();

    return 0;
}
```

与条件变量配合使用：

```cpp
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void print_id(int id) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; }); // 等待条件变量，自动解锁并重新加锁
    std::cout << "Thread " << id << '\n';
}

void go() {
    std::unique_lock<std::mutex> lock(mtx);
    ready = true;
    lock.unlock();
    cv.notify_all(); // 通知所有等待的线程
}

int main() {
    std::thread threads[10];
    for (int i = 0; i < 10; ++i) {
        threads[i] = std::thread(print_id, i);
    }

    std::cout << "10 threads ready to race...\n";
    go(); // 启动线程

    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
```


# 条件变量

std::condition_variable是C++标准库中的一个类，用于多线程编程中实现线程间的条件变量和线程同步；提供了等待和通知的机制，使得线程可以等待某个条件成立时被唤醒，或者在满足某个条件时通知其他等待的线程；

- wait：使当前线程进入等待状态，直到被其他线程通过notify_one或者notify_all函数唤醒；该函数需要一个互斥锁作为参数，调用时会自动释放互斥锁，并在被唤醒后重新获取互斥锁；
- wait_for：使当前线程进入等待状态，最多等待一定的时间；该函数需要一个互斥锁和一个时间段作为参数，如果等待超时返回：**std::cv_status::timeout**，否则返回：**std::cv_status::no_timeout**
- wait_until：使当前线程进入等待状态，直到被其他线程通过notify_one或者notify_all函数唤醒，或者等待时间达到指定的绝对时间点。该函数需要一个互斥锁和一个绝对时间点作为参数，返回值同wait_for
- notify_one：唤醒一个等待中的线程，如果多个线程在等待，则选择其中一个线程唤醒；
- notify_all：唤醒所有等待中的线程，使它们从等待状态返回；

特点：

- 等待和通知机制：std::condition_variable允许线程进入等待状态，直到某个条件满足时才被唤醒，线程可以调用wait函数进入等待状态，并指定一个互斥量作为参数，以确保线程在等待期间互斥量被锁定，当其他线程满足条件并调用notify_one或者notify_all函数时，等待的线程将被唤醒并继续执行；
- 与互斥锁配合使用：std::condition_variable需要与互斥量（`std::mutex`或 `std::unique_lock<std::mutex>`）配合使用，以确保线程之间的互斥性，在等待之前线程必须先锁定互斥量，以避免竞争条件，当条件满足时，通知其他等待的线程之前，必须再次锁定互斥量；
- 支持超时等待：std::condition_variable提供了带有超时参数的等待函数wait_for和wait_until，允许线程在等待一段时间后自动被唤醒，用于处理超时或限时等待；

使用std::condition_variable的步骤：

1. 创建一个std::condition_variable对象；
2. 创建一个互斥量对象（`std::mutex`或 `std::unique_lock<std::mutex>`）；
3. 在等待线程中，使用std::unique_lock锁定互斥量，并调用wait函数进入等待状态；
4. 在唤醒线程中，使用std::unique_lock锁定互斥量，并调用notify_one或notify_all函数通知等待的线程；
5. 等待线程被唤醒后继续执行相应操作；
