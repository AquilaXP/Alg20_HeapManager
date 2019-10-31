
template< class Fit /*= FirstFit */>
ManagerMemory<Fit>::ManagerMemory( void* memory, size_t size ) : m_finder( m_list ), m_list( memory, size )
{
    // Необходимо добавить первый в отслеживание для BestFit
    m_finder.add( std::addressof( *m_list.begin() ) );
}

template< class Fit /*= FirstFit */>
void* ManagerMemory<Fit>::Alloc( size_t size )
{
    size = align( size );
    Block* b = find( size );
    if( b == nullptr )
        return nullptr;

    eraseBlock( b );
    // Остаток достаточен для хранения минимального блока(8/16 байт), то разбиваем на 2 
    if( b->available() > size + getSizeHeaderBlock() + align( 1 ) )
    {
        auto[b1, b2] = split( b, size );
        b = b1;
        insertBlock( b2 );
    }

    ++m_allocation;
    return b->getMemory();
}

template< class Fit /*= FirstFit */>
void ManagerMemory<Fit>::Dealloc( void* ptr )
{
    if( ptr == nullptr )
        return;

    Block* b = ptrToBlock( ptr );
    assert( b->getHeadInfo() == b->getTailInfo() && "HEAP CORRUPTION DETECTED!" );

    insertBlock( b );
    --m_allocation;
    defragmentation( b );
}

template< class Fit /*= FirstFit */>
void ManagerMemory<Fit>::Debug( std::ostream& os )
{
    size_t s = 0;
    size_t a = 0;
    for( auto& v : m_list )
    {
        s += v.getSize();
        a += v.available();
    }
    os << "SizeMemory = " << m_list.getSizeMemory() << '\n';
    os << "TotalSizeB = " << s << '\n';
    os << "TotalSizeA = " << a << '\n';
    os << "MemoryLeak - " << m_list.getSizeMemory() - s << '\n';
    os << "Allocation = " << 0 << '\n';
    os << "CountBlock = " <<
        std::count_if( std::begin( m_list ), std::end( m_list ), []( auto )->auto{return true; } )
        << '\n';

    m_list.PrintAllBlock( os );
}

template< class Fit /*= FirstFit */>
size_t ManagerMemory<Fit>::MemoryLeak() noexcept
{
    size_t s = 0;
    for( auto& v : m_list )
    {
        s += v.getSize();
    }

    return m_list.getSizeMemory() - s;
}

template< class Fit /*= FirstFit */>
size_t ManagerMemory<Fit>::CountAllocation() const noexcept
{
    return m_allocation;
}

template< class Fit /*= FirstFit */>
Block* ManagerMemory<Fit>::find( size_t size )
{
    return m_finder.find( size );
}

template< class Fit /*= FirstFit */>
size_t ManagerMemory<Fit>::align( size_t size )
{
    // Отдельная обработка размера 0.
    // Данное значение допустимо и должны вернуть минимальный блок памяти.
    // Чтобы округление прошло корректно, необходимо присвоить 1, иначе из 0 будет 0.
    if( size == 0 )
        size = 1;
    // Округляем до 2 указатлей
    // Связанно с тем что Block имеет 2 указателя на свободные блоки и поэтому не может быть меньше.
    // после аллокации они не используются.
    return ( size + ( sizeof( void* ) * 2 - 1 ) ) & ( ~( sizeof( void* ) * 2 - 1 ) );
}

template< class Fit /*= FirstFit */>
void ManagerMemory<Fit>::defragmentation( Block* b )
{
    // Смотрим предыдущий блок
    if( auto prevB = m_list.getPrevMemory( b ); prevB != nullptr && prevB->getFree() )
    {
        eraseBlock( prevB );
        eraseBlock( b );

        b = merge( prevB, b );
        insertBlock( b );
    }

    // Смотрим следующий блок
    if( auto nextB = m_list.getNextMemory( b ); nextB != nullptr && nextB->getFree() )
    {
        eraseBlock( nextB );
        eraseBlock( b );

        b = merge( b, nextB );
        insertBlock( b );
    }
}

template< class Fit /*= FirstFit */>
void ManagerMemory<Fit>::eraseBlock( Block* b )
{
    m_list.eraseBlock( b );
    // Для BestFit и FirstFit
    // FirstFit должен узнать, когда указатель на текущий стал невалидный
    m_finder.remove( b );
}

template< class Fit /*= FirstFit */>
void ManagerMemory<Fit>::insertBlock( Block* b )
{
    m_list.insertBlock( b );
    // Для BestFit
    m_finder.add( b );
}
