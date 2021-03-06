/*  $Id:
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
 * Authors: Amelia Fong
 *
 */

/** @file blastn_node.hpp
 * blastn node api
 */

#ifndef APP__BLASTN_NODE__HPP
#define APP__BLASTN_NODE__HPP

#include <algo/blast/blastinput/blastn_args.hpp>
#include <algo/blast/api/blast_node.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(blast)

class CBlastnNode : public CBlastNode
{
public :

	CBlastnNode (int check_num, const CNcbiArguments & ncbi_args, const CArgs& args,
			       CBlastAppDiagHandler & bah, string & input,
			       int query_index, int num_queries, CBlastNodeMailbox * mailbox = NULL);
	virtual int GetBlastResults(CNcbiOstream & os);
protected:
   	virtual ~CBlastnNode(void);
   	virtual void* Main(void);
private:
	string m_Input;
	CRef<CBlastnNodeArgs>  m_CmdLineArgs;
};

END_SCOPE(blast)
END_NCBI_SCOPE

#endif /* APP__BLASTN_NODE__HPP */
