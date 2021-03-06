#ifndef RANDOM_GEN__HPP
#define RANDOM_GEN__HPP

/*  $Id: random_gen.hpp 626630 2021-03-02 17:34:34Z fukanchi $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Clifford Clausen, Denis Vakatov, Jim Ostell, Jonathan Kans,
 *          Greg Schuler, Eugene Vasilchenko
 * Contact: Clifford Clausen
 *
 * File Description:
 *   Random namber generator.
 */

#include <corelib/ncbistd.hpp>


/** @addtogroup RandomGen
 *
 * @{
 */


BEGIN_NCBI_SCOPE




/////////////////////////////////////////////////////////////////////////////
///  CRandom::
///
/// Wraps a system-dependent random generator (could be slow due to
/// system calls), and implements lagged Fibonacci (LFG) random number
/// generator  with lags 33 and 13, modulus 2^31, and operation '+'
///
/// LFG is a slightly modified version of Nlm_Random() found in the NCBI C
/// toolkit. It generates uniform random numbers between 0 and 2^31 - 1 (inclusive).
/// It can be 100 times faster than system-dependent one.
///
/// More details and literature refs are provided in "random_gen.cpp".


class NCBI_XUTIL_EXPORT CRandom
{
public:
    /// Type of the generated integer value and/or the seed value
    typedef Uint4 TValue;

    /// Random generator to use in the GetRand() functions
    enum EGetRandMethod {
        eGetRand_LFG,  ///< Use lagged Fibonacci (LFG) random number generator
        eGetRand_Sys   ///< Use system-dependent random generator
    };

    /// If "method" is:
    ///  - eGetRand_LFG -- use LFG generator seeded with a hard-coded seed
    ///  - eGetRand_Sys -- use system-dependent generator
    CRandom(EGetRandMethod method = eGetRand_LFG);

    /// Use LFG random generator seeded with "seed"
    CRandom(TValue seed);

    /// Get the next random number in the interval [0..GetMax()] (inclusive)
    /// @sa  EGetRandMethod
    /// @note eGetRand_LFG generator could be 100 times faster
    ///       than eGetRand_Sys one
    TValue GetRand(void);

    /// Get random number in the interval [min_value..max_value] (inclusive)
    /// @sa  EGetRandMethod
    /// @note eGetRand_LFG generator could be 100 times faster
    ///       than eGetRand_Sys one
    TValue GetRand(TValue min_value, TValue max_value); 

    /// Get random Uint8 number
    /// @sa  EGetRandMethod
    /// @note eGetRand_LFG generator could be 100 times faster
    ///       than eGetRand_Sys one
    Uint8 GetRandUint8(void);

    /// Get random number in the interval [min_value..max_value] (inclusive)
    /// @sa  EGetRandMethod
    /// @note eGetRand_LFG generator could be 100 times faster
    ///       than eGetRand_Sys one
    Uint8 GetRandUint8(Uint8 min_value, Uint8 max_value); 

    /// Get random number in the interval [min_value..max_value] (inclusive)
    /// @sa  EGetRandMethod
    /// @note eGetRand_LFG generator could be 100 times faster
    ///       than eGetRand_Sys one
    size_t GetRandSize_t(size_t min_value, size_t max_value); 

    /// Get random number in the interval [0..size-1] (e.g. index in array)
    /// @sa  EGetRandMethod
    /// @note eGetRand_LFG generator could be 100 times faster
    ///       than eGetRand_Sys one
    TValue GetRandIndex(TValue size);

    /// Get random number in the interval [0..size-1] (e.g. index in array)
    /// @sa  EGetRandMethod
    /// @note eGetRand_LFG generator could be 100 times faster
    ///       than eGetRand_Sys one
    Uint8 GetRandIndexUint8(Uint8 size);

    /// Get random number in the interval [0..size-1] (e.g. index in array)
    /// @sa  EGetRandMethod
    /// @note eGetRand_LFG generator could be 100 times faster
    ///       than eGetRand_Sys one
    size_t GetRandIndexSize_t(size_t size);

    /// The max. value GetRand() returns
    static TValue GetMax(void);

    /// Get the random generator type
    EGetRandMethod GetRandMethod(void) const;

    // LFG only:

