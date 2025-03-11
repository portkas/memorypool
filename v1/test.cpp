#include "MemoryPool.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>

// 定义一个简单的测试类 
class TestObject {
public:
    int value;

    TestObject() : value(0) {
        std::cout << "TestObject constructed with value 0\n";
    }

    TestObject(int val) : value(val) {
        std::cout << "TestObject constructed with value " << val << "\n";
    }

    ~TestObject() {
        std::cout << "TestObject destructed with value " << value << "\n";
    }
};

// 测试内存池的基本功能
TEST(MemoryPoolTest, BasicFunctionality) {
    MemoryPool<TestObject, 4096> pool;

    // 分配一个对象
    TestObject* obj1 = pool.newElement(10);
    EXPECT_EQ(obj1->value, 10);

    // 分配第二个对象
    TestObject* obj2 = pool.newElement(20);
    EXPECT_EQ(obj2->value, 20);

    // 删除第一个对象
    pool.deleteElement(obj1);

    // 再次分配，应该复用之前释放的内存
    TestObject* obj3 = pool.newElement(30);
    EXPECT_EQ(obj3->value, 30);

    // 删除所有对象
    pool.deleteElement(obj2);
    pool.deleteElement(obj3);
}

// 测试内存池的线程安全性（虽然代码本身没有线程安全机制）
TEST(MemoryPoolTest, ThreadSafety) {
    MemoryPool<TestObject, 4096> pool;
    const int numThreads = 10;
    const int numAllocationsPerThread = 100;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&pool, numAllocationsPerThread]() {
            for (int j = 0; j < numAllocationsPerThread; ++j) {
                TestObject* obj = pool.newElement(j);
                pool.deleteElement(obj);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

// 性能测试：对比标准分配器和内存池
TEST(MemoryPoolTest, Performance) {
    const int numAllocations = 100000;

    // 测试标准分配器
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numAllocations; ++i) {
        TestObject* obj = new TestObject(i);
        delete obj;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> stdDuration = end - start;
    std::cout << "Standard allocator time: " << stdDuration.count() << " seconds\n";

    // 测试内存池
    MemoryPool<TestObject, 4096> pool;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numAllocations; ++i) {
        TestObject* obj = pool.newElement(i);
        pool.deleteElement(obj);
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> poolDuration = end - start;
    std::cout << "Memory pool time: " << poolDuration.count() << " seconds\n";

    // 比较性能
    EXPECT_LT(poolDuration.count(), stdDuration.count());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}