#ifndef _BLOCK_H_
#define _BLOCK_H_

#include "SourceFile.h"

struct Block {
    SourceFile const* m_source1;
    SourceFile const* m_source2;
    unsigned m_line1;
    unsigned m_line2;
    unsigned m_count;
};

#endif