    /// Re-initialize (re-seed) the generator using platform-specific
    /// randomization.
    /// @note  Does nothing if system generator is used.
    void Randomize(void);

    /// Seed the random number generator with "seed".
    /// @attention  Throw exception if non-LFG (i.e. system) generator is used.
    void SetSeed(TValue seed);

    /// Get the last set seed (LFG only)
    /// @attention  Throw exception if non-LFG (i.e. system) generator is used.
    TValue GetSeed(void) const;

    /// Reset random number generator to initial startup condition (LFG only)
    /// @attention  Throw exception if non-LFG (i.e. system) generator is used.
    void Reset(void);

private:
    enum {
        kStateSize = 33  // size of state array
    };
    EGetRandMethod  m_RandMethod;
    TValue          m_State[kStateSize];
    int             m_RJ;
    int             m_RK;
    TValue          m_Seed;

    TValue x_GetRand32Bits(void);
    Uint8 x_GetRand64Bits(void);
    TValue x_GetSysRand32Bits(void) const;

    // prevent copying
    CRandom(const CRandom&);
    CRandom& operator=(const CRandom&);
};


/// Exceptions generated by CRandom class
class NCBI_XUTIL_EXPORT CRandomException : public CException
{
public:
    enum EErrCode {
        eUnavailable,           ///< System-dependent generator is not available
        eUnexpectedRandMethod,  ///< The user called method which is not
                                ///< allowed for the used generator.
        eSysGeneratorError      ///< Error getting a random value from the
                                ///< system-dependent generator.
    };

    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode()) {
        case eUnavailable           : return "eUnavailable";
        case eUnexpectedRandMethod  : return "eUnexpectedRandMethod";
        case eSysGeneratorError     : return "eSysGeneratorError";
        default                     : return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CRandomException, CException);
};



/* @} */


/////////////////////////////////////////////////////////////////////////////
//  IMPLEMENTATION of INLINE functions
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//  CRandom::
//

inline CRandom::TValue CRandom::x_GetRand32Bits(void)
{
    if (m_RandMethod == eGetRand_Sys)
        return x_GetSysRand32Bits();

    TValue r;

    r = m_State[m_RK] + m_State[m_RJ--];
    m_State[m_RK--] = r;

    if ( m_RK < 0 ) {
        m_RK = kStateSize - 1;
    }
    else if ( m_RJ < 0 ) {
        m_RJ = kStateSize - 1;
    }

    return r;
}


inline CRandom::TValue CRandom::GetRand(void)
{
    return x_GetRand32Bits() >> 1; // discard the least-random bit
}


inline Uint8 CRandom::GetRandUint8(void)
{
    Uint8 v1 = x_GetRand32Bits();
    return (v1 << 32)+x_GetRand32Bits();
}


inline CRandom::TValue CRandom::GetRandIndex(TValue size)
{
    if ( (size & (size-1)) == 0 ) { // only one bit set - power of 2
        // get high bits via multiplication - it's faster than division
        return TValue(Uint8(x_GetRand32Bits())*size >> 32);
    }

    TValue bits, r;
    do {
        bits = x_GetRand32Bits();
        r = bits % size;
    } while ( bits > r - size ); // 32-bit overflow is intentional
    return r;
}


inline size_t CRandom::GetRandIndexSize_t(size_t size)
{
#if SIZEOF_SIZE_T == 4
    return GetRandIndex(Uint4(size));
#else
    return GetRandIndexUint8(size);
#endif
}


inline CRandom::TValue CRandom::GetRand(TValue min_value, TValue max_value)
{
    return min_value + GetRandIndex(max_value - min_value + 1);
}


inline Uint8 CRandom::GetRandUint8(Uint8 min_value, Uint8 max_value)
{
    return min_value + GetRandIndexUint8(max_value - min_value + 1);
}


inline size_t CRandom::GetRandSize_t(size_t min_value, size_t max_value)
{
    return min_value + GetRandIndexSize_t(max_value - min_value + 1);
}


inline CRandom::TValue CRandom::GetMax(void)
{
    return 0x7fffffff;
}


inline CRandom::EGetRandMethod CRandom::GetRandMethod(void) const
{
    return m_RandMethod;
}


END_NCBI_SCOPE

#endif  /* RANDOM_GEN__HPP */
