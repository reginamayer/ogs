/**
 * \copyright
 * Copyright (c) 2013, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#ifndef ASSEMBLERLIB_GLOBALSETUP_H_
#define ASSEMBLERLIB_GLOBALSETUP_H_

#include <functional>

#include "MeshComponentMap.h"

namespace AssemblerLib
{

/// The GlobalSetup collects vector and matrix builder and corresponding global
/// loop executor.
template <typename VectorMatrixBuilder, typename Executor>
struct GlobalSetup
{
    typedef typename VectorMatrixBuilder::VectorType VectorType;
    typedef typename VectorMatrixBuilder::MatrixType MatrixType;

    template <typename... Args>
    VectorType* createVector(Args&& ... args) const
    {
        return VectorMatrixBuilder::createVector(std::forward<Args>(args)...);
    }

    template <typename... Args>
    MatrixType* createMatrix(Args&& ... args) const
    {
        return VectorMatrixBuilder::createMatrix(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void execute(Args&& ... args) const
    {
        return Executor::execute(std::forward<Args>(args)...);
    }
};

}   // namespace AssemblerLib

#endif  // ASSEMBLERLIB_GLOBALSETUP_H_
