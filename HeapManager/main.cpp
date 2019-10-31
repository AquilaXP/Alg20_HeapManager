#include <iostream>
#include <stdexcept>
#include <mutex>

#include "AlgFit.h"
#include "ManagerMemory.h"
#include "TestBlock.h"
#include "TestBlockList.h"
#include "TestMM.h"

// Heap ОС
class AllocCpp : public IAllocator
{
public:
    void* Alloc( size_t size ) override
    {
        return ::operator new( size );
    }
    void Dealloc( void* ptr )
    {
        ::operator delete( ptr );
    }
    std::string Name() const override
    {
        return "new";
    }
    void Debug( std::ostream& os )
    {
    }
    size_t MemoryLeak() const override
    {
        return 0;
    }
    size_t CountReference() const override
    {
        return 0;
    }
};

#ifdef _WIN64
const size_t SizePullManagerMemory = size_t( 2_GB );
#else
const size_t SizePullManagerMemory = size_t(128_MB);
#endif

// For my heap
template<class T>
class AllocTemplate : public IAllocator
{
public:
    AllocTemplate()
    {
        c = new T( new char[SizePullManagerMemory], SizePullManagerMemory );
    }
    void* Alloc( size_t size ) override
    {
        std::lock_guard lock( m_lock );
        return c->Alloc( size );
    }
    void Dealloc( void* ptr )
    {
        std::lock_guard lock( m_lock );
        c->Dealloc( ptr );
    }
    std::string Name() const override
    {
        return typeid(T).name();
    }
    void Debug( std::ostream& os )
    {
        c->Debug( os );
    }
    size_t MemoryLeak() const override
    {
        return c->MemoryLeak();
    }
    size_t CountReference() const override
    {
        return c->CountAllocation();
    }
private:
    std::mutex m_lock;
    T* c = nullptr;
};


int main()
{
    try
    {
        // unit-test
        TestBlock2 t;
        t.Execute();
        TestBlockList t2;
        t2.Execute();
    }
    catch( const std::exception& e )
    {
        std::cerr << "Test fail!\n";
        std::cerr << e.what() << '\n';
        return 1;
    }
    
    try
    {
        TestMM testmm;
        // Добавляем тестируемые экземпляры
        testmm.PushAllocator( new AllocCpp );
        testmm.PushAllocator( new AllocTemplate<ManagerMemory<FirstFit>> );
        testmm.PushAllocator( new AllocTemplate<ManagerMemory<BestFit>> );
        testmm.PushAllocator( new AllocTemplate<ManagerMemory<NextFit>> );

        // количество выделяемых блоков
        size_t count = 10000;
        // количество итераций для рандомных аллокаций/деаллокаций
        size_t iter = 10000;

        // варианты максимальных блоков
        std::vector< size_t > sizemem 
            = { 1, 4, 8, 32, 64, 256, 512, size_t( 4_KB ), size_t( 32_KB ), size_t( 1_MB ), size_t( 32_MB ) };

        for( auto& s : sizemem )
        {
            testmm.SetParam( s, std::min( SizePullManagerMemory/s, count ), iter );
            testmm.Execute( std::cout );
        }
    }
    catch( const std::exception& err )
    {
        std::cerr << "Fail testing perfomance!\n";
        std::cerr << err.what() << '\n';
        return 1;
    }

    return 0;
}
