#pragma once

#include <vector>
#include <cstddef>

#include "TestMacros.h"
#include "BlockList.h"

class TestBlockList
{
public:
    void Execute()
    {
        TestInit();
        TestEraseAndInsertBlock();
        TestPrevAndNextBlock();
    }

private:
    void TestInit()
    {
        std::vector< std::byte > memory( size_t( 800 ) );
        Block* b = makeBlock( memory.data(), memory.size() );

        BlockList list;
        list.init( b );

        T_COMPARE( list.getSizeMemory(), memory.size() );
    }
    void TestEraseAndInsertBlock()
    {
        std::vector< std::byte > memory( size_t( 800 ) );
        Block* b = makeBlock( memory.data(), memory.size() );

        BlockList list;
        list.init( b );
        list.eraseBlock( b );
        T_COMPARE( list.getCountBlocks(), 0 );
        T_COMPARE( b->getNext(), nullptr );
        T_COMPARE( b->getPrev(), nullptr );
        T_COMPARE( b->getFree(), false );

        auto[b1, b2] = split( b, memory.size() / 2 - getSizeHeaderBlock() );
        list.insertBlock( b2 );
        list.insertBlock( b1 );

        T_COMPARE( list.getCountBlocks(), 2 );
        for( auto& v : list )
        {
            T_COMPARE( v.available(), memory.size() / 2 - getSizeHeaderBlock() );
            T_COMPARE( v.getFree(), true );
        }
        list.eraseBlock( b1 );
        T_COMPARE( list.getCountBlocks(), 1 );
        T_COMPARE( b1->getNext(), nullptr );
        T_COMPARE( b1->getPrev(), nullptr );
        T_COMPARE( b1->getFree(), false );

        b2 = std::addressof( *list.begin() );
        T_COMPARE( b2->getNext(), nullptr );
        T_COMPARE( b2->getPrev(), nullptr );
        T_COMPARE( b2->getFree(), true );

        list.insertBlock( b1 );
        list.eraseBlock( b2 );
        T_COMPARE( b2->getNext(), nullptr );
        T_COMPARE( b2->getPrev(), nullptr );
        T_COMPARE( b2->getFree(), false );
    }
    void TestPrevAndNextBlock()
    {
        std::vector< std::byte > memory( size_t( 800 ) );
        Block* b = makeBlock( memory.data(), memory.size() );
        BlockList list;
        // |  b  |
        list.init( b );
        list.eraseBlock( b );
        
        // | b1 | b2 |
        auto[b1, b2] = split( b, memory.size() / 2 - getSizeHeaderBlock() );
        T_COND( b2 > b1 );
        // | b11 | b12 | b2 |
        auto[b11, b12] = split( b1, memory.size() / 4 - getSizeHeaderBlock() );
        T_COND( b12 > b11 );
        // | b11 | b12 | b21 | b22 |
        auto[b21, b22] = split( b2, memory.size() / 4 - getSizeHeaderBlock() );
        T_COND( b22 > b21 );

        // порядок не имеет значения, т.к. проверяем не его
        list.insertBlock( b22 );
        list.insertBlock( b21 );
        list.insertBlock( b12 );
        list.insertBlock( b11 );

        T_COMPARE( list.getPrevMemory( b11 ), nullptr );
        T_COMPARE( list.getNextMemory( b11 ), b12 );

        T_COMPARE( list.getPrevMemory( b12 ), b11 );
        T_COMPARE( list.getNextMemory( b12 ), b21 );

        T_COMPARE( list.getPrevMemory( b21 ), b12 );
        T_COMPARE( list.getNextMemory( b21 ), b22 );

        T_COMPARE( list.getPrevMemory( b22 ), b21 );
        T_COMPARE( list.getNextMemory( b22 ), nullptr );
    }
};