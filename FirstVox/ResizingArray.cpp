#include "ResizingArray.h"

#include <malloc.h>
#include <cstring>

#include "FirstVoxHeader.h"

namespace vox::data
{
    ResizingArray::ResizingArray( int elem_size, int elem_align ):
        buf_( nullptr ),
        end_offset_( 0 ),
        end_capacity_offset_( 0 ),
        elem_size_( elem_size ),
        elem_align_( elem_align )
    {
        constexpr size_t START_SIZE = 8U;
        end_capacity_offset_ = elem_size * START_SIZE;
        buf_ = _aligned_malloc( (size_t)end_capacity_offset_, (size_t)elem_align );
    }

    ResizingArray::~ResizingArray()
    {
        _aligned_free( buf_ );
    }
    void* ResizingArray::GetRawData() const
    {
        return buf_;
    }

    size_t ResizingArray::size() const
    {
        return end_offset_ / elem_size_;
    }

    void ResizingArray::clear()
    {
        end_offset_ = 0;
    }

    void ResizingArray::push_back( void* vertex_data )
    {
        if ( end_offset_ == end_capacity_offset_ )
        {
#if 1
            end_capacity_offset_ *= 2;
            void* new_buf = _aligned_realloc( buf_, end_capacity_offset_, elem_align_ );
            if ( new_buf == nullptr )
            {
                return;
            }
            buf_ = new_buf;
#else
            void* new_buf = _aligned_malloc( end_capacity_offset_ * 2, elem_align_ );
            memcpy( new_buf, buf_, end_capacity_offset_ );
            _aligned_free( buf_ );
            buf_ = new_buf;
            end_capacity_offset_ *= 2;
#endif
        }
        memcpy( (unsigned char*)buf_ + end_offset_, vertex_data, elem_size_ );
        end_offset_ += elem_size_;
    }
}