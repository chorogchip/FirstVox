#pragma once

namespace vox::data
{

    template<typename Elem, unsigned size>
    class QueueFor1WorkerThread {
    private:
        Elem data[size];
        volatile unsigned front;
        volatile unsigned back;
    public:
        QueueFor1WorkerThread();
        void Push(Elem elem);
        Elem Pop();
        bool IsEmpty();
        bool IsFull();
        unsigned Size();
    };


    template<typename Elem, unsigned size>
    QueueFor1WorkerThread<Elem, size>::QueueFor1WorkerThread()
    {
        front = back = 0;
    }


    template<typename Elem, unsigned size>
    void QueueFor1WorkerThread<Elem, size>::Push(Elem elem)
    {
        const unsigned bk = this->back;
        this->data[bk] = elem;
        this->back = (bk + 1) % size;
    }

    template<typename Elem, unsigned size>
    Elem QueueFor1WorkerThread<Elem, size>::Pop()
    {
        const unsigned fr = this->front;
        const Elem ret = this->data[fr];
        this->front = (fr + 1) % size;
        return ret;
    }

    template<typename Elem, unsigned size>
    bool QueueFor1WorkerThread<Elem, size>::IsEmpty()
    {
        return this->front == this->back;
    }

    template<typename Elem, unsigned size>
    bool QueueFor1WorkerThread<Elem, size>::IsFull()
    {
        return (this->back + 1) % size == this->front;
    }

    template<typename Elem, unsigned size>
    unsigned QueueFor1WorkerThread<Elem, size>::Size()
    {
        const unsigned fr = this->front;
        const unsigned bk = this->back;
        if (fr <= bk) return bk - fr;
        else return size - fr + bk;
    }

}