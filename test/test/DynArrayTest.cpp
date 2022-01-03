#include <gtest/gtest.h>
#include "../../src/DynArray.h"

struct POD {
    int A;
};

using SmallArray = tinyxml2::DynArray<POD,5>;

void pushItem(const POD& item, SmallArray& array)
{
    array.Push(item);
}

TEST(DynArrayTest, ConstructorTest) {
    SmallArray dynArray;
    auto [memptr, allocated, size] = dynArray.GetState();
    EXPECT_EQ(allocated, 5);
    EXPECT_EQ(size, 0);
}

TEST(DynArrayTest, PushTest) {
    SmallArray dynArray;
    pushItem(POD{42}, dynArray);
    auto [memptr, allocated, size] = dynArray.GetState();
    EXPECT_EQ(size, 1);
    EXPECT_EQ(allocated, 5);
}

TEST(DynArrayTest, ClearTest) {
    SmallArray dynArray;
    pushItem(POD{42}, dynArray);
    dynArray.Clear();
    auto [memptr, allocated, size] = dynArray.GetState();
    EXPECT_EQ(size, 0);
    EXPECT_EQ(allocated, 5);
}

TEST(DynArrayTest, PushArrTest) {
    const int arraySize {20};
    SmallArray dynArray;
    POD* items = dynArray.PushArr(arraySize);
    auto [memptr, allocated, size] = dynArray.GetState();
    EXPECT_EQ(size, arraySize);
    EXPECT_EQ(allocated, 40);
    items[0].A = 33;
    EXPECT_EQ(dynArray[0].A, 33);
    items[arraySize-1].A = 99;
    EXPECT_EQ(dynArray[arraySize-1].A, 99);

}

TEST(DynArrayTest, PopTest) {
    SmallArray dynArray;
    pushItem(POD{42}, dynArray);
    POD item = dynArray.Pop();
    auto [memptr, allocated, size] = dynArray.GetState();
    EXPECT_EQ(size, 0);
    EXPECT_EQ(allocated, 5);
    EXPECT_EQ(item.A, 42);
}

TEST(DynArrayTest, PopArrTest) {
    const int arraySize {20};
    SmallArray dynArray;
    POD* items = dynArray.PushArr(arraySize);
    auto [memptr, allocated, size] = dynArray.GetState();
    EXPECT_EQ(size, arraySize);
    EXPECT_EQ(allocated, 40);
    dynArray.PopArr(19);
    auto [memptr1, allocated1, size1] = dynArray.GetState();
    EXPECT_EQ(size1, 1);
    EXPECT_EQ(allocated1, 40);
}

TEST(DynArrayTest, EmptyTest) {
    SmallArray dynArray;
    pushItem(POD{42}, dynArray);
    EXPECT_FALSE(dynArray.Empty());
    dynArray.Pop();
    EXPECT_TRUE(dynArray.Empty());
}

TEST(DynArrayTest, BracketOperatorTest) {
    SmallArray dynArray;
    pushItem(POD{42}, dynArray);
    POD& item = dynArray[0];
    EXPECT_EQ(item.A, 42);
    item.A = 33;
    const POD& citem = dynArray[0];
    EXPECT_EQ(citem.A, 33);
}

TEST(DynArrayTest, PeekTopTest) {
    SmallArray dynArray;
    pushItem(POD{42}, dynArray);
    const POD& citem = dynArray.PeekTop();
    EXPECT_EQ(citem.A, 42);
}

TEST(DynArrayTest, SizeTest) {
    const int arraySize {20};
    SmallArray dynArray;
    POD* items = dynArray.PushArr(arraySize);
    EXPECT_EQ(dynArray.Size(), 20);
}

TEST(DynArrayTest, CapacityTest) {
    const int arraySize {20};
    SmallArray dynArray;
    POD* items = dynArray.PushArr(arraySize);
    EXPECT_EQ(dynArray.Capacity(), 40);
}

TEST(DynArrayTest, SwapRemoveTest) {
    
    SmallArray dynArray;
    for (int i=0; i < 10; i++) {
        pushItem(POD{i}, dynArray);
    }
    dynArray.SwapRemove(1);
    EXPECT_EQ(dynArray.Size(), 9);
    EXPECT_EQ(dynArray[0].A, 0);
    EXPECT_EQ(dynArray[1].A, 9);
}

TEST(DynArrayTest, MemTest) {

    SmallArray dynArray;
    for (int i=0; i < 10; i++) {
        pushItem(POD{i}, dynArray);
    }
    POD* thePod = dynArray.Mem();
    EXPECT_NE(thePod, nullptr);
    thePod[1].A = 33;
    const POD* theCPod = dynArray.Mem();
    EXPECT_EQ(theCPod[1].A, 33);
}
