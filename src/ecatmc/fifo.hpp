/*******************************************************************************
* Description: fifo.hpp
* 	Declares the FIFO class. 
*
* Author:	Jay.Wang
* License:	kabang
* Copyright (c)2016-2016 All rights reserved.
*
* Last change:	2016/9/28 15:43
*******************************************************************************/

#ifndef JAY_FIFO_H
#define JAY_FIFO_H

#include <Windows.h>

#ifndef min
#define min(a,b)  ((a)>(b) ? (b) : (a))            /*  */
#endif

#ifndef roundup_pow_of_two
#define roundup_pow_of_two(n)   \
    (1UL    <<                             \
        (                                  \
            (                              \
            (n) & (1UL << 31) ? 31 :       \
            (n) & (1UL << 30) ? 30 :       \
            (n) & (1UL << 29) ? 29 :       \
            (n) & (1UL << 28) ? 28 :       \
            (n) & (1UL << 27) ? 27 :       \
            (n) & (1UL << 26) ? 26 :       \
            (n) & (1UL << 25) ? 25 :       \
            (n) & (1UL << 24) ? 24 :       \
            (n) & (1UL << 23) ? 23 :       \
            (n) & (1UL << 22) ? 22 :       \
            (n) & (1UL << 21) ? 21 :       \
            (n) & (1UL << 20) ? 20 :       \
            (n) & (1UL << 19) ? 19 :       \
            (n) & (1UL << 18) ? 18 :       \
            (n) & (1UL << 17) ? 17 :       \
            (n) & (1UL << 16) ? 16 :       \
            (n) & (1UL << 15) ? 15 :       \
            (n) & (1UL << 14) ? 14 :       \
            (n) & (1UL << 13) ? 13 :       \
            (n) & (1UL << 12) ? 12 :       \
            (n) & (1UL << 11) ? 11 :       \
            (n) & (1UL << 10) ? 10 :       \
            (n) & (1UL <<  9) ?  9 :       \
            (n) & (1UL <<  8) ?  8 :       \
            (n) & (1UL <<  7) ?  7 :       \
            (n) & (1UL <<  6) ?  6 :       \
            (n) & (1UL <<  5) ?  5 :       \
            (n) & (1UL <<  4) ?  4 :       \
            (n) & (1UL <<  3) ?  3 :       \
            (n) & (1UL <<  2) ?  2 :       \
            (n) & (1UL <<  1) ?  1 :       \
            (n) & (1UL <<  0) ?  0 : -1    \
            ) + 1                          \
        )                                  \
)
#endif

template<typename T>
class Fifo
{
private:
	class Mutex
	{
		::CRITICAL_SECTION cs;
	public:
		Mutex()
		{
			do { InitializeCriticalSection(&cs); } while (0);
		}
		~Mutex()
		{
			do{ DeleteCriticalSection(&cs); } while (0);
		}
		void lock()
		{
			do{ EnterCriticalSection(&cs); } while (0);
		}
		void unlock() 
		{
			do{ LeaveCriticalSection(&cs); } while (0);
		}
	} mutex;
private:
	unsigned int volatile _in_offset;	// data ia added at offset(_in_offset % _size)
	unsigned int volatile _out_offset;   // data is extracted from offset(_out_offset % _size)
	unsigned int volatile _size; // size of the allocated buffer
	void*        volatile _data; // the buffer holding the data

private:

	 /**
	  * Dynamically allocates a new fifo buffer.
	  *
	  * @param [in]	size	the size of the fifo will be allocated, which will be
	  * 					rounded up to a power of 2.
	  *
	  * @return	the size allocated actually.
	  */

	unsigned int allocate(unsigned int size);

public:
	Fifo() :_in_offset(0), _out_offset(0), _size(0), _data(NULL)
	{};
	Fifo(unsigned int size) :_in_offset(0), _out_offset(0), _size(0), _data(NULL)
	{
		this->init(size);
	};
	~Fifo()
	{
		this->mutex.lock();
		if (this->_data != NULL && this->_size != 0)
		{
			std::free(this->_data);
		}
		else{}
		this->mutex.unlock();
	};

	/**
	 * Initialize the fifo to the demanded size.
	 * 
	 * @note    The given parameter size must less than half of the max unsigned int.
	 *			The actual size of the fifo will be rounded up to a power of 2
	 * 			based on the given parameter size. You should check the return number
	 * 			of the function to make sure the initialization is done.
	 *
	 * @param [in]	size	size demanded of the fifo.
	 *
	 * @return	actural size of the fifo.
	 */

	unsigned int init(unsigned int size);

	/**
	 * Free the fifo.
	 *
	 * @return	ture if no error, otherwise an error code.
	 */

	int free();

	/**
	 * Removes the entire fifo content. If failed the fifo will not change.
	 * 
	 * @note    Usage of reset() is dangerous. It should be only called when the
	 * 			fifo is exclusived locked or when it is secured that no other thread
	 * 			is accessing the fifo.
	 *
	 * @return	ture if no error, otherwise returns an error code.
	 */

	int reset();

	inline unsigned int avaiable_size()
	{
		this->mutex.lock();
		unsigned int ret = this->_size - (this->_in_offset - this->_out_offset);
		this->mutex.unlock();
		return ret;
	}

