/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        Sched.h
* Description:  C++ support templates for the SDF scheduler
*
* $Date:        29 July 2021
* $Revision:    V1.10.0
*
* Target Processor: Cortex-M and Cortex-A cores
* -------------------------------------------------------------------- */

/* Copyright 2010-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef _SCHEDGEN_H_
#define _SCHEDGEN_H_

#include <vector>

/* FIFOS */
template<typename T>
class FIFOBase {
public:
    virtual T * pxGetWriteBuffer( int nb ) = 0;
    virtual T * pxGetReadBuffer( int nb ) = 0;
};


template<typename T, int length, int isArray = 0>
class FIFO : public FIFOBase<T>
{
public:
    FIFO( T * buffer,
          int delay = 0 ) : mBuffer( buffer ), readPos( 0 ), writePos( delay )
    {
    }

    FIFO( uint8_t * buffer,
          int delay = 0 ) : mBuffer( ( T * ) buffer ), readPos( 0 ), writePos( delay )
    {
    }

    T * pxGetWriteBuffer( int nb ) override
    {
        if( isArray == 1 )
        {
            return( mBuffer );
        }

        T * ret;

        if( readPos > 0 )
        {
            memcpy( ( void * ) mBuffer, ( void * ) ( mBuffer + readPos ), ( writePos - readPos ) * sizeof( T ) );
            writePos -= readPos;
            readPos = 0;
        }

        ret = mBuffer + writePos;
        writePos += nb;
        return( ret );
    }

    T * pxGetReadBuffer( int nb ) override
    {
        if( isArray == 1 )
        {
            return( mBuffer );
        }

        T * ret = mBuffer + readPos;
        readPos += nb;
        return( ret );
    }

protected:
    T * mBuffer;
    int readPos, writePos;
};

/* GENERIC NODES */

class NodeBase
{
public:
    virtual int run() = 0;
};

template<typename IN, int inputSize, typename OUT, int outputSize>
class GenericNode : public NodeBase
{
public:
    GenericNode( FIFOBase<IN> &src,
                 FIFOBase<OUT> &dst ) : mSrc( src ), mDst( dst )
    {
    }

protected:
    OUT * pxGetWriteBuffer()
    {
        return mDst.pxGetWriteBuffer( outputSize );
    }

    IN * pxGetReadBuffer()
    {
        return mSrc.pxGetReadBuffer( inputSize );
    }

private:
    FIFOBase<IN> &mSrc;
    FIFOBase<OUT> &mDst;
};

template<typename IN, int inputSize, typename OUT1, int output1Size, typename OUT2, int output2Size>
class GenericNode12 : public NodeBase
{
public:
    GenericNode12( FIFOBase<IN> &src,
                   FIFOBase<OUT1> &dst1,
                   FIFOBase<OUT2> &dst2 ) : mSrc( src ),
        mDst1( dst1 ), mDst2( dst2 )
    {
    }

protected:
    OUT1 * pxGetWriteBuffer1()
    {
        return mDst1.pxGetWriteBuffer( output1Size );
    }

    OUT2 * pxGetWriteBuffer2()
    {
        return mDst2.pxGetWriteBuffer( output2Size );
    }

    IN * pxGetReadBuffer()
    {
        return mSrc.pxGetReadBuffer( inputSize );
    }

private:
    FIFOBase<IN> &mSrc;
    FIFOBase<OUT1> &mDst1;
    FIFOBase<OUT2> &mDst2;
};

template<typename IN1, int input1Size, typename IN2, int input2Size, typename OUT, int outputSize>
class GenericNode21 : public NodeBase
{
public:
    GenericNode21( FIFOBase<IN1> &src1,
                   FIFOBase<IN2> &src2,
                   FIFOBase<OUT> &dst ) : mSrc1( src1 ),
        mSrc2( src2 ),
        mDst( dst )
    {
    }

protected:
    OUT * pxGetWriteBuffer()
    {
        return mDst.pxGetWriteBuffer( outputSize );
    }

    IN1 * pxGetReadBuffer1()
    {
        return mSrc1.pxGetReadBuffer( input1Size );
    }

