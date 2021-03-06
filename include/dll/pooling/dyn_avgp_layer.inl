//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pooling_layer.hpp"

namespace dll {

/*!
 * \brief Standard average pooling layer
 */
template <typename Desc>
struct dyn_avgp_layer_3d final : dyn_pooling_layer_3d<dyn_avgp_layer_3d<Desc>, Desc> {
    using desc      = Desc;                                  ///< The layer descriptor
    using weight    = typename desc::weight;                 ///< The layer weight type
    using this_type = dyn_avgp_layer_3d<Desc>;               ///< This layer's type
    using base      = dyn_pooling_layer_3d<this_type, desc>; ///< The layer base type

    dyn_avgp_layer_3d() = default;

    /*!
     * \brief Get a string representation of the layer
     */
    std::string to_short_string() const {
        char buffer[1024];
        snprintf(buffer, 1024, "AVGP(3D): %lux%lux%lu -> (%lux%lux%lu) -> %lux%lux%lu",
                 base::i1, base::i2, base::i3, base::c1, base::c2, base::c3, base::o1, base::o2, base::o3);
        return {buffer};
    }

    using input_one_t  = typename base::input_one_t;  ///< The type of one input
    using output_one_t = typename base::output_one_t; ///< The type of one output
    using input_t      = typename base::input_t;      ///< The type of many input
    using output_t     = typename base::output_t;     ///< The type of many output

    /*!
     * \brief Forward activation of the layer for one sample
     * \param h The output matrix
     * \param v The input matrix
     */
    void activate_hidden(output_one_t& h, const input_one_t& v) const {
        h = etl::avg_pool_3d(v, base::c1, base::c2, base::c3);
    }

    /*!
     * \brief Forward activation of the layer for one batch of sample
     * \param output The output matrix
     * \param input The input matrix
     */
    template <typename Input, typename Output>
    void batch_activate_hidden(Output& output, const Input& input) const {
        output = etl::avg_pool_3d(input, base::c1, base::c2, base::c3);
    }

    template <typename DBN>
    void init_sgd_context() {
        this->sgd_context_ptr = std::make_shared<sgd_context<DBN, this_type>>(base::i1, base::i2, base::i3, base::c1, base::c2, base::c3);
    }

    template<typename DRBM>
    static void dyn_init(DRBM&){
        //Nothing to change
    }

    /*!
     * \brief Adapt the errors, called before backpropagation of the errors.
     *
     * This must be used by layers that have both an activation fnction and a non-linearity.
     *
     * \param context the training context
     */
    template<typename C>
    void adapt_errors(C& context) const {
        cpp_unused(context);
    }

    /*!
     * \brief Backpropagate the errors to the previous layers
     * \param output The ETL expression into which write the output
     * \param context The training context
     */
    template<typename H, typename C>
    void backward_batch(H&& output, C& context) const {
        size_t c1 = base::c1;
        size_t c2 = base::c2;
        size_t c3 = base::c3;

        output = etl::avg_pool_derivative_3d(context.input, context.output, c1, c2, c3) >> etl::upsample_3d(context.errors, c1, c2, c3);
    }

    /*!
     * \brief Compute the gradients for this layer, if any
     * \param context The trainng context
     */
    template<typename C>
    void compute_gradients(C& context) const {
        cpp_unused(context);
    }
};

// Declare the traits for the Layer

template<typename Desc>
struct layer_base_traits<dyn_avgp_layer_3d<Desc>> {
    static constexpr bool is_neural     = false; ///< Indicates if the layer is a neural layer
    static constexpr bool is_dense      = false; ///< Indicates if the layer is dense
    static constexpr bool is_conv       = false; ///< Indicates if the layer is convolutional
    static constexpr bool is_deconv     = false; ///< Indicates if the layer is deconvolutional
    static constexpr bool is_standard   = true;  ///< Indicates if the layer is standard
    static constexpr bool is_rbm        = false; ///< Indicates if the layer is RBM
    static constexpr bool is_pooling    = true;  ///< Indicates if the layer is a pooling layer
    static constexpr bool is_unpooling  = false; ///< Indicates if the layer is an unpooling laye
    static constexpr bool is_transform  = false; ///< Indicates if the layer is a transform layer
    static constexpr bool is_patches    = false; ///< Indicates if the layer is a patches layer
    static constexpr bool is_augment    = false; ///< Indicates if the layer is an augment layer
    static constexpr bool is_dynamic    = true; ///< Indicates if the layer is dynamic
    static constexpr bool pretrain_last = false; ///< Indicates if the layer is dynamic
    static constexpr bool sgd_supported = true;  ///< Indicates if the layer is supported by SGD
};

/*!
 * \brief Specialization of sgd_context for dyn_mp_layer
 */
template <typename DBN, typename Desc>
struct sgd_context<DBN, dyn_avgp_layer_3d<Desc>> {
    using layer_t = dyn_avgp_layer_3d<Desc>;
    using weight  = typename layer_t::weight;

    static constexpr auto batch_size = DBN::batch_size;

    etl::dyn_matrix<weight, 4> input;
    etl::dyn_matrix<weight, 4> output;
    etl::dyn_matrix<weight, 4> errors;

    sgd_context(size_t i1, size_t i2, size_t i3, size_t c1, size_t c2, size_t c3)
            : input(batch_size, i1, i2, i3),
              output(batch_size, i1 / c1, i2 / c2, i3 / c3),
              errors(batch_size, i1 / c1, i2 / c2, i3 / c3) {}
};

} //end of dll namespace
