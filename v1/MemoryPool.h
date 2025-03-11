#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <limits.h>
#include <stddef.h>

// 内存块默认大小设置为4096字节
template <typename T, size_t BlockSize = 4096>
class MemoryPool
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <typename U> struct rebind {
        typedef MemoryPool<U> other;
    };

    // 构造函数
    MemoryPool() throw();
    MemoryPool(const MemoryPool& memoryPool) throw();
    template <class U> MemoryPool(const MemoryPool<U>& memoryPool) throw();

    // 析构函数
    ~MemoryPool() throw();

    // 返回指向元素的指针
    pointer address(reference x) const throw();
    const_pointer address(const_reference x) const throw();

    // 分配和回收一个元素的内存
    pointer allocate(size_type n = 1, const_pointer hint = 0);
    void deallocate(pointer p, size_type n = 1);

    // 返回可成功分配的最大元素数量
    size_type max_size() const throw();

    // 基于内存池的元素构造和析构
    void construct(pointer p, const_reference val);
    void destroy(pointer p);

    // 申请内存和释放内存的构造与析构
    pointer newElement(const_reference val);
    void deleteElement(pointer p);

private:
    // union Slot_是一个联合体，用于存放元素和指向下一个内存块的指针
    union Slot_ {
        value_type element;
        Slot_* next;
    };

    typedef char* data_pointer_;
    typedef Slot_ slot_type_;
    typedef Slot_* slot_pointer_;

    slot_pointer_ currentBlock_; // 指向当前内存块的链表头
    slot_pointer_ currentSlot_;  // 指向当前内存块中下一个可用的元素
    slot_pointer_ lastSlot_;     // 指向当前内存块中最后一个可用的元素
    slot_pointer_ freeSlots_;    // 指向已释放的 Slot_ 的链表头

    size_type padPointer(data_pointer_ p, size_type align) const throw();
    void allocateBlock();
    static_assert(BlockSize >= 2 * sizeof(slot_type_), "BlockSize too small.");
};


#include "MemoryPool.h"

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::padPointer(data_pointer_ p, size_type align) const throw()
{
    size_t result = reinterpret_cast<size_t>(p);
    return ((align - result) % align);
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() throw()
{
    currentBlock_ = 0;
    currentSlot_ = 0;
    lastSlot_ = 0;
    freeSlots_ = 0;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& memoryPool) throw()
{
    MemoryPool();
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() throw()
{
    slot_pointer_ curr = currentBlock_;
    while (curr != 0)
    {
        slot_pointer_ prev = curr->next;
        operator delete(reinterpret_cast<void*>(curr));
        curr = prev;
    }
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::address(reference x) const throw()
{
    return &x;
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::const_pointer
MemoryPool<T, BlockSize>::address(const_reference x) const throw()
{
    return &x;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock()
{
    data_pointer_ newBlock = reinterpret_cast<data_pointer_>(operator new(BlockSize));
    reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
    currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
    data_pointer_ body = newBlock + sizeof(slot_pointer_);
    size_type bodyPadding = padPointer(body, sizeof(slot_type_));
    currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
    lastSlot_ = reinterpret_cast<slot_pointer_>(newBlock + BlockSize - sizeof(slot_type_) + 1);
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::allocate(size_type n, const_pointer hint)
{
    if (freeSlots_ != 0)
    {
        pointer result = reinterpret_cast<pointer>(freeSlots_);
        freeSlots_ = freeSlots_->next;
        return result;
    }
    else
    {
        if (currentSlot_ >= lastSlot_)
            allocateBlock();
        return reinterpret_cast<pointer>(currentSlot_++);
    }
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::deallocate(pointer p, size_type n)
{
    if (p != 0)
    {
        reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
        freeSlots_ = reinterpret_cast<slot_pointer_>(p);
    }
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::max_size() const throw()
{
    size_type maxBlocks = -1 / BlockSize;
    return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::construct(pointer p, const_reference val)
{
    new (p) value_type(val);
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::destroy(pointer p)
{
    p->~value_type();
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::newElement(const_reference val)
{
    pointer result = allocate();
    construct(result, val);
    return result;
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::deleteElement(pointer p)
{
    if (p != 0)
    {
        p->~value_type();
        deallocate(p);
    }
}

#endif // MEMORY_POOL_H