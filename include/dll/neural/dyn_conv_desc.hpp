//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "dll/base_conf.hpp"
#include "dll/util/tmp.hpp"

namespace dll {

/*!
 * \brief Describe a standard dynamic convolutional layer.
 */
template <typename... Parameters>
struct dyn_conv_desc {
    /*!
     * \brief A list of all the parameters of the descriptor
     */
    using parameters = cpp::type_list<Parameters...>;

    static constexpr auto activation_function = detail::get_value<activation<function::SIGMOID>, Parameters...>::value;
    static constexpr auto w_initializer       = detail::get_value<initializer<initializer_type::LECUN>, Parameters...>::value;
    static constexpr auto b_initializer       = detail::get_value<initializer_bias<initializer_type::LECUN>, Parameters...>::value;

    /*! The type used to store the weights */
    using weight = typename detail::get_type<weight_type<float>, Parameters...>::value;

    /*! The conv type */
    using layer_t = dyn_conv_layer<dyn_conv_desc<Parameters...>>;

    /*! The conv type */
    using dyn_layer_t = dyn_conv_layer<dyn_conv_desc<Parameters...>>;

    //Make sure only valid types are passed to the configuration list
    static_assert(
        detail::is_valid<cpp::type_list<weight_type_id, activation_id, initializer_id, initializer_bias_id>, Parameters...>::value,
        "Invalid parameters type for dyn_conv_desc");
};

} //end of dll namespace
