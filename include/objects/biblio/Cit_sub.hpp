/* $Id: Cit_sub.hpp 642716 2021-12-27 19:04:27Z fukanchi $
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
 *   using specifications from the data definition file
 *   'biblio.asn'.
 */

#ifndef OBJECTS_BIBLIO_CIT_SUB_HPP
#define OBJECTS_BIBLIO_CIT_SUB_HPP


// generated includes
#include <objects/biblio/Cit_sub_.hpp>

#include <objects/biblio/citation_base.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

class NCBI_BIBLIO_EXPORT CCit_sub : public CCit_sub_Base, public ICitationBase
{
    typedef CCit_sub_Base Tparent;
public:
    // constructor
    CCit_sub(void);
    // destructor
    ~CCit_sub(void);

protected:
    // Appends a label onto "label" based on content
    bool GetLabelV1(string* label, TLabelFlags flags) const override;
    bool GetLabelV2(string* label, TLabelFlags flags) const override;

private:
    // Prohibit copy constructor and assignment operator
    CCit_sub(const CCit_sub& value);
    CCit_sub& operator=(const CCit_sub& value);

};



/////////////////// CCit_sub inline methods

// constructor
inline
CCit_sub::CCit_sub(void)
{
}


/////////////////// end of CCit_sub inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

#endif // OBJECTS_BIBLIO_CIT_SUB_HPP
/* Original file checksum: lines: 93, chars: 2380, CRC32: 7fc8278d */
