#pragma once

#include <vector>
#include <utility>

#include "TestMacros.h"
#include "Block.h"

class TestBlock2
{
public:
    void Execute()
    {
        TestMakeBlock();
        TestSplit();
        TestMerge();
        TestPtrToBlock();
    }
private:
    void TestMakeBlock()
    {
        std::vector< std::byte > memory( size_t( 100 ) );
        Block* b = makeBlock( memory.data(), memory.size() );

        T_COMPARE( b->getSize(), memory.size() );
        T_COMPARE( b->getFree(), true );
        T_COMPARE( b->getHeadInfo(), b->getTailInfo() );
        T_COMPARE( b->getNext(), nullptr );
        T_COMPARE( b->getPrev(), nullptr );
    }
    void TestSplit()
    {
        std::vector< std::byte > memory( size_t( 200 ) );
        Block* b = makeBlock( memory.data(), memory.size() );
        auto end = b->getEnd();
        auto[b1, b2] = split( b, memory.size() / 2 - getSizeHeaderBlock() );
        T_COMPARE( b1->available(), b2->available() );
        T_COMPARE( b1, b );
        T_COMPARE( b1->getEnd(), ( std::byte* )b2 );
        T_COMPARE( b2->getEnd(), end );
        T_COMPARE( b1->getSize(), memory.size() / 2 );
        T_COMPARE( b1->getHeadInfo(), b1->getTailInfo() );
        T_COMPARE( b2->getHeadInfo(), b2->getTailInfo() );
        T_COMPARE( b1->getHeadInfo(), b2->getHeadInfo() );
    }
    void TestMerge()
    {
        std::vector< std::byte > memory( size_t( 200 ) );
        Block* b = makeBlock( memory.data(), memory.size() );
        InfoBlock2 info = b->getHeadInfo();
        auto end = b->getEnd();
        auto[b1, b2] = split( b, memory.size() / 2 - getSizeHeaderBlock() );
        Block* bm = merge( b1, b2 );

        T_COMPARE( bm, b );
        T_COMPARE( bm->getSize(), memory.size() );
        T_COMPARE( bm->getHeadInfo(), bm->getTailInfo() );
        T_COMPARE( bm->getHeadInfo(), info );
        T_COMPARE( bm->getEnd(), end );
    }
    void TestPtrToBlock()
    {
        std::vector< std::byte > memory( size_t( 200 ) );
        Block* b = makeBlock( memory.data(), memory.size() );

        T_COMPARE( ptrToBlock( b->getMemory() ), b );
    }
};
