#include "MemoryPool.h"

namespace memoryPool
{
    MemoryPool::MemoryPool(size_t BlockSize)
        : BlockSize_(BlockSize), SlotSize_(0), firstBlock_(nullptr), curSlot_(nullptr), freeList_(nullptr), lastSlot_(nullptr)
    {
    }

    MemoryPool::~MemoryPool()
    {
        Slot *cur = firstBlock_;
        while (cur)
        {
            Slot *next = cur->next;
            // 转化为 void 指针，因为 void 类型不需要调用析构函数，只释放空间
            operator delete(reinterpret_cast<void *>(cur));
            cur = next;
        }
    }

    void MemoryPool::init(size_t size)
    {
        assert(size > 0);
        SlotSize_ = size;
        firstBlock_ = nullptr;
        curSlot_ = nullptr;
        freeList_ = nullptr;
        lastSlot_ = nullptr;
    }

    void *MemoryPool::allocte()
    {
        Slot *slot = popFreeList();
        if (slot != nullptr)
        {
            return slot;
        }

        Slot *temp;
        {
            // std::lock_guard<std::mutex> 是啥？
            std::lock_guard<std::mutex> lock(mutexForBlock_);
            if (curSlot_ >= lastSlot_)
            {
                allocateNewBlock();
            }

            temp = curSlot_;
            curSlot_ += SlotSize_ / sizeof(Slot);
        }
        return temp;
    }

    void MemoryPool::deallocate(void *ptr)
    {
        if (!ptr)
            return;

        Slot *slot = reinterpret_cast<Slot *>(ptr);
        pushFreeList(slot);
    }

    void MemoryPool::allocateNewBlock()
    {
        std::cout << "申请一块内存块，SlotSize: " << SlotSize_ << std::endl;
        void* newBlock = operator new(BlockSize_);
        reinterpret_cast<Slot *>(newBlock)->next = firstBlock_;
        firstBlock_ = reinterpret_cast<Slot *>(newBlock);

        
    }

}