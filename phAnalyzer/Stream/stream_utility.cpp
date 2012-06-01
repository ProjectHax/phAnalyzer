#include "stream_utility.h"
#include <iostream>
#include <iomanip>
#include <cctype>
#include <sstream>

//-----------------------------------------------------------------------------

StreamUtility::StreamUtility()
: m_stream( m_stream_internal )
{
	Clear();
}

StreamUtility::StreamUtility( std::vector< uint8_t > & stream )
: m_stream( stream )
{
	m_read_index = 0;
	m_read_error = false;
	m_write_error = 0;
}

StreamUtility::StreamUtility( const void * const buffer, int32_t size )
: m_stream( m_stream_internal )
{
	Clear();
	Overwrite< uint8_t >( 0, reinterpret_cast< const uint8_t * >( buffer ), size );
}

StreamUtility::StreamUtility( const StreamUtility & rhs )
: m_stream( m_stream_internal ), m_read_index ( rhs.m_read_index ), m_read_error( rhs.m_read_error )
{
	m_stream = rhs.m_stream;
}

StreamUtility::~StreamUtility()
{
}

StreamUtility & StreamUtility::operator =( const StreamUtility & rhs )
{
	if( &rhs != this )
	{
		m_stream = rhs.m_stream;
		m_read_index = rhs.m_read_index;
		m_read_error = rhs.m_read_error;
		m_write_error = rhs.m_read_error;
	}
	return *this;
}

void StreamUtility::Clear()
{
	m_stream.clear();
	m_read_index = 0;
	m_read_error = false;
	m_write_error = false;
}

bool StreamUtility::WasWriteError()
{
	return m_write_error;
}

bool StreamUtility::WasReadError()
{
	return m_read_error;
}

void StreamUtility::ClearReadError()
{
	m_read_error = false;
}

void StreamUtility::ClearWriteError()
{
	m_write_error = false;
}

const std::vector< uint8_t > & StreamUtility::GetStreamVector()
{
	return m_stream;
}

const uint8_t * StreamUtility::GetStreamPtr()
{
	return m_stream.empty() ? 0 : &m_stream[0];
}

int32_t StreamUtility::GetStreamSize()
{
	return static_cast< int32_t >( m_stream.size() );
}

int32_t StreamUtility::GetWriteIndex()
{
	return static_cast< int32_t >( m_stream.size() ) - 1;
}

const uint8_t * StreamUtility::GetReadStreamPtr()
{
	if( m_read_index == 0 || m_read_index >= static_cast< int32_t >( m_stream.size() ) )
	{
		return NULL;
	}
	return &m_stream[m_read_index];
}

int32_t StreamUtility::GetReadStreamSize()
{
	if( m_read_index >= static_cast< int32_t >( m_stream.size() ) )
	{
		return 0;
	}
	return static_cast< int32_t >( m_stream.size() ) - m_read_index;
}

int32_t StreamUtility::GetReadIndex()
{
	return m_read_index;
}

bool StreamUtility::SeekRead( int32_t index, SeekDirection dir )
{
	int32_t old_read_index = m_read_index;
	if( dir == Seek_Set )
	{
		m_read_index = index;
	}
	else if( dir == Seek_Forward )
	{
		m_read_index += index;
	}
	else if( dir == Seek_Backward )
	{
		m_read_index -= index;
	}
	else if( dir == Seek_End )
	{
		m_read_index = static_cast< int32_t >( m_stream.size() ) - 1;
		m_read_index -= index;
	}
	if( m_read_index > static_cast< int32_t >( m_stream.size() ) )
	{
		m_read_index = old_read_index;
		return false;
	}
	return true;
}

int32_t StreamUtility::Delete( int32_t index, int32_t count )
{
	int32_t delcount = 0;
	if( index < static_cast< int32_t >( m_stream.size() ) )
	{
		delcount = static_cast< int32_t >( m_stream.size() ) - index;
		if( delcount > count )
		{
			delcount = count;
		}
	}
	if( delcount )
	{
		m_stream.erase( m_stream.begin() + index, m_stream.begin() + index + delcount );
		if( m_read_index > static_cast< int32_t >( m_stream.size() ) )
		{
			m_read_index = static_cast< int32_t >( m_stream.size() );
		}
	}
	return delcount;
}

std::string StreamUtility::Read_Ascii( int32_t count )
{
	if( count == 0 )
	{
		return "";
	}
	std::string str;
	str.resize( count );
	Read< char >( &str[0], count );
	if( m_read_error )
	{
		str.clear();
	}
	return str;
}

std::wstring StreamUtility::Read_AsciiToUnicode( int32_t count )
{
	if( count == 0 )
	{
		return L"";
	}
	std::string str;
	str.resize( count );
	Read< char >( &str[0], count );
	if( m_read_error )
	{
		return L"";
	}
	int32_t converted = static_cast< int32_t >( mbstowcs( NULL, str.c_str(), count ) );
	if( converted == 0 || ( converted - 1 ) != count )
	{
		m_read_error = true;
		return L"";
	}
	std::wstring wcsStr;
	wcsStr.resize( converted );
	mbstowcs( &wcsStr[0], str.c_str(), count );
	wcsStr.erase( wcsStr.end() - 1 ); // remove extra null terminator
	return wcsStr;
}

