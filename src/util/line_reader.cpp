/*  $Id: line_reader.cpp 637545 2021-09-14 15:28:34Z ucko $
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
* Author:  Aaron Ucko, Anatoliy Kuznetsov
*
* File Description:
*   Lightweight interface for getting lines of data with minimal
*   memory copying.
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>
#include <util/line_reader.hpp>
#include <util/util_exception.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/stream_utils.hpp>
#include <util/error_codes.hpp>

#include <string.h>

#define NCBI_USE_ERRCODE_X   Util_LineReader

BEGIN_NCBI_SCOPE


CRef<ILineReader> ILineReader::New(const string& filename)
{
    CRef<ILineReader> lr;
    if (filename != "-") {
        try {
            lr.Reset(new CMemoryLineReader(new CMemoryFile(filename),
                                           eTakeOwnership));
        } catch (exception& e) { // CFileException is the main concern
            ERR_POST_X(1, Info << "ILineReader::New: falling back from"
                       " CMemoryLineReader to CBufferedLineReader for "
                       << filename << " due to exception: " << e.what());
        }
    }
    if (lr.Empty()) {
        lr.Reset(new CBufferedLineReader(filename));
    }
    return lr;
}


CRef<ILineReader> ILineReader::New(CNcbiIstream& is, EOwnership ownership)
{
    CRef<ILineReader> lr;
    lr.Reset(new CBufferedLineReader(is, ownership));
    return lr;
}


CStreamLineReader::CStreamLineReader(CNcbiIstream& is,
                                     EEOLStyle eol_style,
                                     EOwnership ownership)
    : m_Stream(&is, ownership), m_LineNumber(0), m_LastReadSize(0),
      m_UngetLine(false), m_AutoEOL(eol_style == eEOL_unknown),
      m_EOLStyle(eol_style)
{
}


CStreamLineReader::CStreamLineReader(CNcbiIstream& is,
                                     EOwnership ownership)
    : m_Stream(&is, ownership), m_LineNumber(0), m_LastReadSize(0),
      m_UngetLine(false), m_AutoEOL(true), m_EOLStyle(eEOL_unknown)
{
}


CStreamLineReader::~CStreamLineReader()
{
}


bool CStreamLineReader::AtEOF(void) const
{
    return !m_UngetLine &&
        (m_Stream->eof()  ||  CT_EQ_INT_TYPE(m_Stream->peek(), CT_EOF));
}


char CStreamLineReader::PeekChar(void) const
{
    _ASSERT(!AtEOF());
    /* If at EOF - undefined behavior, return m_Stream->peek() */
    if (AtEOF()) {
        return (char)m_Stream->peek();
    }
    /* If right after constructor (line number is 0 and line was not ungot) - 
       return the first character of the first line */
    if (m_LineNumber == 0 && !m_UngetLine) {
        char c = (char)m_Stream->peek();
        /* If there are delimiters right from the start (which means the
        first line is empty), return 0 */
        if (c == '\n' || c == '\r') {
            return 0;
        }
        return c;
    }
    /* If line was ungot - return its first symbol */
    if (m_UngetLine) {
        /* if line is empty - return 0 */
        if (m_Line.empty()) {
            return 0;
        }
        return *m_Line.begin();
    }
    char c = (char)m_Stream->peek();
    /* If line is empty - return 0 */
    if (c == '\n' || c == '\r') {
        return 0;
    }
    return c;
}


void CStreamLineReader::UngetLine(void)
{
    _ASSERT(!m_UngetLine && m_LineNumber != 0);
    /* If after UngetLine() or after constructor - noop */
    if (m_UngetLine || m_LineNumber == 0) {
        return;
    }
    --m_LineNumber;
    m_UngetLine = true;
}