	inline unsigned int size()
	{
		this->mutex.lock();
		unsigned int ret = this->_size;
		this->mutex.unlock();
		return ret;
	}

	inline unsigned int elements_size()
	{
		this->mutex.lock();
		unsigned int ret = (this->_in_offset - this->_out_offset);
		this->mutex.unlock();
		return ret;
	}

	inline int is_empty()
	{
		this->mutex.lock();
		int ret = (this->_in_offset == this->_out_offset);
		this->mutex.unlock();
		return ret;
	}

	inline int is_full()
	{
		return (avaiable_size() == 0);
	}

	/**
	 * Insert elements to the fifo.
	 *
	 * @param [in]	src	source buffer.
	 * @param [in]	len number of elements in the source buffer to be copied into fifo.
	 *
	 * @return	number of elements actually copied into the buffer.
	 */

	unsigned int in(T *src, unsigned int len);

	/**
	 * Get data from the fifo.
	 *
	 * @param [out]	dest	where the elements will be stored.
	 * @param [in] 	len 	number of elements to get.
	 *
	 * @return	number of elements actually get from the fifo.
	 */

	unsigned int out(T *dest, unsigned int len);
	
};

template<typename T>
unsigned int Fifo<T>::allocate(unsigned int _size)
{
	this->free();

	this->mutex.lock();
	if (_size & (_size - 1))
	{
		this->_size = roundup_pow_of_two(_size);
	}
	else
	{
		this->_size = _size;
	}

	if (this->_size >= 2)
	{
		this->_data= malloc(this->_size * sizeof(T));
		if (this->_data == NULL)
		{
			this->_size = 0;
		}
		else{}
	}
	else
	{
		this->_size = 0;
	}
	unsigned int ret = this->_size;
	this->mutex.unlock();
	return ret;
}

template<typename T>
unsigned int Fifo<T>::init(unsigned int _size)
{
	return this->allocate(_size);
}

template<typename T>
int Fifo<T>::free()
{
	this->mutex.lock();
	this->_in_offset = 0;
	this->_out_offset = 0;

	if (this->_size == 0)
	{
		this->_data = NULL;	// ignore wild pointer.
	}
	else
	{
		this->_size = 0;
		if (this->_data != NULL)
		{
			std::free(this->_data);
			this->_data = NULL;
		}
		else
		{
		}

	}
	this->mutex.unlock();
	return true;	// always free the fifo.				
}

template<typename T>
int Fifo<T>::reset()
{
	this->mutex.lock();
	unsigned int in_offset	= this->_in_offset;
	unsigned int out_offset	= this->_out_offset;
	unsigned int size		= this->_size;
	void*		 data		= this->_data;

	this->_in_offset = 0;
	this->_out_offset = 0;
	this->_size = 0;
	this->_data = NULL;
	this->mutex.unlock();

	this->allocate(size);

	this->mutex.lock();
	if (size != this->_size)
	{
		this->_in_offset = in_offset;
		this->_out_offset = out_offset;
		this->_size = size;
		this->_data = data;
		this->mutex.unlock();
		return false;
	}
	else
	{
		if (size == 0)
		{
			// ignore wild pointer.
		}
		else
		{
			if (data != NULL)
			{
				delete data;
			}
			else
			{
			}
		}
		this->mutex.unlock();
		return true;
	}
}

template<typename T>
unsigned int Fifo<T>::in(T *src, unsigned int len)
{
	unsigned int _avaiable_size = this->avaiable_size();
	unsigned int _size = this->size();
	this->mutex.lock();
	unsigned int actual_size_copied = min(_avaiable_size, len);
	unsigned int in_offset_of_fifo = this->_in_offset & (_size - 1);
	unsigned int truncation_offset_of_src = min(actual_size_copied, (_size - in_offset_of_fifo));
	memcpy(((char*)this->_data + in_offset_of_fifo * sizeof(T)), (char*)src, truncation_offset_of_src * sizeof(T));
	memcpy((char*)this->_data, (char*)src + truncation_offset_of_src * sizeof(T), (actual_size_copied - truncation_offset_of_src) * sizeof(T));
	this->_in_offset += actual_size_copied;
	this->mutex.unlock();
	return actual_size_copied;
}

template<typename T>
unsigned int Fifo<T>::out(T *dest, unsigned int len)
{
	unsigned int _elements_size = this->elements_size();
	unsigned int _size = this->size();
	this->mutex.lock();
	unsigned int actual_size_copied = min(_elements_size, len);
	unsigned int out_offset_of_fifo = this->_out_offset & (_size - 1);
	unsigned int truncation_offset_of_dest = min(actual_size_copied, (_size - out_offset_of_fifo));
	memcpy((char*)dest, (char*)this->_data + out_offset_of_fifo * sizeof(T), truncation_offset_of_dest * sizeof(T));
	memcpy((char*)dest + truncation_offset_of_dest*sizeof(T), (char*)this->_data, (actual_size_copied - truncation_offset_of_dest) * sizeof(T));
	this->_out_offset += actual_size_copied;
	this->mutex.unlock();
	return actual_size_copied;
}

#endif