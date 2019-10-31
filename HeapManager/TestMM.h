#pragma once

#include <sstream>
#include <random>
#include <string>
#include <chrono>
#include <iomanip>

class Timer
{
    using select_clock = std::chrono::high_resolution_clock;
public:
    Timer() = default;

    void start()
    {
        t_start = select_clock::now();
    }

    void stop()
    {
        t_end = select_clock::now();
    }

    template< class T >
    size_t t()
    {
        return static_cast<size_t>( std::chrono::duration_cast<T>( t_end - t_start ).count() );
    }

    size_t t()
    {
        return t<std::chrono::milliseconds>();
    }

    int rel()
    {
        auto[baseT, dt] = baseT_dt();

        return static_cast<int>(dt * 100.0 / baseT);
    }

    double speedup()
    {
        auto[baseT, dt] = baseT_dt();

        return double( baseT ) / dt;
    }

    void print( std::ostream& os )
    {
        print( os, "milliseconds" );
    }

    void print( std::ostream& os, const std::string& duration )
    {
        os << "\t" << duration << ": " << std::setw( 7 ) << t<std::chrono::milliseconds>() <<
            "\trelative time:" << std::setw( 4 ) << std::setprecision(2) << rel() << "%" <<
            "\tspeed-up factor: " << std::setw( 2 ) << std::setprecision( 3 ) << (speedup() > 0.01 ? speedup() : 0.0);
    }
private:
    std::pair< size_t, size_t > baseT_dt()
    {
        using namespace std::chrono;

        if( t_base.count() == 0 )
        {
            t_base = t_end - t_start;
        }
        size_t dt = static_cast<size_t>( duration_cast<nanoseconds>( t_end - t_start ).count() );
        size_t base = static_cast<size_t>( duration_cast<nanoseconds>( t_base ).count() );

        return std::make_pair( base, dt );
    }

    select_clock::time_point t_start = {};
    select_clock::time_point t_end = {};
    select_clock::duration   t_base = {};
};

unsigned long long operator ""_KB( unsigned long long v )
{
    return v * 1024;
}

unsigned long long operator ""_MB( unsigned long long v )
{
    return v * 1024 * 1024;
}

unsigned long long operator ""_GB( unsigned long long v )
{
    return v * 1024 * 1024 * 1024;
}

class IAllocator
{
public:
    virtual void* Alloc( size_t size ) = 0;
    virtual void Dealloc( void* ptr ) = 0;
    virtual std::string Name() const = 0;
    virtual void Debug( std::ostream& os ) = 0;
    virtual size_t MemoryLeak() const = 0;
    virtual size_t CountReference() const = 0;
};

class TestMM
{
    size_t m_size;
    size_t m_count;
    size_t m_random_iteration;
    size_t m_max_w_name = 0;
    bool m_debug = false;
public:
    void SetParam( size_t size, size_t count, size_t random_iteration )
    {
        m_size = size;
        m_count = count;
        m_random_iteration = random_iteration;
    }
    void debug( bool useDebug = true )
    {
        m_debug = useDebug;
    }
    void PushAllocator( IAllocator* alloc )
    {
        m_Alloces.push_back( alloc );
        if( alloc->Name().size() > m_max_w_name )
            m_max_w_name = alloc->Name().size();
    }
    void Execute( std::ostream& os )
    {
        using namespace std;
        using namespace chrono;

        os << endl << endl;
        os << "Allocator Benchmark Tests with " << m_size << " bytes objects " << endl;
        os << endl;
        for( auto& a : m_Alloces )
        {
            os << a->Name() << '\n';
        }
        os << endl << endl;

        delete_new( m_count, m_size, os );
        debug( os );

        newnew_deletedelete( m_count, m_size, os );
        debug( os );

        random( m_random_iteration, std::min(sizeof( size_t ), m_size ), m_size, os );
        debug( os );

        // Отличие от random в том что пытаемся сделать сразу множество выделений памяти
        // и также освобождений ( в данном случае от 5 до 50 )
        random_packet( 5, 50, m_random_iteration/10, std::min( sizeof( size_t ), m_size ), m_size, os );
        debug( os );
    }

private:
    void debug( std::ostream& os )
    {
        if( m_debug == false )
            return;

        os << "Debug info\n";
        for( auto& alloc : m_Alloces )
        {
            os << std::setw( m_max_w_name + 1 ) << alloc->Name() << '\n';
            alloc->Debug( os );
        }
        os << "\n\n";
    }
    void delete_new( size_t count, size_t sizeData, std::ostream& os )
    {
        os << count << " times 'delete new T'\n";
        Timer timer;
        for( auto& alloc : m_Alloces )
        {
            timer.start();
            for( size_t i = 0; i < count; ++i )
            {
                auto p = alloc->Alloc( sizeData );
                alloc->Dealloc( p );
            }
            timer.stop();
            
            os << std::setw( m_max_w_name + 1 ) << alloc->Name();
            timer.print( os );
            os << '\n';
            CheckMemory( alloc );
        }
        os << "\n\n";
    }
    void newnew_deletedelete( size_t count, size_t sizeData, std::ostream& os )
    {
        os << "i = " << count << " times 1.'arr[i] = new [size]' 2.'delete arr[i]'\n";
        std::vector< void* > pointers( count, nullptr );
        Timer timer;

        for( auto& alloc : m_Alloces )
        {
            size_t failAllocation = 0;
            size_t countAllocation = 0;
            timer.start();
            for( size_t i = 0; i < count; ++i )
            {
                auto p = alloc->Alloc( sizeData );
                if( p == nullptr )
                {
                    failAllocation++;
                }
                else
                {
                    countAllocation++;
                    pointers[i] = p;
                }
            }
            for( size_t i = 0; i < count; ++i )
            {
                alloc->Dealloc( pointers[i] );
                pointers[i] = nullptr;
            }

            timer.stop();

            os << std::setw( m_max_w_name + 1 ) << alloc->Name();
            timer.print( os );
            os << "\tFail allocation: " << failAllocation << " from " << countAllocation << " - " << double( countAllocation - failAllocation ) / countAllocation * 100.0 << "%\n";
            CheckMemory( alloc );
        }
        os << "\n\n";
    }
    void random( size_t countIteration, size_t minSize, size_t maxSize, std::ostream& os )
    {
        os << "i = " << countIteration << " times. RANDOM() ? 'arr[i] = new [RANDOM(" << minSize << "-" << maxSize << ")]' : 'delete arr[i]'\n";
        Timer timer;

        std::vector< void* > pointers;
        pointers.reserve( countIteration );

        for( auto& alloc : m_Alloces )
        {
            pointers.clear();

            size_t failAllocation = 0;
            size_t countAllocation = 0;
            std::mt19937_64 mt64_free_or_malloc( 100 );
            std::mt19937_64 mt64_allocate;
            std::uniform_int<int> free_or_malloc{ 0,1 };
            std::uniform_int<size_t> allocate{ minSize, maxSize };

            timer.start();
            for( size_t i = 0; i < countIteration; ++i )
            {
                if( pointers.empty() || free_or_malloc( mt64_free_or_malloc ) > 0 )
                {
                    size_t s = allocate( mt64_allocate );
                    ++countAllocation;
                    auto p = alloc->Alloc( s );
                    if( p )
                    {
                        pointers.push_back( p );
                        memset( p, 0, s );
                    }
                    else
                    {
                        ++failAllocation;
                    }
                }
                else
                {
                    std::uniform_int<int> pos{ 0, (int)pointers.size() - 1 };
                    size_t posDel = pos( mt64_allocate );
                    alloc->Dealloc( pointers[posDel] );
                    pointers.erase( pointers.begin() + posDel );
                }
            }
            for( auto& p : pointers )
                alloc->Dealloc( p );

            timer.stop();
            os << std::setw( m_max_w_name + 1 ) << alloc->Name();
            timer.print( os );
            os << "\tFail allocation: " << failAllocation << " from " << countAllocation << " - " << double( countAllocation - failAllocation ) / countAllocation * 100.0 << "%\n";
            CheckMemory( alloc );
        }
        os << "\n\n";
    }