CStreamLineReader& CStreamLineReader::operator++(void)
{
    /* If at EOF - noop */
    if (AtEOF()) {
        m_Line = string();
        return *this;
    }
    ++m_LineNumber;
    if ( m_UngetLine ) {
        m_UngetLine = false;
        return *this;
    }

    switch (m_EOLStyle) {
    case eEOL_unknown: x_AdvanceEOLUnknown();                   break;
    case eEOL_cr:      x_AdvanceEOLSimple('\r', '\n');          break;
    case eEOL_lf:      x_AdvanceEOLSimple('\n', '\r');          break;
    case eEOL_crlf:    x_AdvanceEOLCRLF();                      break;
    case eEOL_mixed:   NcbiGetline(*m_Stream, m_Line, "\r\n");  break;
    }
    return *this;
}


CTempString CStreamLineReader::operator*(void) const
{
    _ASSERT(!m_UngetLine);
    /* After UngetLine() - undefined behavior */
    if (m_UngetLine) {
        return CTempString(NULL);
    }
    /* If at EOF - returns NULL */
//    if (AtEOF()) {
//        return CTempString(NULL);
//    }
    /* Right after constructor (m_LineNumber is 0 and UngetLine() was not run, 
       the latter was already checked) - returns NULL */
    if (m_LineNumber == 0) {
        return CTempString(NULL);
    }
    return CTempString(m_Line);
}


CT_POS_TYPE CStreamLineReader::GetPosition(void) const
{
    CT_POS_TYPE pos = m_Stream->tellg();
    if (m_UngetLine) {
        pos -= m_LastReadSize;
    }
    return pos;
}


Uint8 CStreamLineReader::GetLineNumber(void) const
{
    /* Right after constructor (m_LineNumber is 0 and UngetLine() was not run) 
       - 0 */
    /* If at EOF - returns the number of the last string */
    /* After UngetLine() - number of the previous string */
    /* Not at EOF, not after UngetLine() - number of the current string */
    return m_LineNumber;
}


CStreamLineReader::EEOLStyle CStreamLineReader::x_AdvanceEOLUnknown(void)
{
    _ASSERT(m_AutoEOL);
    NcbiGetline(*m_Stream, m_Line, "\r\n", &m_LastReadSize);
    m_Stream->unget();
    CT_INT_TYPE eol = m_Stream->get();
    if (CT_EQ_INT_TYPE(eol, CT_TO_INT_TYPE('\r'))) {
        m_EOLStyle = eEOL_cr;
    } else if (CT_EQ_INT_TYPE(eol, CT_TO_INT_TYPE('\n'))) {
        // NcbiGetline doesn't yield enough information to determine
        // whether eEOL_lf or eEOL_crlf is more appropriate, and not
        // all streams allow tellg() (which could otherwise resolve
        // matters), so defer further analysis to x_AdvanceEOLCRLF,
        // which will be responsible for reading the next line and
        // supports switching to eEOL_lf as appropriate.
        //
        // An alternative approach would have been to pass \n\r rather
        // than \r\n, and then check for an immediately following \n
        // if eol turned out to be \r, but that would miscount an
        // actual(!) \n\r sequence as a single line break.
        m_EOLStyle = eEOL_crlf;
    }
    return m_EOLStyle;
}


CStreamLineReader::EEOLStyle CStreamLineReader::x_AdvanceEOLSimple(char eol,
                                                                   char alt_eol)
{
    SIZE_TYPE pos;
    NcbiGetline(*m_Stream, m_Line, eol, &m_LastReadSize);
    if (m_AutoEOL  &&  (pos = m_Line.find(alt_eol)) != NPOS) {
        ++pos;
        if (eol != '\n'  ||  pos != m_Line.size()) {
            // an *immediately* preceding CR is quite all right
            CStreamUtils::Pushback(*m_Stream, m_Line.data() + pos,
                                   m_Line.size() - pos);
            m_EOLStyle = eEOL_mixed;
        }
        m_Line.resize(pos - 1);
        m_LastReadSize = pos;
        return (m_EOLStyle == eEOL_mixed) ? m_EOLStyle : eEOL_crlf;
    } else if (m_AutoEOL  &&  eol == '\r'  &&
               CT_EQ_INT_TYPE(m_Stream->peek(), CT_TO_INT_TYPE(alt_eol))) {
        m_Stream->get();
        ++m_LastReadSize;
        return eEOL_crlf;
    }
    return (eol == '\r') ? eEOL_cr : eEOL_lf;
}


