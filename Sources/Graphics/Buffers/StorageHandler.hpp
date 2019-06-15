#pragma once

#include "StorageBuffer.hpp"

namespace acid
{
/**
 * @brief Class that handles a storage buffer.
 */
class ACID_EXPORT StorageHandler
{
public:
	explicit StorageHandler(const bool &multipipeline = false);

	explicit StorageHandler(const Shader::UniformBlock &uniformBlock, const bool &multipipeline = false);

	void Push(void *data, const std::size_t &size)
	{
		if (size != m_size)
		{
			m_size = static_cast<uint32_t>(size);
			m_handlerStatus = Buffer::Status::Reset;
			return;
		}

		if (!m_uniformBlock || !m_storageBuffer)
		{
			return;
		}

		if (!m_bound)
		{
			m_storageBuffer->MapMemory(&m_data);
			m_bound = true;
		}

		if (std::memcmp(static_cast<char *>(m_data), data, size) != 0)
		{
			std::memcpy(static_cast<char *>(m_data), data, size);
			m_handlerStatus = Buffer::Status::Changed;
		}
	}

	template<typename T>
	void Push(const T &object, const std::size_t &offset, const std::size_t &size)
	{
		if (!m_uniformBlock || !m_storageBuffer)
		{
			return;
		}

		if (!m_bound)
		{
			m_storageBuffer->MapMemory(&m_data);
			m_bound = true;
		}

		if (std::memcmp(static_cast<char *>(m_data) + offset, &object, size) != 0)
		{
			std::memcpy(static_cast<char *>(m_data) + offset, &object, size);
			m_handlerStatus = Buffer::Status::Changed;
		}
	}

	template<typename T>
	void Push(const std::string &uniformName, const T &object, const std::size_t &size = 0)
	{
		if (!m_uniformBlock)
		{
			return;
		}

		auto uniform{m_uniformBlock->GetUniform(uniformName)};

		if (!uniform)
		{
			return;
		}

		auto realSize{size};

		if (realSize == 0)
		{
			realSize = std::min(sizeof(object), static_cast<std::size_t>(uniform->GetSize()));
		}

		Push(object, static_cast<std::size_t>(uniform->GetOffset()), realSize);
	}

	bool Update(const std::optional<Shader::UniformBlock> &uniformBlock);

	const StorageBuffer *GetStorageBuffer() const { return m_storageBuffer.get(); }

private:
	bool m_multipipeline;
	std::optional<Shader::UniformBlock> m_uniformBlock;
	uint32_t m_size{};
	void *m_data{};
	bool m_bound{};
	std::unique_ptr<StorageBuffer> m_storageBuffer;
	Buffer::Status m_handlerStatus;
};
}