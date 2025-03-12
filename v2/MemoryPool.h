#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
namespace memoryPool
{
#define MEMORY_POOL_NUM 64
#define SLOT_BASE_SIZE 8
#define SLOT_MAX_SIZE 512

    struct Slot
    {
        std::atomic<Slot *> next;
    };

    class MemoryPool
    {
    public:
        MemoryPool(size_t BlockSize = 4096);
        ~MemoryPool();
        void init(size_t);
        void *allocte();
        void deallocate(void *p);

    private:
        void allocateNewBlock();
        size_t padPointer(char *p, size_t align);
        bool pushFreeList(Slot *slot);
        Slot *popFreeList();

    private:
        int BlockSize_;
        int SlotSize_;
        Slot *firstBlock_;            // 指向内存池管理的首个实际内存块
        Slot *curSlot_;               // 指向当前内存块的第一个空闲内存槽
        Slot freeList_;               // 空闲内存槽链表
        Slot *lastSlot_;              // 指向当前内存块的最后一个空闲内存槽
        std::mutex mutexForFreeList_; // 保证freeList_的线程安全
        std::mutex mutexForBlock_;    // 避免重复开辟内存
    };

    class HashBucket
    {
    public:
        // 为什么使用static？
        static void initMemoryPool();
        static MemoryPool &getMemoryPool(int index);
        static void *useMemory(size_t size)
        {
            if (size <= 0)
            {
                return nullptr;
            }
            else if (size <= SLOT_MAX_SIZE)
            {
                return operator new(size);
            }
            return getMemoryPool(size / SLOT_BASE_SIZE).allocte();
        }

        static void freeMemory(void *p, size_t size)
        {
            if (p == nullptr)
            {
                return;
            }
            else if (size > SLOT_MAX_SIZE)
            {
                operator delete(p);
                return;
            }
            getMemoryPool(size / SLOT_BASE_SIZE).deallocate(p);
        }

        template <typename T, typename... Args>
        friend T *newElement(Args &&...args);

        template <typename T>
        friend void deleteElement(T *p);
    };

    template <typename T, typename... Args>
    T *newElement(Args &&...args)
    {
        T *p = nullptr;
        if ((p = reinterpret_cast<T *>(HashBucket::useMemory(sizeof(T)))) != nullptr)
        {
            new (p) T(std::forward<Args>(args)...);
        }
        return p;
    }

    template <typename T>
    void deleteElement(T *p)
    {
        if (p != nullptr)
        {
            p->~T();
            HashBucket::freeMemory(reinterpret_cast<void *>(p), sizeof(T));
        }
    }

} // namespace memoryPool