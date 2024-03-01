//
// Created by 37496 on 2024/2/4.
//

#ifndef WEBSERVER_STRUCT_CPP_YAML_H
#define WEBSERVER_STRUCT_CPP_YAML_H


#include "yamlutil.h"
#include "visit_struct/visit_struct.hpp"
#include "magic_enum/magic_enum.hpp"


#ifdef NOT_USE_YCS_INIT_VALUE
#define YCS_ADD_STRUCT(T, ...)                                                         \
	VISITABLE_STRUCT(T, __VA_ARGS__);                                                  \
	namespace YAML {                                                                   \
	template <>                                                                        \
	struct convert<T> {                                                                \
		static bool decode(const Node& node, T& rhs) {                                 \
			visit_struct::for_each(rhs, [&](const char* name, auto& value) {           \
				using ToType = std::remove_reference_t<std::decay_t<decltype(value)>>; \
				if constexpr (yaml_struct::is_optional<ToType>()) {                \
					try {                                                              \
						value = yaml_struct::node_as<ToType>(node[name]);          \
					} catch (const std::runtime_error&) {                              \
					}                                                                  \
				}                                                                      \
				else                                                                   \
					try {                                                              \
						value = yaml_struct::node_as<ToType>(node[name]);          \
					} catch (const std::runtime_error&) {                              \
						auto error = yaml_struct::string_format("lost: %s", name); \
						throw yaml_struct::Exception(error);                       \
					}                                                                  \
			});                                                                        \
			return true;                                                               \
		}                                                                              \
		static Node encode(const T& rhs) {                                             \
			Node node(NodeType::Map);                                                  \
			visit_struct::for_each(rhs, [&](const char* name, auto& value) {           \
				node[name] = value;                                                    \
			});                                                                        \
			return node;                                                               \
		}                                                                              \
	};                                                                                 \
	}
#else
#define YCS_ADD_STRUCT(T, ...)                                                         \
	VISITABLE_STRUCT(T, __VA_ARGS__);                                                  \
	namespace YAML {                                                                   \
	template <>                                                                        \
	struct convert<T> {                                                                \
		static bool decode(const Node& node, T& rhs) {                                 \
			visit_struct::for_each(rhs, [&](const char* name, auto& value) {           \
				using ToType = std::remove_reference_t<std::decay_t<decltype(value)>>; \
				try {                                                                  \
					value = yaml_struct::node_as<ToType>(node[name]);              \
				} catch (const std::runtime_error&) {                                  \
				}                                                                      \
			});                                                                        \
			return true;                                                               \
		}                                                                              \
		static Node encode(const T& rhs) {                                             \
			Node node(NodeType::Map);                                                  \
			visit_struct::for_each(rhs, [&](const char* name, auto& value) {           \
				node[name] = value;                                                    \
			});                                                                        \
			return node;                                                               \
		}                                                                              \
	};                                                                                 \
	}
#endif

#define YCS_ADD_ENUM(E, ...)                                                                         \
	namespace YAML {                                                                                 \
	template <>                                                                                      \
	struct convert<E> {                                                                              \
		static bool decode(const Node& node, E& rhs) {                                               \
			const auto enum_str = node.as<std::string>();                                            \
			auto value = magic_enum::enum_cast<E>(enum_str);                                         \
			if (value.has_value())                                                                   \
				rhs = value.value();                                                                 \
			else {                                                                                   \
				auto error = yaml_struct::string_format("bad enum value: %s", enum_str.c_str()); \
				throw yaml_struct::Exception(error);                                             \
			}                                                                                        \
			return true;                                                                             \
		}                                                                                            \
		static Node encode(const E& rhs) {                                                           \
			auto enum_name = magic_enum::enum_name(rhs);                                             \
			return Node{enum_name.data()};                                                           \
		}                                                                                            \
	};                                                                                               \
	}

#endif //WEBSERVER_STRUCT_CPP_YAML_H
