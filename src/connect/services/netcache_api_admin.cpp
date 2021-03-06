/*  $Id: netcache_api_admin.cpp 546541 2017-09-19 15:47:16Z sadyrovr $
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
 * Author:  Maxim Didenko, Anatoliy Kuznetsov
 *
 * File Description:
 *   Implementation of net cache client.
 *
 */

#include <ncbi_pch.hpp>

#include "netcache_api_impl.hpp"

#include <connect/services/netcache_admin.hpp>
#include <connect/services/netcache_api_expt.hpp>

BEGIN_NCBI_SCOPE

void SNetCacheAdminImpl::ExecOnAllServers(string cmd)
{
    m_API->AppendClientIPSessionIDHitID(&cmd);
    m_API->m_Service.ExecOnAllServers(cmd);
}

void SNetCacheAdminImpl::PrintCmdOutput(string cmd, CNcbiOstream& output_stream, bool multiline_output)
{
    auto style = multiline_output ? CNetService::eMultilineOutput_NetCacheStyle : CNetService::eSingleLineOutput;
    m_API->AppendClientIPSessionIDHitID(&cmd);
    m_API->m_Service.PrintCmdOutput(cmd, output_stream, style);
}

void CNetCacheAdmin::ShutdownServer(EShutdownOption shutdown_option)
{
    string cmd("SHUTDOWN");

    CRequestContext& req = CDiagContext::GetRequestContext();
    m_Impl->m_API->AppendClientIPSessionID(&cmd, req);

    if (shutdown_option == eDrain)
        cmd += " drain=1";

    m_Impl->m_API->AppendHitID(&cmd, req);
    m_Impl->m_API->m_Service.ExecOnAllServers(cmd);
}

void CNetCacheAdmin::ReloadServerConfig(EReloadConfigOption reload_option)
{
    string cmd("RECONF");

    if (reload_option == eMirrorReload) {
        cmd += " section=mirror";
    }

    m_Impl->ExecOnAllServers(cmd);
}

void CNetCacheAdmin::Purge(const string& cache_name)
{
    string cmd("PURGE \"" + NStr::PrintableString(cache_name));
    cmd += '"';
    m_Impl->ExecOnAllServers(cmd);
}

void CNetCacheAdmin::PrintConfig(CNcbiOstream& output_stream)
{
    m_Impl->PrintCmdOutput("GETCONF", output_stream);
}

void CNetCacheAdmin::PrintStat(CNcbiOstream& output_stream,
    const string& aggregation_period,
    CNetCacheAdmin::EStatPeriodCompleteness period_completeness)
{
    string cmd("GETSTAT");

    if (period_completeness != eReturnCurrentPeriod) {
        cmd += " prev=1 type=\"";
        if (!aggregation_period.empty())
            cmd += NStr::PrintableString(aggregation_period);
        cmd += '"';
    } else if (!aggregation_period.empty()) {
        cmd += " prev=0 type=\"";
        cmd += NStr::PrintableString(aggregation_period);
        cmd += '"';
    }

    m_Impl->PrintCmdOutput(cmd, output_stream);
}

void CNetCacheAdmin::PrintHealth(CNcbiOstream& output_stream)
{
    m_Impl->PrintCmdOutput("HEALTH", output_stream);
}

void CNetCacheAdmin::GetServerVersion(CNcbiOstream& output_stream)
{
    m_Impl->PrintCmdOutput("VERSION", output_stream, false);
}


END_NCBI_SCOPE
