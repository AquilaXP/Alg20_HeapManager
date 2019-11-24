#pragma once

#include "BlockList.h"
#include "AlgFit.h"

// Менеджер памяти хранит в двухсвязанном списке блоки свободной памяти (LIFO-вариант).
// При аллокации ищется блок памяти по выбранному алгоритму (шаблоный параметр Fit, выбор в AlgFit.h).
// Если размер блока слишком большой, разбиваем на 2 блока.
// При деаллакации кладем свободный блок в начало + пытается выполнить слияние с соседними блоками (дефрагментация).
// policy-based design.
template< class Fit = FirstFit >
class ManagerMemory
{
public:
    ManagerMemory( void* memory, size_t size );

    void* Alloc( size_t size );
    void Dealloc( void* ptr );

    void Debug( std::ostream& os );
    size_t MemoryLeak() noexcept;
    size_t CountAllocation() const noexcept;
private:

    Block* find( size_t size );
    size_t align( size_t size );

    void defragmentation( Block* b );

    void eraseBlock( Block* b );
    void insertBlock( Block* b );

    size_t m_allocation = 0;
    BlockList m_list;
    Fit m_finder;
};
#include "ManagerMemory.inl"