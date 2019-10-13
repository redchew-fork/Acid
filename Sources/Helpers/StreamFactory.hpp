#pragma once

#include "Engine/Log.hpp"
#include "Files/Node.hpp"

namespace acid {
template<typename Base, class... Args>
class StreamFactory {
public:
	using TCreateReturn = std::unique_ptr<Base>;

	using TCreateMethod = std::function<TCreateReturn(Args...)>;
	using TRegistryMap = std::unordered_map<std::string, TCreateMethod>;

	virtual ~StreamFactory() = default;

	static TCreateReturn Create(const std::string &name, Args &&... args) {
		auto it = Registry().find(name);
		if (it == Registry().end()) {
			Log::Error("Failed to create ", std::quoted(name), " from factory\n");
			return nullptr;
		}
		return it->second(std::forward<Args>(args)...);
	}

	static TRegistryMap &Registry() {
		static TRegistryMap impl;
		return impl;
	}

	template<typename T>
	class Registrar : public Base {
	protected:
		static bool Register(const std::string &name) {
			StreamFactory::Registry()[name] = [](Args... args) -> TCreateReturn {
				return std::make_unique<T>(std::forward<Args>(args)...);
			};
			return true;
		}
		
		const Node &Load(const Node &node) override {
			return node >> *dynamic_cast<T *>(this);
		}
		
		Node &Write(Node &node) const override {
			return node << *dynamic_cast<const T *>(this);
		}
	};

	friend inline const Node &operator>>(const Node &node, std::unique_ptr<Base> &object) {
		object = Create(node["type"].Get<std::string>());
		node >> *object;
		return node;
	}

	/*friend inline Node &operator<<(Node &node, const std::unique_ptr<Base> &object) {
		if (object == nullptr) {
			return node << nullptr;
		}

		node << *object;
		return node;
	}*/

	friend const Node &operator>>(const Node &node, Base &base) {
		return base.Load(node);
	}
	
	friend Node &operator<<(Node &node, const Base &base) {
		return base.Write(node);
	}
protected:
	virtual const Node &Load(const Node &node) { return node; }
	virtual Node &Write(Node &node) const { return node; }
};
}
