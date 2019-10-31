#pragma once

#include <iomanip>
#include <iterator>

#include "Block.h"

// Итератор по свободным блокам памяти
class IteratorBlockList
{
public:
    friend class BlockList;

    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = Block;
    using difference_type = ptrdiff_t;
    using pointer = Block*;
    using reference = Block&;

    IteratorBlockList() = default;
    IteratorBlockList( Block* ptr );
    [[nodiscard]] reference operator*() const;
    [[nodiscard]] pointer operator->() const;
    IteratorBlockList& operator++();
    IteratorBlockList operator++( int );
    IteratorBlockList& operator--();
    IteratorBlockList operator--( int );
    [[nodiscard]] bool operator == ( const  IteratorBlockList& rhs ) const;
    [[nodiscard]] bool operator != ( const  IteratorBlockList& rhs ) const;
private:
    Block* m_ptr = nullptr;
};

// Данный класс разбивает память на чанки с использованием класса Block
// Он реализует следующие функционал:
//  получение соседних блоков относительно текущего;
//  удаление/добавления блока в пулл свободной памяти;
//  обход по всем свободным блокам.
class BlockList
{
public:
    using iterator = IteratorBlockList;

    BlockList() = default;
    BlockList( Block* block );
    BlockList( void* memory, size_t size );
    void init( Block* block ) noexcept;
    void eraseBlock( Block* block ) noexcept;
    void insertBlock( Block* block ) noexcept;
    [[nodiscard]] Block* getPrevMemory( Block* b ) noexcept;
    [[nodiscard]] Block* getNextMemory( Block* b ) noexcept;
    [[nodiscard]] iterator begin() noexcept;
    [[nodiscard]] iterator end() noexcept;
    [[nodiscard]] size_t getSizeMemory() const noexcept;
    [[nodiscard]] size_t getCountBlocks() const noexcept;
    void PrintAllBlock( std::ostream& os );
private:
    size_t m_count_block = 0;
    size_t m_size = 0;
    Block* m_begin = nullptr;
    Block* m_end = nullptr;
    Block* m_root = nullptr;
};