CStreamLineReader::EEOLStyle CStreamLineReader::x_AdvanceEOLCRLF(void)
{
    if (m_AutoEOL) {
        EEOLStyle style = x_AdvanceEOLSimple('\n', '\r');
        if (style == eEOL_mixed) {
            // found an embedded CR
            m_EOLStyle = eEOL_cr;
        } else if (style != eEOL_crlf) {
            m_EOLStyle = eEOL_lf;
        }
    } else {
        string extra;
        NcbiGetline(*m_Stream, m_Line, '\n', &m_LastReadSize);
        while ( !AtEOF()  &&  !NStr::EndsWith(m_Line, "\r") ) {
            SIZE_TYPE extra_count;
            m_Line += '\n';
            NcbiGetline(*m_Stream, extra, '\n', &extra_count);
            m_Line += extra;
            m_LastReadSize += extra_count + 1;
        }
        if (NStr::EndsWith(m_Line, "\r")) {
            m_Line.resize(m_Line.size() - 1);
        }
    }
    return m_EOLStyle;
}


CMemoryLineReader::CMemoryLineReader(CMemoryFile* mem_file,
                                     EOwnership ownership)
    : m_Start(static_cast<char*>(mem_file->GetPtr())),
      m_End(m_Start + mem_file->GetSize()),
      m_Pos(m_Start),
      m_MemFile(mem_file, ownership),
      m_LineNumber(0)
{
    m_MemFile->MemMapAdvise(CMemoryFile::eMMA_Sequential);
}


bool CMemoryLineReader::AtEOF(void) const
{
    return m_Pos >= m_End;
}


char CMemoryLineReader::PeekChar(void) const
{
    _ASSERT(!AtEOF());
    /* If at EOF - undefined behavior, return m_Pos */
    if (AtEOF()) {
        return *m_Pos;
    }
    /* If line was ungot - return its first symbol */
    if (m_Pos == m_Line.begin()) {
        /* if line is empty - return 0 */
        if (m_Line.empty()) {
            return 0;
        }
        return *m_Pos;
    }
    /* If line is empty - return 0 */
    if (*m_Pos == '\n' || *m_Pos == '\r') {
        return 0;
    }
    return *m_Pos;
}


void CMemoryLineReader::UngetLine(void)
{
    _ASSERT(m_Pos != m_Line.begin());
    _ASSERT(m_Line.begin() != 0);
    /* If after UngetLine() or after constructor - noop */
    if (m_Pos == m_Line.begin() || m_Line.begin() == 0) {
        return;
    }
    --m_LineNumber;
    m_Pos = m_Line.begin();
}


CMemoryLineReader& CMemoryLineReader::operator++(void)
{
    /* If at EOF - noop */
    if (AtEOF()) {
        m_Line = CTempString(NULL);
        return *this;
    }
    const char* p = m_Pos;
    if ( p == m_Line.begin() ) {
        /* If after UngetLine(), line is already in buffer, so end is known*/
        p = m_Line.end();
    } else {
        /* Line is in stream, go char by char until delimiters */
        while ( p < m_End  &&  *p != '\r'  && *p != '\n' ) {
            ++p;
        }
        m_Line = CTempString(m_Pos, p - m_Pos);
    }
    // skip over delimiters until the beginning of the next string
    if (p + 1 < m_End  &&  *p == '\r'  &&  p[1] == '\n') {
        m_Pos = p + 2;
    } else if (p < m_End) {
        m_Pos = p + 1;
    } else { // no final line break
        m_Pos = p;
    }
    ++m_LineNumber;
    return *this;
}


CTempString CMemoryLineReader::operator*(void) const
{
    _ASSERT(m_Pos != m_Line.begin());
    /* After UngetLine() - undefined behavior */
    if (m_Pos == m_Line.begin()) {
        return CTempString(NULL);
    }
    /* Right after constructor (m_LineNumber is 0 and UngetLine() was not run,
    the latter was already checked) - returns NULL */
    if (m_LineNumber == 0) {
        return CTempString(NULL);
    }
    return m_Line;
}


