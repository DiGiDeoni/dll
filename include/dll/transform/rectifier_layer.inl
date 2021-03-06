//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "dll/base_traits.hpp"
#include "transform_layer.hpp"

namespace dll {

/*!
 * \brief Configuraable rectifier layer.
 *
 * Use abs as a rectifier by default
 */
template <typename Desc>
struct rectifier_layer : transform_layer<rectifier_layer<Desc>> {
    using desc = Desc; ///< The descriptor type

    static constexpr const rectifier_method method = desc::method; ///< The rectifier method

    static_assert(method == rectifier_method::ABS, "Only ABS rectifier has been implemented");

    /*!
     * \brief Returns a string representation of the layer
     */
    static std::string to_short_string() {
        return "Rectifier";
    }

    /*!
     * \brief Apply the layer to the input
     * \param output The output
     * \param input The input to apply the layer to
     */
    template <typename Input, typename Output>
    static void activate_hidden(Output& output, const Input& input) {
        if (method == rectifier_method::ABS) {
            output = etl::abs(input);
        }
    }

    /*!
     * \brief Apply the layer to the batch of input
     * \param output The batch of output
     * \param input The batch of input to apply the layer to
     */
    template <typename Input, typename Output>
    static void batch_activate_hidden(Output& output, const Input& input) {
        if (method == rectifier_method::ABS) {
            output = etl::abs(input);
        }
    }
};

//Allow odr-use of the constexpr static members

template <typename Desc>
const rectifier_method rectifier_layer<Desc>::method;

// Declare the traits for the layer

template<typename Desc>
struct layer_base_traits<rectifier_layer<Desc>> {
    static constexpr bool is_neural     = false; ///< Indicates if the layer is a neural layer
    static constexpr bool is_dense      = false; ///< Indicates if the layer is dense
    static constexpr bool is_conv       = false; ///< Indicates if the layer is convolutional
    static constexpr bool is_deconv     = false; ///< Indicates if the layer is deconvolutional
    static constexpr bool is_standard   = false; ///< Indicates if the layer is standard
    static constexpr bool is_rbm        = false; ///< Indicates if the layer is RBM
    static constexpr bool is_pooling    = false; ///< Indicates if the layer is a pooling layer
    static constexpr bool is_unpooling  = false; ///< Indicates if the layer is an unpooling laye
    static constexpr bool is_transform  = true;  ///< Indicates if the layer is a transform layer
    static constexpr bool is_patches    = false; ///< Indicates if the layer is a patches layer
    static constexpr bool is_augment    = false; ///< Indicates if the layer is an augment layer
    static constexpr bool is_dynamic    = false; ///< Indicates if the layer is dynamic
    static constexpr bool pretrain_last = false; ///< Indicates if the layer is dynamic
    static constexpr bool sgd_supported = true;  ///< Indicates if the layer is supported by SGD
};

/*!
 * \brief Specialization of sgd_context for lcn_layer
 */
template <typename DBN, typename Desc>
struct sgd_context<DBN, rectifier_layer<Desc>> {
    using layer_t = rectifier_layer<Desc>;
    using weight  = typename DBN::weight;

    using inputs_t = transform_output_type_t<DBN, layer_t>;

    inputs_t input;
    inputs_t output;
    inputs_t errors;
};

} //end of dll namespace
