#pragma once

#include <memory>
#include <set>
#include <tuple>

#include "Block.h"
#include "BlockList.h"

//
// Алгоритмы поиска подходящего блока
//

// Строит дерева для быстрого поиска
class BestFit
{
    class LessBlock
    {
    public:
        bool operator()( Block* lhs, Block* rhs ) const
        {
            return std::make_tuple( lhs->getSize(), lhs ) < std::make_tuple( rhs->getSize(), rhs );
        }
    };
public:
    BestFit( BlockList& list )
    {
    }
    Block* find( size_t size )
    {
        Block pattern;
        pattern.info.size = size + getSizeHeaderBlock();
        auto b = m_tree.lower_bound( &pattern );
        if( b == m_tree.end() )
            return nullptr;

        return *b;
    }
    void add( Block* b )
    {
        m_tree.insert( b );
    }
    void remove( Block* b )
    {
        m_tree.erase( b );
    }
private:
    std::set< Block*, LessBlock > m_tree;
};

// Строит дерева для быстрого поиска
class BestFit2
{
    class LessBlock
    {
    public:
        bool operator()( Block* lhs, Block* rhs ) const
        {
            return std::make_tuple( lhs->getSize() ) < std::make_tuple( rhs->getSize() );
        }
    };
public:
    BestFit2( BlockList& list )
    {
    }
    Block* find( size_t size )
    {
        Block pattern;
        pattern.info.size = size + getSizeHeaderBlock();
        auto b = m_tree.lower_bound( &pattern );
        if( b == m_tree.end() )
            return nullptr;
        return *b;
    }
    void add( Block* b )
    {
        m_tree.insert( b );
    }
    void remove( Block* b )
    {
        m_tree.erase( b );
    }
private:
    std::multiset< Block*, LessBlock > m_tree;
};

// Находит первый подходящий
class FirstFit
{
public:
    FirstFit( BlockList& list )
        : m_list( list )
    {
    }
    Block* find( size_t size )
    {
        for( auto& v : m_list )
        {
            if( v.available() >= size )
                return std::addressof( v );
        }

        return nullptr;
    }
    void add( Block* b )
    {
        // unused
    }
    void remove( Block* b )
    {
        // unused
    }
protected:
    BlockList& m_list;
};

class NextFit : public FirstFit
{
public:
    NextFit( BlockList& list )
        : FirstFit( list ), prev(m_list.end())
    {
    }
    Block* find( size_t size )
    {
        if( prev == m_list.end() )
            prev = m_list.begin();

        for( auto curr = prev; curr != m_list.end(); ++curr )
        {
            if( curr->available() >= size )
            {
                Block* ret = std::addressof( *curr );
                prev = ++curr;
                return ret;
            }
        }

        // Ищем в оставшейсей части, которую пропустили
        if( prev != m_list.begin() )
        {
            for( auto curr = m_list.begin(); curr != prev; ++curr )
            {
                if( curr->available() >= size )
                {
                    Block* ret = std::addressof( *curr );
                    prev = ++curr;
                    return ret;
                }
            }
        }

        return nullptr;
    }
    void remove( Block* b )
    {
        if( std::addressof( *prev ) == b )
            prev = m_list.end();
    }
private:
    BlockList::iterator prev;
};