CT_POS_TYPE CMemoryLineReader::GetPosition(void) const
{
    return NcbiInt8ToStreampos(m_Pos - m_Start);
}


Uint8 CMemoryLineReader::GetLineNumber(void) const
{
    /* Right after constructor (m_LineNumber is 0 and UngetLine() was not run)
    - 0 */
    /* If at EOF - returns the number of the last string */
    /* After UngetLine() - number of the previous string */
    /* Not at EOF, not after UngetLine() - number of the current string */
    return m_LineNumber;
}


CBufferedLineReader::CBufferedLineReader(IReader* reader,
                                         EOwnership ownership)
    : m_Reader(reader, ownership),
      m_Eof(false),
      m_UngetLine(false),
      m_BufferSize(32*1024),
      m_Buffer(new char[m_BufferSize]),
      m_Pos(m_Buffer.get()),
      m_End(m_Pos),
      m_InputPos(0),
      m_LineNumber(0)
{
    x_ReadBuffer();
}


CBufferedLineReader::CBufferedLineReader(CNcbiIstream& is,
                                         EOwnership ownership)
    : m_Reader(new CStreamReader(is, ownership)),
      m_Eof(false),
      m_UngetLine(false),
      m_BufferSize(32*1024),
      m_Buffer(new char[m_BufferSize]),
      m_Pos(m_Buffer.get()),
      m_End(m_Pos),
      m_InputPos(0),
      m_LineNumber(0)
{
    x_ReadBuffer();
}


CBufferedLineReader::CBufferedLineReader(const string& filename)
    : m_Reader(CFileReader::New(filename)),
      m_Eof(false),
      m_UngetLine(false),
      m_BufferSize(32*1024),
      m_Buffer(new char[m_BufferSize]),
      m_Pos(m_Buffer.get()),
      m_End(m_Pos),
      m_InputPos(0),
      m_LineNumber(0)
{
    x_ReadBuffer();
}


CBufferedLineReader::~CBufferedLineReader()
{
}


bool CBufferedLineReader::AtEOF(void) const
{
    return m_Eof && !m_UngetLine;
}


char CBufferedLineReader::PeekChar(void) const
{
    _ASSERT(!AtEOF());
    /* If at EOF - undefined behavior */
    if (AtEOF()) {
        return *m_Pos;
    }
    /* If line was ungot - return its first symbol */
    if (m_UngetLine) {
        /* if line is empty - return 0 */
        if (m_Line.empty()) {
            return 0;
        }
        return *m_Line.begin();
    }
    /* If line is empty - return 0 */
    if (*m_Pos == '\n' || *m_Pos == '\r') {
        return 0;
    }
    return *m_Pos;
}


void CBufferedLineReader::UngetLine(void)
{
    _ASSERT(!m_UngetLine && m_Line.begin());
    /* If after UngetLine() or after constructor - noop */
    if (m_UngetLine || m_Line.begin() == NULL)
        return;
    --m_LineNumber;
    m_UngetLine = true;
}


CBufferedLineReader& CBufferedLineReader::operator++(void)
{
    /* If at EOF - noop */
    if (AtEOF()) {
        m_Line = CTempString(NULL);
        return *this;
    }
    ++m_LineNumber;
    if ( m_UngetLine ) {
        _ASSERT(m_Line.begin());
        m_UngetLine = false;
        return *this;
    }
    // check if we are at the buffer end
    const char* start = m_Pos;
    const char* end = m_End;
    for ( const char* p = start; p < end; ++p ) {
        if ( *p == '\n' ) {
            m_Line = CTempString(start, p - start);
            m_LastReadSize = p + 1 - start;
            m_Pos = ++p;
            if ( p == end ) {
                m_String = m_Line;
                m_Line = m_String;
                x_ReadBuffer();
            }
            return *this;
        }
        else if ( *p == '\r' ) {
            m_Line = CTempString(start, p - start);
            m_LastReadSize = p + 1 - start;
            m_Pos = ++p;
            if ( p == end ) {
                m_String = m_Line;
                m_Line = m_String;
                if ( x_ReadBuffer() ) {
                    p = m_Pos;
                    if ( *p == '\n' ) {
                        m_Pos = p+1;
                        ++m_LastReadSize;
                    }
                }
                return *this;
            }
            if ( *p != '\n' ) {
                return *this;
            }
            ++m_LastReadSize;
            m_Pos = ++p;
            if ( p == end ) {
                m_String = m_Line;
                m_Line = m_String;
                x_ReadBuffer();
            }
            return *this;
        }
    }
    x_LoadLong();
    return *this;
}


