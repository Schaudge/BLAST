/*  $Id: ncbi_cache.cpp 560849 2018-03-28 12:46:32Z ivanov $
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
 * Author: Aleksey Grichenko, Denis Vakatov
 *
 * File Description:
 *     Generic cache.
 *
 * ===========================================================================
 */
 
#include <ncbi_pch.hpp>
#include <util/ncbi_cache.hpp>

BEGIN_NCBI_SCOPE


const char* CCacheException::GetErrCodeString(void) const
{
    switch ( GetErrCode() ) {
    case eIndexOverflow:   return "eIndexOverflow";
    case eWeightOverflow:  return "eWeightOverflow";
    case eNotFound:        return "eNotFound";
    case eOtherError:      return "eOtherError";
    default:               return CException::GetErrCodeString();
    }
}


END_NCBI_SCOPE
