/*
Copyright 2022 MetaOPT Team. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
================================================================================
*/

// See https://jax.readthedocs.io/en/latest/pytrees.html for the documentation
// about PyTrees.

// Caution: this code uses exceptions. The exception use is local to the
// binding code and the idiomatic way to emit Python exceptions.

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <pybind11/pybind11.h>
#include <pybind11_abseil/absl_casters.h>

#include <memory>

namespace optree {

namespace py = pybind11;

enum class PyTreeKind {
    Leaf,        // An opaque leaf node
    None,        // None
    Tuple,       // A tuple
    NamedTuple,  // A collections.namedtuple
    List,        // A list
    Dict,        // A dict
    Custom,      // A custom type
};

// Registry of custom node types.
class PyTreeTypeRegistry {
 public:
    struct Registration {
        PyTreeKind kind;

        // The following values are populated for custom types.
        // The Python type object, used to identify the type.
        py::object type;
        // A function with signature: object -> (iterable, aux_data)
        py::function to_iterable;
        // A function with signature: (aux_data, iterable) -> object
        py::function from_iterable;
    };

    // Registers a new custom type. Objects of `type` will be treated as container
    // node types in PyTrees.
    static void Register(py::object type, py::function to_iterable, py::function from_iterable);

    // Finds the custom type registration for `type`. Returns nullptr if none
    // exists.
    static const Registration *Lookup(py::handle type);

 private:
    static PyTreeTypeRegistry *Singleton();

    class TypeHash {
     public:
        using is_transparent = void;
        size_t operator()(const py::object &t) const;
        size_t operator()(const py::handle &t) const;
    };
    class TypeEq {
     public:
        using is_transparent = void;
        bool operator()(const py::object &a, const py::object &b) const;
        bool operator()(const py::object &a, const py::handle &b) const;
    };

    absl::flat_hash_map<py::object, std::unique_ptr<Registration>, TypeHash, TypeEq> registrations;
};

}  // namespace optree