void CBufferedLineReader::x_LoadLong(void)
{
    const char* start = m_Pos;
    const char* end = m_End;
    m_String.assign(start, end);
    while ( x_ReadBuffer() ) {
        start = m_Pos;
        end = m_End;
        for ( const char* p = start; p < end; ++p ) {
            char c = *p;
            if ( c == '\r' || c == '\n' ) {
                m_String.append(start, p - start);
                m_Line = m_String;
                m_LastReadSize = m_Line.size() + 1;
                if ( ++p == end ) {
                    m_String = m_Line;
                    m_Line = m_String;
                    if ( x_ReadBuffer() ) {
                        p = m_Pos;
                        end = m_End;
                        if ( p < end && c == '\r' && *p == '\n' ) {
                            ++p;
                            m_Pos = p;
                            ++m_LastReadSize;
                        }
                    }
                }
                else {
                    if ( c == '\r' && *p == '\n' ) {
                        if ( ++p == end ) {
                            x_ReadBuffer();
                            p = m_Pos;
                            ++m_LastReadSize;
                        }
                    }
                    m_Pos = p;
                }
                return;
            }
        }
        m_String.append(start, end - start);
    }
    m_Line = m_String;
    m_LastReadSize = m_Line.size();
    return;
}


bool CBufferedLineReader::x_ReadBuffer()
{
    _ASSERT(m_Reader);

    if ( m_Eof ) {
        return false;
    }

    m_InputPos += CT_OFF_TYPE(m_End - m_Buffer.get());
    m_Pos = m_End = m_Buffer.get();
    for (bool flag = true; flag; ) {
        size_t size;
        ERW_Result result =
            m_Reader->Read(m_Buffer.get(), m_BufferSize, &size);
        switch (result) {
        case eRW_NotImplemented:
        case eRW_Error:
            NCBI_THROW(CIOException, eRead, "Read error");
            /*NOTREACHED*/
            break;
        case eRW_Timeout:
            // keep spinning around
            break;
        case eRW_Eof:
            m_Eof = true;
            // fall through
        case eRW_Success:
            m_End = m_Pos + size;
            return (result == eRW_Success  ||  size > 0);
        default:
            _ASSERT(0);
        }
    } // for
    return false;
}


CTempString CBufferedLineReader::operator*(void) const
{
    _ASSERT(!m_UngetLine);
    /* After UngetLine() - undefined behavior */
    if (m_UngetLine) {
        return CTempString(NULL);
    }
    /* Right after constructor (m_LineNumber is 0 and UngetLine() was not run,
    the latter was already checked) - returns NULL */
    if (m_Line.begin() == NULL) {
        return CTempString(NULL);
    }
    return m_Line;
}


CT_POS_TYPE CBufferedLineReader::GetPosition(void) const
{
    CT_OFF_TYPE offset = m_Pos - m_Buffer.get();
    if (m_UngetLine) {
        offset -= m_LastReadSize;
    }
    return m_InputPos + offset;
}


Uint8 CBufferedLineReader::GetLineNumber(void) const
{
    /* Right after constructor (m_LineNumber is 0 and UngetLine() was not run)
       - 0 */
    /* If at EOF - returns the number of the last string */
    /* After UngetLine() - number of the previous string */
    /* Not at EOF, not after UngetLine() - number of the current string */
    return m_LineNumber;
}


END_NCBI_SCOPE