    IN2 * pxGetReadBuffer2()
    {
        return mSrc2.pxGetReadBuffer( input2Size );
    }

private:
    FIFOBase<IN1> &mSrc1;
    FIFOBase<IN2> &mSrc2;
    FIFOBase<OUT> &mDst;
};



template<typename OUT, int outputSize>
class GenericSource : public NodeBase
{
public:
    GenericSource( FIFOBase<OUT> &dst ) : mDst( dst )
    {
    }

protected:
    OUT * pxGetWriteBuffer()
    {
        return mDst.pxGetWriteBuffer( outputSize );
    }

private:
    FIFOBase<OUT> &mDst;
};

template<typename IN, int inputSize>
class GenericSink : public NodeBase
{
public:
    GenericSink( FIFOBase<IN> &src ) : mSrc( src )
    {
    }

protected:
    IN * pxGetReadBuffer()
    {
        return mSrc.pxGetReadBuffer( inputSize );
    }

private:
    FIFOBase<IN> &mSrc;
};


#define REPEAT( N )    for( int i = 0; i < N; i++ )

/* GENERIC APPLICATION NODES */

template<typename IN, int windowSize, int overlap>
class SlidingBuffer : public GenericNode<IN, windowSize - overlap, IN, windowSize>
{
public:
    SlidingBuffer( FIFOBase<IN> &src,
                   FIFOBase<IN> &dst ) : GenericNode<IN, windowSize - overlap, IN, windowSize> ( src, dst )
    {
        static_assert( ( windowSize - overlap ) > 0, "Overlap is too big" );
        memory.resize( overlap );
    }

    int run()
    {
        IN * a = this->pxGetReadBuffer();
        IN * b = this->pxGetWriteBuffer();

        memcpy( ( void * ) b, ( void * ) memory.data(), overlap * sizeof( IN ) );
        memcpy( ( void * ) ( b + overlap ), ( void * ) a, ( windowSize - overlap ) * sizeof( IN ) );
        memcpy( ( void * ) memory.data(), ( void * ) ( b + windowSize - overlap ), overlap * sizeof( IN ) );
        return( 0 );
    }

protected:
    std::vector<IN> memory;
};

template<typename IN, int windowSize, int overlap>
class OverlapAdd : public GenericNode<IN, windowSize, IN, windowSize - overlap>
{
public:
    OverlapAdd( FIFOBase<IN> &src,
                FIFOBase<IN> &dst ) : GenericNode<IN, windowSize, IN, overlap> ( src, dst )
    {
        static_assert( ( windowSize - overlap ) > 0, "Overlap is too big" );
        memory.resize( overlap );
    }

    int run()
    {
        int i;
        IN * a = this->pxGetReadBuffer();
        IN * b = this->pxGetWriteBuffer();

        for( i = 0; i < overlap; i++ )
        {
            memory[ i ] = a[ i ] + memory[ i ];
        }

        if( 2 * overlap - windowSize > 0 )
        {
            memcpy( ( void * ) b, ( void * ) memory.data(), ( windowSize - overlap ) * sizeof( IN ) );

            memmove( memory.data(), memory.data() + windowSize - overlap, ( 2 * overlap - windowSize ) * sizeof( IN ) );
            memcpy( memory.data() + 2 * overlap - windowSize, a + overlap, ( windowSize - overlap ) * sizeof( IN ) );
        }
        else if( 2 * overlap - windowSize < 0 )
        {
            memcpy( ( void * ) b, ( void * ) memory.data(), overlap * sizeof( IN ) );
            memcpy( ( void * ) ( b + overlap ), ( void * ) ( a + overlap ), ( windowSize - 2 * overlap ) * sizeof( IN ) );

            memcpy( ( void * ) memory.data(), ( void * ) ( a + windowSize - overlap ), overlap * sizeof( IN ) );
        }
        else
        {
            memcpy( ( void * ) b, ( void * ) memory.data(), overlap * sizeof( IN ) );

            memcpy( ( void * ) memory.data(), ( void * ) ( a + overlap ), overlap * sizeof( IN ) );
        }

        return( 0 );
    }

protected:
    std::vector<IN> memory;
};

#if !defined( CHECKERROR )
    #define CHECKERROR \
    if( sdfError < 0 ) \
    {                  \
        break;         \
    }

#endif
#endif /* ifndef _SCHEDGEN_H_ */
