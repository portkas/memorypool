

# std::alloctor

分配器：定义内存模型的类。

头文件：`<memory>`

```cpp
template <> class allocator<void> {
public:
  typedef void* pointer;
  typedef const void* const_pointer;
  typedef void value_type;
  template <class U> struct rebind { typedef allocator<U> other; };
};
```




# std::alloctor_traits

为分配器类型提供一个统一的接口。



# STL中的内存分配器

- 一级内存分配器 `__malloc_alloc_template`（malloc/free）
- 二级内存分配器 `__default_alloc_template`（memorypool/freelist）

如果要分配的内存大于128字节，调用一级内存分配器直接向系统申请，如果小于等于128字节则去内存池中申请。

## 一级内存分配器

1. 当malloc申请内存失败后：
   * 如果客户端没有设置内存不足处理机制，则直接抛出异常信息，终止程序；
   * 如果有设置，则调用内存处理机制，释放一块内存，以满足申请内存的需求；
2. 内存不足处理机制 `__malloc_alloc_oom_handle`


## 二级内存分配器



# placement new

## 基本介绍

`placemen new`是C++中一种特殊的 `new`操作符，是重载 `operator new`的一个全局版本，不能用户自定义。它用于在已分配的内存上构造对象（它不负责分配新的内存）。

函数原型：

```cpp
void* operator new(size_t void *p) throw(){
   return p;
}
```

函数语法：

```cpp
new (pointer_to_memory) Type(constructor_arguments);
```

- `pointer_to_memory`：一个指向已分配内存的指针；
- `Type`：要构造的对象类型；
- `constructor_argument`：构造函数的参数；


## 函数用法

1. 分配内存
2. 构造对象
3. 销毁对象
4. 释放内存

```cpp
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
```

## 注意事项

1. `placement new`用于在已分配的内存上构造函数，不分配新内存；
2. 当使用 `placement new`构造对象的时候，必须显示调用析构函数来销毁对象；
3. 如果使用 `placement new[]`需要申请比原始对象大小多出sizeof(int)个字节来存访对象的个数；


## new

`new`是C++中用于动态分配内存并构造对象的运算符；

- `new`会调用 `operator new`来分配内存；
- 分配内存后，`new`会调用对象的构造函数来初始化对象；

```cpp
Type* ptr = new Type(constructor_arguments);
```

这里的 `new`实际上是执行了三个过程：

1. 调用 `operator new`分配内存；
2. 调用构造函数生成类对象；
3. 返回相应指针；


## operator new

`operator new`是一个函数，用于分配内存，并且只负责分配内存，不负责构造对象。

- 可以重载（一般只能在类中进行重载）；
- `operator[]`,`delete`,`delete[]`也是可以重载的；


# union

## 基本介绍

union表示，所有的成员共用一个空间，并且同一时间只能存储其中一个成员变量的值。

- 内存共享：共用体所有成员共享同一块内存，所以在任何时刻，共用体变量中只能存储其中一个成员的值；
- 内存大小：共用体的大小由其最大成员的大小决定；
- 访问成员：访问共用体成员的时候，必须明确指定成员的名称，`bar.i`；

```cpp
union foo {
    int i;
    char c;
    double k;
};

union foo bar;
```

`union foo bar;`表示定义了一个名为 `bar`类型的共用体变量，它属于 `foo`，其内部的i,c,k成员共享同一块内存；

## 示例

```cpp
#include <iostream>
using namespace std;

union StateMachine
{
    char character;
    int number;
    char *str;
    StateMachine(char c)
    {
        character = c;
    }
    StateMachine(int n)
    {
        number = n;
    }
    StateMachine(char *s)
    {
        str = s;
    }
};
enum State
{
    character,
    number,
    str
};

int main()
{
    State state = character;
    StateMachine machine('J');
    if (state == character)
        cout << machine.character << endl;
    else if (state == number)
        cout << machine.number << endl;
    else if (state == str)
        cout << machine.str << endl;
    return 0;
}

```

## 注意事项

共用体的成员不能是静态成员或引用类型。如果成员是自定义类型，则该自定义类型不能有构造函数、析构函数或复制构造函数等特殊成员函数。

# 内存碎片

1、什么是内部碎片？什么是外部碎片？

内部碎片是已经分配的内存块，没有被使用的部分。通常内存分配器分配内存是按照固定的对齐规则来分配内存的。比如按照4字节对齐的话，如果需要申请13个字节的内存，那么内存分配器实际上会按照对齐规则分配16个字节，那么就会多出来3个字节的内存不会被使用，这就是内部碎片。

外部碎片是还没分配的内存，但是内存不连续，不够一个较大的内存分配请求。通常是由于，频繁的申请和释放较小的内存，导致内存块变得分散。比如内存有5个100MB的内存块，但是他们不连续，如果这个时候想申请一个500MB的内存，就不太行。这个就是外部碎片。


2、小内存频繁申请带来的性能问题

- 分配器找一个空闲内存块给用户的时候需要时间；
- 当外部碎片比较多，不能满足分配需求的时候，合并空闲块需要时间；
- malloc申请内存块时，会附带记录内存块信息的头，也会造成空间的浪费；

# rebind

`rebind`是一个模板结构体，用于将分配器的类型从一种类型 `T`绑定到另一种类型 `U`。

```cpp
template <typename U> struct rebind {
    typedef MemoryPool<U> other;
};
```

这段代码的作用是允许 `MemoryPool`类型分配器在需要是重新绑定到另一种类型U：

* `rebind` 是一个模板结构体，模板参数 `U` 表示新的类型。
* `other` 是一个类型别名，表示重新绑定后的分配器类型，即 `MemoryPool<U>`。

示例：

```cpp
MemoryPool<int> intAllocator; // 分配器用于 int 类型
MemoryPool<int>::rebind<char>::other charAllocator; // 重新绑定为 char 类型
```

* `MemoryPool<int>` 是一个分配器，用于分配 `int` 类型的对象。
* 通过 `rebind<char>::other`，我们得到了一个 `MemoryPool<char>` 类型的分配器，用于分配 `char` 类型的对象。


# 内存池