std::wstring StreamUtility::Read_Unicode( int32_t count )
{
	if( count == 0 )
	{
		return L"";
	}
	std::wstring str;
	str.resize( count );
	Read< wchar_t >( &str[0], count );
	if( m_read_error )
	{
		str.clear();
	}
	return str;
}

std::string StreamUtility::Read_UnicodeToAscii( int32_t count )
{
	if( count == 0 )
	{
		return "";
	}
	std::wstring str;
	str.resize( count );
	Read< wchar_t >( &str[0], count );
	if( m_read_error )
	{
		return "";
	}
	int32_t converted = static_cast< int32_t >( wcstombs( NULL, str.c_str(), count ) );
	if( converted == 0 || ( converted - 1 ) != count )
	{
		m_read_error = true;
		return "";
	}
	std::string mbsStr;
	mbsStr.resize( converted );
	wcstombs( &mbsStr[0], str.c_str(), count );
	mbsStr.erase( mbsStr.end() - 1 ); // remove extra null terminator
	return mbsStr;
}

void StreamUtility::Write_Ascii( const std::string & mbs_text )
{
	Write_Ascii( mbs_text.c_str(), static_cast< int32_t >( mbs_text.size() ) );
}

void StreamUtility::Write_AsciiToUnicode( const std::string & mbs_text )
{
	Write_AsciiToUnicode( mbs_text.c_str(), static_cast< int32_t >( mbs_text.size() ) );
}

void StreamUtility::Write_UnicodeToAscii( const std::wstring & wcs_text )
{
	Write_UnicodeToAscii( wcs_text.c_str(), static_cast< int32_t >( wcs_text.size()  ) );
}

void StreamUtility::Write_Unicode( const std::wstring & wcs_text )
{
	Write_Unicode( wcs_text.c_str(), static_cast< int32_t >( wcs_text.size() ) );
}

void StreamUtility::Write_Ascii( const char * mbs_text, int32_t count )
{
	Write< char >( mbs_text, count );
}

void StreamUtility::Write_AsciiToUnicode( const char * mbs_text, int32_t count )
{
	if( count == 0 )
	{
		return;
	}
	int32_t converted = static_cast< int32_t >( mbstowcs( NULL, mbs_text, count ) );
	if( converted == 0 || ( converted - 1 ) != count )
	{
		m_write_error = true;
		return;
	}
	std::wstring wcsStr;
	wcsStr.resize( converted );
	mbstowcs( &wcsStr[0], mbs_text, count );
	Write_Unicode( wcsStr.c_str(), converted - 1 );
}

void StreamUtility::Write_UnicodeToAscii( const wchar_t * wcs_text, int32_t count )
{
	if( count == 0 )
	{
		return;
	}
	int32_t converted = static_cast< int32_t >( wcstombs( NULL, wcs_text, count ) );
	if( converted == 0 || ( converted - 1 ) != count )
	{
		m_write_error = true;
		return;
	}
	std::string mbsStr;
	mbsStr.resize( converted );
	wcstombs( &mbsStr[0], wcs_text, count );
	Write_Ascii( mbsStr.c_str(), converted - 1 );
}

void StreamUtility::Write_Unicode( const wchar_t * wcs_text, int32_t count )
{
	Write< wchar_t >( wcs_text, count );
}

StreamUtility StreamUtility::Extract( int32_t index, int32_t count )
{
	StreamUtility result;
	if( count == -1 )
	{
		count = static_cast< int32_t >( m_stream.size() ) - index;
	}
	if( count && index + count <= static_cast< int32_t >( m_stream.size() ) )
	{
		result.Write( &m_stream[index], count );
	}
	return result;
}

//-----------------------------------------------------------------------------

std::string DumpToString( StreamUtility & stream_utility )
{
	return DumpToString( stream_utility.GetStreamVector() );
}

std::string DumpToString( const std::vector< uint8_t > & buffer )
{
	return DumpToString( buffer.empty() ? 0 : &buffer[0], static_cast< int32_t >( buffer.size() ) );
}

std::string DumpToString( const void * stream, int32_t size )
{
	std::stringstream ss;
	const unsigned char * buffer = reinterpret_cast< const unsigned char * >( stream );
	int fields[16];
	int32_t index = 0;
	int32_t cur = 0;
	int32_t total = size;
	if( total == 0 || total % 16 )
	{
		total += ( 16 - ( total % 16 ) );
	}
	for( int32_t x = 0; x < total; ++x )
	{
		fields[ index++ ] = cur < size ? buffer[ x ] : 0;
		++cur;
		if( index == 16 )
		{
			for( int32_t y = 0; y < 16; ++y )
			{
				if( cur - 16 + y < size )
				{
					ss << std::hex << std::setfill( '0' ) << std::setw( 2 ) << fields[ y ] << " ";
				}
				else
				{
					ss << "   ";
				}
			}
			ss << "   ";
			for( int32_t y = 0; y < 16; ++y )
			{
				if( cur - 16 + y < size )
				{
					int ch = fields[ y ];
					if( isprint( ch ) && !isspace( ch ) )
					{
						ss << (char)ch;
					}
					else
					{
						ss << ".";
					}
				}
				else
				{
					ss << ".";
				}
			}
			ss << std::endl;
			index = 0;
		}
	}
	std::string final = ss.str();
	return final;
}

//-----------------------------------------------------------------------------
