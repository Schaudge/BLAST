/* $Id: Delta_ext.hpp 437315 2014-06-05 13:07:19Z gotvyans $
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
 */

/// @file Delta_ext.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'seq.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: Delta_ext_.hpp


#ifndef OBJECTS_SEQ_DELTA_EXT_HPP
#define OBJECTS_SEQ_DELTA_EXT_HPP


// generated includes
#include <objects/seq/Delta_ext_.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seqloc/Na_strand.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

class CDelta_seq;
class CSeq_id;

/////////////////////////////////////////////////////////////////////////////
class NCBI_SEQ_EXPORT CDelta_ext : public CDelta_ext_Base
{
    typedef CDelta_ext_Base Tparent;
public:
    // constructor
    CDelta_ext(void);
    // destructor
    ~CDelta_ext(void);

    /// add a literal segment at the end
    /// this variant adds a gap literal
    CDelta_seq& AddLiteral(TSeqPos len);

    /// add a literal segment at the end
    /// this variant adds a non-gap literal
    CDelta_seq& AddLiteral(const CTempString& iupac_seq, CSeq_inst::EMol mol,
        bool do_pack = true);

    /// add a chunk of sequence, splitting it as necessary for the
    /// sake of compactness (isolating ambiguous portions and optionally gaps)
    void AddAndSplit(const CTempString& src, CSeq_data::E_Choice format,
                     TSeqPos length /* in residues */, bool gaps_ok = false,
                     bool allow_packing = true);

    /// add a segment that refers to another segment
    CDelta_seq& AddSeqRange(const CSeq_id& id, TSeqPos from, TSeqPos to,
                            ENa_strand strand = eNa_strand_plus);

private:
    // Prohibit copy constructor and assignment operator
    CDelta_ext(const CDelta_ext& value);
    CDelta_ext& operator=(const CDelta_ext& value);

};

/////////////////// CDelta_ext inline methods

// constructor
inline
CDelta_ext::CDelta_ext(void)
{
}


/////////////////// end of CDelta_ext inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

#endif // OBJECTS_SEQ_DELTA_EXT_HPP
/* Original file checksum: lines: 94, chars: 2559, CRC32: 732ab450 */
