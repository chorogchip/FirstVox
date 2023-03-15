#pragma once

namespace vox::data
{
    class ResizingArray
    {
    private:
        void* buf_;
        int end_offset_;
        int end_capacity_offset_;
        const int elem_size_;
        const int elem_align_;
    public:
        ResizingArray( int elem_size, int elem_align );
        ~ResizingArray();
        void* GetRawData() const;
        size_t size() const;
        const void* operator[]( int index ) const
        {
            return (unsigned char*)buf_ + index * elem_size_;
        }
        void clear();
        void push_back( void* vertex_data );
    };
}