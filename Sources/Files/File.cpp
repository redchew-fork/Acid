﻿#include "File.hpp"

#include "Engine/Engine.hpp"
#include "Files.hpp"
#include "FileSystem.hpp"

namespace acid
{
File::File(std::string filename, std::unique_ptr<Metadata> &&metadata) :
	m_filename{std::move(filename)},
	m_metadata{std::move(metadata)}
{
}

void File::Load()
{
#if defined(ACID_VERBOSE)
	auto debugStart = Time::Now();
#endif

	if (Files::ExistsInPath(m_filename))
	{
		IFStream inStream(m_filename);
		m_metadata->Load(&inStream);
	}
	else if (FileSystem::Exists(m_filename))
	{
		std::ifstream inStream(m_filename);
		m_metadata->Load(&inStream);
		inStream.close();
	}

#if defined(ACID_VERBOSE)
	auto debugEnd = Time::Now();
	Log::Out("File '%s' loaded in %.3fms\n", m_filename, (debugEnd - debugStart).AsMilliseconds<float>());
#endif
}

void File::Write() const
{
#if defined(ACID_VERBOSE)
	auto debugStart = Time::Now();
#endif

	if (Files::ExistsInPath(m_filename))
	{
		OFStream outStream(m_filename);
		m_metadata->Write(&outStream);
	}
	else // if (FileSystem::Exists(m_filename))
	{
		FileSystem::Create(m_filename);
		std::ofstream outStream(m_filename);
		m_metadata->Write(&outStream);
		outStream.close();
	}

#if defined(ACID_VERBOSE)
	auto debugEnd = Time::Now();
	Log::Out("File '%s' saved in %.3fms\n", m_filename, (debugEnd - debugStart).AsMilliseconds<float>());
#endif
}

void File::Clear()
{
	m_metadata->ClearChildren();
}
}
