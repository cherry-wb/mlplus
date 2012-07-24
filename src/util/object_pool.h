#ifndef MLPLUS_OBJECT_POOL_H
#define MLPLUS_OBJECT_POOL_H
#include <cstddef>
#include <vector>

namespace mlplus
{

template <typename T>
class ObjectPool
{
public:
    ObjectPool(int blockSize = 512);
    ~ObjectPool();
public:
    void * alloc(size_t size);
    void free(void *mem, size_t size);

private:
    //make sure allocated memory can hold a pointer
    union AllocateElement
    {
        char mPlacehodler[sizeof(T)];
        AllocateElement *mNext;
    };
    int mBlockSize;
    AllocateElement *mHeadOfFreeList;
    std::vector<void *> mBlockMem;
};

template <typename T>
ObjectPool<T>::ObjectPool(int blockSize)
{
    mBlockSize = blockSize;
    mHeadOfFreeList = NULL;
}

template <typename T>
ObjectPool<T>::~ObjectPool()
{
    std::vector<void *>::iterator it = mBlockMem.begin();
    for(; it != mBlockMem.end(); it++)
    {
        ::operator delete(*it);
    }
}

template <typename T>
void * ObjectPool<T>::alloc(size_t size)
{
    AllocateElement *p = mHeadOfFreeList;
    if(p != NULL)
    {
        mHeadOfFreeList = p->mNext;
    }
    else
    {
        void *blockMem = ::operator new(mBlockSize * sizeof(AllocateElement));
        AllocateElement *newMem = static_cast<AllocateElement *>(blockMem);

        //record begin address of block
        mBlockMem.push_back(blockMem);

        //create free list
        for(int i = 1; i < (mBlockSize - 1); i++)
        {
            newMem[i].mNext = newMem + i + 1;
        }

        newMem[mBlockSize - 1].mNext = NULL;

        p = newMem;
        mHeadOfFreeList = newMem + 1;
    }
    return static_cast<void *>(p);
}

template <typename T>
void ObjectPool<T>::free(void *mem, size_t size)
{
    if(mem == NULL)
    {
        return;
    }
    AllocateElement *objMem = static_cast<AllocateElement *>(mem);
    objMem->mNext = mHeadOfFreeList;
    mHeadOfFreeList = objMem;
}

/*
#define DECLARE_MEMORY_POOL(Cls) \
  public: \
  static void* operator new(size_t s) \
  { \
          assert(s == sizeof(Cls)); \
          return MemoryPool<sizeof(Cls)> \
                 ::instance().allocate(); \
        } \
  static void operator delete(void* p) \
  { \
          if (p != NULL) \
            MemoryPool<sizeof(Cls)> \
            ::instance().deallocate(p); \
  }

class Obj {
      ...
      DECLARE_MEMORY_POOL(Obj)
};
*/
}
#endif
