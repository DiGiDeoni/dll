//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pooling_layer_desc.hpp"

namespace dll {

template <std::size_t T_I1, std::size_t T_I2, std::size_t T_I3, std::size_t T_C1, std::size_t T_C2, std::size_t T_C3, typename... Parameters>
struct mp_layer_3d_desc : pooling_layer_3d_desc<T_I1, T_I2, T_I3, T_C1, T_C2, T_C3, Parameters...> {
    /*!
     * A list of all the parameters of the descriptor
     */
    using parameters = cpp::type_list<Parameters...>;

    /*! The RBM type */
    using layer_t = mp_layer_3d<mp_layer_3d_desc<T_I1, T_I2, T_I3, T_C1, T_C2, T_C3, Parameters...>>;

    /*! The RBM type */
    using dyn_layer_t = dyn_mp_layer_3d<dyn_mp_layer_3d_desc<Parameters...>>;
};

} //end of dll namespace
