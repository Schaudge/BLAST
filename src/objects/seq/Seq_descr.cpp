/* $Id: Seq_descr.cpp 448516 2014-10-07 14:40:49Z gotvyans $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using the following specifications:
 *   'seq.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>
#include <objects/seq/Seqdesc.hpp>

// generated includes
#include <objects/seq/Seq_descr.hpp>

#include <corelib/ncbi_param.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CSeq_descr::~CSeq_descr(void)
{
}

NCBI_PARAM_DECL(bool, OBJECTS, SEQ_DESCR_ALLOW_EMPTY);
NCBI_PARAM_DEF_EX(bool, OBJECTS, SEQ_DESCR_ALLOW_EMPTY, false,
                  eParam_NoThread, OBJECTS_SEQ_DESCR_ALLOW_EMPTY);
static CSafeStatic<NCBI_PARAM_TYPE(OBJECTS, SEQ_DESCR_ALLOW_EMPTY)> s_SeqDescrAllowEmpty;

void CSeq_descr::PostRead(void) const
{
    if ( !s_SeqDescrAllowEmpty->Get() && Get().empty() ) {
        NCBI_THROW(CSerialException, eInvalidData,
                   "empty Seq-descr is not allowed");
    }
}

void CSeq_descr::PreWrite(void) const
{
    static NCBI_PARAM_TYPE(OBJECTS, SEQ_DESCR_ALLOW_EMPTY) sx_Value;
    if ( !s_SeqDescrAllowEmpty->Get() && Get().empty() ) {
        NCBI_THROW(CSerialException, eInvalidData,
                   "empty Seq-descr is not allowed");
    }
}

bool CAutoAddDesc::IsNull() const
{
    if (m_desc.IsNull())
        m_desc = LocateDesc(*m_descr, m_which);
    return (m_desc.IsNull());
}

const CSeqdesc& CAutoAddDesc::Get() const
{
    if (m_desc.IsNull())
        m_desc = LocateDesc(*m_descr, m_which);
    return *m_desc;
}

CSeqdesc& CAutoAddDesc::Set(bool skip_lookup)
{
    if (!skip_lookup && m_desc.IsNull())
        m_desc = LocateDesc(*m_descr, m_which);
    if (m_desc.IsNull())
    {
        m_desc.Reset(new CSeqdesc);
        m_descr->Set().push_back(m_desc);
    }
    return *m_desc;
}

// update-date should go only to top-level bioseq-set or bioseq
CRef<CSeqdesc> CAutoAddDesc::LocateDesc(const CSeq_descr& descr, CSeqdesc::E_Choice which)
{
    ITERATE(CSeq_descr::Tdata, it, descr.Get())
    {
        if ((**it).Which() == which)
            return *it;
    }

    return CRef<CSeqdesc>();
}

void CAutoAddDesc::Erase()
{
    if (!IsNull())
      m_descr->Set().remove(CRef<CSeqdesc>(&Set()));
}

bool CAutoAddDesc::EraseDesc(CSeq_descr& descr, CSeqdesc::E_Choice which)
{
    bool erased = false;
    for (CSeq_descr::Tdata::iterator it = descr.Set().begin(); it!= descr.Set().end(); )
    {
        if ((**it).Which() == which)
        {
            erased = true;
            descr.Set().erase(it++);
        }
        else
            ++it;
    }

    return erased;
}

END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 65, chars: 1883, CRC32: ed475d39 */