    void random_packet( size_t minSizePacket, size_t maxSizePacket, size_t countIteration, size_t minSize, size_t maxSize, std::ostream& os )
    {
        os << "i = " << countIteration << " times. RANDOM() ? 'arr[" 
            << minSizePacket << "-" << maxSizePacket << "] = new [RANDOM(" << minSize << "-" << maxSize << ")]' : 'delete arr[" 
            << minSizePacket << "-" << maxSizePacket << "]'\n";
        Timer timer;

        std::vector< void* > pointers;
        pointers.reserve( countIteration );

        for( auto& alloc : m_Alloces )
        {
            pointers.clear();

            size_t failAllocation = 0;
            size_t countAllocation = 0;
            std::mt19937_64 mt64_free_or_malloc( 100 );
            std::mt19937_64 mt64_allocate;
            std::mt19937_64 mt64_packet( 500 );

            std::uniform_int<int> free_or_malloc{ 0,1 };
            std::uniform_int<size_t> allocate{ minSize, maxSize };
            std::uniform_int<size_t> packet{ 5, maxSizePacket };
            timer.start();
            for( size_t i = 0; i < countIteration; ++i )
            {
                if( pointers.empty() || free_or_malloc( mt64_free_or_malloc ) > 0 )
                {
                    size_t sp = packet( mt64_packet );
                    for( size_t j = 0; j < sp; ++j )
                    {
                        size_t s = allocate( mt64_allocate );
                        ++countAllocation;
                        auto p = alloc->Alloc( s );
                        if( p )
                        {
                            pointers.push_back( p );
                            memset( p, 0, s );
                        }
                        else
                            ++failAllocation;
                    }
                }
                else
                {
                    size_t countDel = ( std::min )( packet( mt64_packet ), pointers.size() );
                    for( size_t j = 0; j < countDel; ++j )
                    {
                        std::uniform_int<int> pos{ 0, (int)pointers.size() - 1 };
                        size_t posDel = pos( mt64_allocate );
                        alloc->Dealloc( pointers[posDel] );
                        pointers.erase( pointers.begin() + posDel );
                    }
                }
            }
            for( auto& p : pointers )
                alloc->Dealloc( p );

            timer.stop();
            os << std::setw( m_max_w_name + 1 ) << alloc->Name();
            timer.print( os );
            os << "\tFail allocation: " << failAllocation << " from " << countAllocation << " - " << double( countAllocation - failAllocation ) / countAllocation * 100.0 << "%\n";

            CheckMemory( alloc );
        }
        os << "\n\n";
    }

    void CheckMemory( IAllocator* alloc )
    {
        if( alloc->MemoryLeak() != 0 || alloc->CountReference() != 0 )
        {
            std::ostringstream ss;
            ss << alloc->Name() << " have memory leak = " <<
                alloc->MemoryLeak() << " and CountReference = " << alloc->CountReference();
            throw std::runtime_error( ss.str() );
        }
    }

    std::vector< IAllocator* > m_Alloces;
};