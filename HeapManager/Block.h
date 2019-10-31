#pragma once

#include <cstddef>
#include <cassert>

#include <utility>
#include <memory>

// Блок свободной памяти
//
//
//
//
//         ___________________________________________________________________
//        |      |      |      |      |                         |      |      |
//        | size | free | prev | next | ...free memory for user | size | free |
//        |______|______|______|______|_________________________|______|______|
//        | \ _______________________ /                         \ __________ /
//        |             |                                              |
//        |           Block                                       InfoBlock(TailInfo)
//        \ __________ /
//              |
//          InfoBlock(HeadInfo)
//
//  size - размер всего блока
//  free - свободно ли
//  prev - предыдущий свободный блок
//  next - следующий свободный блок
//
struct InfoBlock2
{
    size_t size;
    bool free;
};

bool operator==( const InfoBlock2& lhs, const InfoBlock2& rhs );
bool operator!=( const InfoBlock2& lhs, const InfoBlock2& rhs );

struct Block
{
    InfoBlock2 info;
    Block* prev;
    Block* next;

    // getHeadInfo() - указатель на информацию заложенную в конце блока
    //         ___________________________________________________________________
    //        |      |      |      |      |                         |      |      |
    //        | size | free | prev | next | ...free memory for user | size | free |
    //        |______|______|______|______|_________________________|______|______|
    //        ^
    //        |
    InfoBlock2& getHeadInfo() noexcept;

    // getTailInfo() - указатель на информацию заложенную в конце блока
    //         ___________________________________________________________________
    //        |      |      |      |      |                         |      |      |
    //        | size | free | prev | next | ...free memory for user | size | free |
    //        |______|______|______|______|_________________________|______|______|
    //                                                              ^
    // 
    InfoBlock2& getTailInfo() noexcept;

    // available() - доступный размер
    //         ___________________________________________________________________
    //        |      |      |      |      |                         |      |      |
    //        | size | free | prev | next | ...free memory for user | size | free |
    //        |______|______|______|______|_________________________|______|______|
    //                       \ ____________________________________ /
    //                                                  ^
    //  
    size_t available() const noexcept;

    // getMemory() - указатель на блок памяти
    //         ___________________________________________________________________
    //        |      |      |      |      |                         |      |      |
    //        | size | free | prev | next | ...free memory for user | size | free |
    //        |______|______|______|______|_________________________|______|______|
    //                      ^
    //                      |
    std::byte* getMemory() noexcept;

    void increaceAvSize( void* begin, void* end ) noexcept;
    std::pair<void*, size_t> decreaceAvSize( size_t newAvailSize ) noexcept;

    // getEnd() - указатель за пределы блока
    //         ___________________________________________________________________
    //        |      |      |      |      |                         |      |      |
    //        | size | free | prev | next | ...free memory for user | size | free |
    //        |______|______|______|______|_________________________|______|______|
    //                                                                            ^
    //  
    std::byte* getEnd() noexcept;
    size_t getSize() const noexcept;

    void ChangeFree( bool free ) noexcept;
    bool getFree() const noexcept;

    void setNext( Block* v );
    void setPrev( Block* v );
    Block* getNext();
    Block* getPrev();
};

// Из чистой памяти сделать блок памяти
Block* makeBlock( void* ptr, size_t size );

// Преобразует выделнную память из блока памяти в указатель на начало Block
// Block* b = ...;
// b == ptrToBlock( b->getMemory() )
Block* ptrToBlock( void* ptr );

// Выполянет слияние 2 блоков. Порядок важен, b1 должен идти раньше b2!
// Возращается b1
Block* merge( Block* b1, Block* b2 );

// Уменьшается доступную память блока, возращает 2 блока,
// первый является b с уменьшиным размером памяти до sizeAvFirst.
// должно быть достаточно памяти для нового блока!(sizeof(InfoBlock) + sizeof(Block))
std::pair<Block*, Block*> split( Block* b, size_t sizeAvFirst );

size_t getSizeHeaderBlock();
