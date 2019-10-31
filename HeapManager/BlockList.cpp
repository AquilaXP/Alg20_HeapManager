#include "BlockList.h"



bool IteratorBlockList::operator!=( const IteratorBlockList& rhs ) const
{
    return !( ( *this ) == rhs );
}

bool IteratorBlockList::operator==( const IteratorBlockList& rhs ) const
{
    return m_ptr == rhs.m_ptr;
}

IteratorBlockList::IteratorBlockList( Block* ptr ) : m_ptr( ptr )
{

}

IteratorBlockList::reference IteratorBlockList::operator*() const
{
    return *m_ptr;
}

IteratorBlockList::pointer IteratorBlockList::operator->() const
{
    return m_ptr;
}

IteratorBlockList IteratorBlockList::operator++( int )
{
    IteratorBlockList tmp = *this;
    ++( *this );
    return tmp;
}

IteratorBlockList& IteratorBlockList::operator++()
{
    m_ptr = m_ptr->next;
    return *this;
}

IteratorBlockList IteratorBlockList::operator--( int )
{
    IteratorBlockList tmp = *this;
    --( *this );
    return tmp;
}

IteratorBlockList& IteratorBlockList::operator--()
{
    m_ptr = m_ptr->prev;
    return *this;
}

BlockList::BlockList( void* memory, size_t size ) : BlockList( makeBlock( memory, size ) )
{

}

BlockList::BlockList( Block* block )
{
    init( block );
}

void BlockList::init( Block* block ) noexcept
{
    assert( block );
    assert( block->getSize() > 0 );

    m_root = block;
    m_begin = m_root;
    m_end = (Block*)( m_root->getEnd() );
    m_count_block = 1;
}

void BlockList::eraseBlock( Block* block ) noexcept
{
    if( m_root == block )
    {
        m_root = block->next;
    }
    if( block->prev )
    {
        block->prev->next = block->next;
    }
    if( block->next )
    {
        block->next->prev = block->prev;
    }

    block->ChangeFree( false );
    block->prev = nullptr;
    block->next = nullptr;
    --m_count_block;
}

void BlockList::insertBlock( Block* block ) noexcept
{
    assert( block->prev == nullptr );
    assert( block->next == nullptr );

    ++m_count_block;
    block->ChangeFree( true );
    if( m_root == nullptr )
    {
        m_root = block;
        return;
    }

    block->next = m_root;
    m_root->prev = block;
    m_root = block;
}

Block* BlockList::getPrevMemory( Block* b ) noexcept
{
    assert( m_begin <= b );
    if( b == m_begin )
        return nullptr;

    auto info = reinterpret_cast<InfoBlock2*>( b ) - 1;
    assert( info->size < getSizeMemory() );
    Block* prevB = reinterpret_cast<Block*>( ( std::byte* )b - info->size );
    assert( prevB->getHeadInfo() == *info );
    return prevB;
}

Block* BlockList::getNextMemory( Block* b ) noexcept
{
    auto nextB = (Block*)b->getEnd();
    assert( nextB <= m_end );
    if( nextB == m_end )
        return nullptr;

    return nextB;
}

BlockList::iterator BlockList::begin() noexcept
{
    return iterator( m_root );
}

BlockList::iterator BlockList::end() noexcept
{
    return iterator( nullptr );
}

size_t BlockList::getSizeMemory() const noexcept
{
    return ( std::byte* )m_end - ( std::byte* )m_begin;
}

size_t BlockList::getCountBlocks() const noexcept
{
    return m_count_block;
}

void BlockList::PrintAllBlock( std::ostream& os )
{
    Block* curr = m_begin;
    while( curr )
    {
        os << curr << " | " << std::setw( 10 ) << curr->getSize()
            << " | " << ( curr->getHeadInfo().free ? 1 : 0 ) << " | " << curr->getPrev() << " | " << curr->getNext()
            << " | " << std::setw( 10 ) << curr->getSize()
            << " | " << ( curr->getTailInfo().free ? 1 : 0 ) << " | " << curr->getEnd() << " |\n";

        curr = getNextMemory( curr );
    }
}
