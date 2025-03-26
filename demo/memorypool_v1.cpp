#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#endif

inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
    void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    void* ptr = mmap(nullptr, kpage << 13, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) ptr = nullptr;
#endif
    if (ptr == nullptr)
        throw std::bad_alloc();
    return ptr;
}

template<class T>
class ObjectPool
{
public:
    // 申请空间
    T* New()
    {
        T* obj = nullptr; // 定义指向所申请空间的指针
        // 从自由链表中申请
        if (_freeList)
        {
            obj = (T*)_freeList;
            // 指向下一个
            _freeList = *(void**)obj;
        }
        else
        {
            // 扩容：如果剩余的内存块大小少于所申请的空间大小
            if (_SurplusBytes < sizeof(T))
            {
                size_t pageSize = 128; // 每次分配 128KB
                _memory = (char*)SystemAlloc(pageSize >> 13);
                if (_memory == nullptr)
                {
                    throw std::bad_alloc();
                }
                _SurplusBytes = pageSize;
            }
            obj = (T*)_memory;
            size_t OBJSIZE = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
            // 分配空间
            _memory += OBJSIZE;
            _SurplusBytes -= OBJSIZE;
        }
        // 对开辟好空间的obj进行初始化
        new(obj) T; // 使用定位new操作符对已经分配好的内存空间进行对象的构造（初始化）操作
        return obj;
    }

    // 将空间释放还回到自由链表中
    void Delete(T* obj)
    {
        // 将对象加入自由链表
        *(void**)obj = _freeList;
        _freeList = obj;
    }

    // 析构函数：释放所有分配的内存
    ~ObjectPool()
    {
        if (_memory) // 释放内存池
        {
#ifdef _WIN32
            VirtualFree(_memory, 0, MEM_RELEASE);
#else
            munmap(_memory, (_SurplusBytes + sizeof(T) - 1) >> 13 << 13); // 释放分配的内存页
#endif
        }
    }

private:
    char* _memory = nullptr; // 指向内存块的指针
    size_t _SurplusBytes = 0; // 内存块中剩余字节数
    void* _freeList = nullptr; // 管理还回来的内存对象的自由链表的头指针
};

struct TreeNode
{
    int _val;
    TreeNode* _left;
    TreeNode* _right;
    TreeNode()
        : _val(0), _left(nullptr), _right(nullptr)
    {}
};

void TestObjectPool()
{
    // 申请释放的轮次
    const size_t Rounds = 3;
    // 每轮申请释放多少次
    const size_t N = 100000;
    size_t begin1 = clock();
    std::vector<TreeNode*> v1;
    v1.reserve(N);
    for (size_t j = 0; j < Rounds; ++j)
    {
        for (int i = 0; i < N; ++i)
        {
            v1.push_back(new TreeNode);
        }
        for (int i = 0; i < N; ++i)
        {
            delete v1[i];
        }
        v1.clear();
    }
    size_t end1 = clock();

    // 使用定长内存池
    ObjectPool<TreeNode> TNPool;
    size_t begin2 = clock();
    std::vector<TreeNode*> v2;
    v2.reserve(N);
    for (size_t j = 0; j < Rounds; ++j)
    {
        for (int i = 0; i < N; ++i)
        {
            v2.push_back(TNPool.New());
        }
        for (int i = 0; i < N; ++i)
        {
            TNPool.Delete(v2[i]);
        }
        v2.clear();
    }
    size_t end2 = clock();
    std::cout << "new cost time: " << end1 - begin1 << " ticks" << std::endl;
    std::cout << "object pool cost time: " << end2 - begin2 << " ticks" << std::endl;
}

int main()
{
    TestObjectPool();
    return 0;
}