#include "Block.h"

bool operator==( const InfoBlock2& lhs, const InfoBlock2& rhs )
{
    return rhs.size == rhs.size && lhs.free == rhs.free;
}

bool operator!=( const InfoBlock2& lhs, const InfoBlock2& rhs )
{
    return !( lhs == rhs );
}

Block* makeBlock( void* ptr, size_t size )
{
    Block* b = (Block*)ptr;

    b->getHeadInfo().size = size;
    b->getHeadInfo().free = true;
    b->getTailInfo() = b->getHeadInfo();
    b->setNext( nullptr );
    b->setPrev( nullptr );

    return b;
}

Block* ptrToBlock( void* ptr )
{
    return (Block*)( ( std::byte* )ptr - sizeof( InfoBlock2 ) );
}

Block* merge( Block* b1, Block* b2 )
{
    assert( b2 > b1 );
    assert( b1->getEnd() == ( std::byte* )b2 );

    b1->increaceAvSize( b2, b2->getEnd() );

    return b1;
}

std::pair<Block*, Block*> split( Block* b, size_t sizeAvFirst )
{
    auto mem = b->decreaceAvSize( sizeAvFirst );
    Block* b2 = makeBlock( mem.first, mem.second );

    return std::make_pair( b, b2 );
}

size_t getSizeHeaderBlock()
{
    return sizeof( InfoBlock2 ) * 2;
}

InfoBlock2& Block::getHeadInfo() noexcept
{
    return info;
}

InfoBlock2& Block::getTailInfo() noexcept
{
    return *reinterpret_cast<InfoBlock2*>( ( ( std::byte* )this + ( info.size - sizeof( InfoBlock2 ) ) ) );
}

size_t Block::available() const noexcept
{
    assert( info.size > sizeof( InfoBlock2 ) * 2 );
    return info.size - sizeof( InfoBlock2 ) * 2;
}

std::byte* Block::getMemory() noexcept
{
    return reinterpret_cast<std::byte*>( &prev );
}

void Block::increaceAvSize( void* begin, void* end ) noexcept
{
    assert( begin == getEnd() );
    assert( begin < end );
    size_t newSize = getSize() + size_t( ( std::byte* )end - ( std::byte* )begin );

    getHeadInfo().size += size_t( ( std::byte* )end - ( std::byte* )begin );
    getTailInfo() = getHeadInfo();
}

std::pair<void*, size_t> Block::decreaceAvSize( size_t newAvailSize ) noexcept
{
    assert( newAvailSize >= sizeof( void* ) * 2 );
    assert( available() > newAvailSize );

    size_t dSize = ( available() - newAvailSize );
    getHeadInfo().size -= ( available() - newAvailSize );
    getTailInfo() = getHeadInfo();

    return std::make_pair( getEnd(), dSize );
}

std::byte* Block::getEnd() noexcept
{
    return ( std::byte* )this + getSize();
}

size_t Block::getSize() const noexcept
{
    return info.size;
}

void Block::ChangeFree( bool free ) noexcept
{
    getHeadInfo().free = free;
    getTailInfo().free = free;
}

bool Block::getFree() const noexcept
{
    return info.free;
}

void Block::setNext( Block* v )
{
    next = v;
}

void Block::setPrev( Block* v )
{
    prev = v;
}

Block* Block::getNext()
{
    return next;
}

Block* Block::getPrev()
{
    return prev;
}
