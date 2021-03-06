//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "util/tmp.hpp"
#include "base_traits.hpp"
#include "layer_fwd.hpp"
#include "base_conf.hpp"

namespace dll {

/*!
 * \brief Type Traits to get information on layer type
 */
template <typename Layer>
struct layer_traits {
    using layer_t = Layer; ///< The layer type being inspected
    using base_traits = layer_base_traits<layer_t>;

    /*!
     * \brief Indicates if the layer is neural (dense or conv)
     */
    static constexpr bool is_neural_layer() {
        return base_traits::is_neural;
    }

    /*!
     * \brief Indicates if the layer is dense
     */
    static constexpr bool is_dense_layer() {
        return base_traits::is_dense;
    }

    /*!
     * \brief Indicates if the layer is convolutional
     */
    static constexpr bool is_convolutional_layer() {
        return base_traits::is_conv;
    }

    /*!
     * \brief Indicates if the layer is convolutional
     */
    static constexpr bool is_deconvolutional_layer() {
        return base_traits::is_deconv;
    }

    /*!
     * \brief Indicates if the layer is a standard (non-rbm) layer.
     */
    static constexpr bool is_standard_layer() {
        return base_traits::is_standard;
    }

    /*!
     * \brief Indicates if the layer is a standard (non-rbm) dense layer.
     */
    static constexpr bool is_standard_dense_layer() {
        return is_standard_layer() && is_dense_layer();
    }

    /*!
     * \brief Indicates if the layer is a standard (non-rbm) convolutionl layer.
     */
    static constexpr bool is_standard_convolutional_layer() {
        return is_standard_layer() && is_convolutional_layer();
    }

    /*!
     * \brief Indicates if the layer is a standard (non-rbm) deconvolutionl layer.
     */
    static constexpr bool is_standard_deconvolutional_layer() {
        return is_standard_layer() && is_deconvolutional_layer();
    }

    /*!
     * \brief Indicates if this layer is a RBM layer.
     */
    static constexpr bool is_rbm_layer() {
        return base_traits::is_rbm;
    }

    /*!
     * \brief Indicates if this layer is a dense RBM layer.
     */
    static constexpr bool is_dense_rbm_layer() {
        return is_rbm_layer() && is_dense_layer();
    }

    /*!
     * \brief Indicates if the layer is convolutional
     */
    static constexpr bool is_convolutional_rbm_layer() {
        return is_rbm_layer() && is_convolutional_layer();
    }

    /*!
     * \brief Indicates if this layer is a pooling layer.
     */
    static constexpr bool is_pooling_layer() {
        return base_traits::is_pooling;
    }

    /*!
     * \brief Indicates if this layer is a pooling layer.
     */
    static constexpr bool is_unpooling_layer() {
        return base_traits::is_unpooling;
    }

    /*!
     * \brief Indicates if this layer is a transformation layer.
     */
    static constexpr bool is_transform_layer() {
        return base_traits::is_transform;
    }

    /*!
     * \brief Indicates if this layer is a patches layer.
     */
    static constexpr bool is_patches_layer() {
        return base_traits::is_patches;
    }

    /*!
     * \brief Indicates if this layer is an augmentation layer.
     */
    static constexpr bool is_augment_layer() {
        return base_traits::is_augment;
    }

    /*!
     * \brief Indicates if this layer is a multipley layer.
     */
    static constexpr bool is_multiplex_layer() {
        return is_augment_layer() || is_patches_layer();
    }

    /*!
     * \brief Indicates if this layer keeps the same type
     */
    static constexpr bool has_same_type() {
        return is_transform_layer() || is_augment_layer();
    }

    /*!
     * \brief Indicates if this layer is trained or not.
     */
    static constexpr bool is_trained() {
        return is_neural_layer();
    }

    /*!
     * \brief Indicates if this layer is pretrained or not.
     */
    static constexpr bool is_pretrained() {
        return is_rbm_layer();
    }

    /*!
     * \brief Indicates if the layer is dynamic
     */
    static constexpr bool is_dynamic() {
        return base_traits::is_dynamic;
    }

    static constexpr bool pretrain_last() {
        return base_traits::pretrain_last;
    }
};

/*!
 * \brief Type Traits to get information on layer type
 */
template <typename Layer>
struct rbm_layer_traits {
    using layer_t = Layer; ///< The layer type being inspected
    using base_traits = rbm_layer_base_traits<layer_t>;

    static constexpr bool has_momentum() {
        return base_traits::has_momentum;
    }

    static constexpr bool has_clip_gradients() {
        return base_traits::has_clip_gradients;
    }

    static constexpr bool is_parallel_mode() {
        return base_traits::is_parallel_mode;
    }

    static constexpr bool is_serial() {
        return base_traits::is_serial;
    }

    static constexpr bool is_verbose() {
        return base_traits::is_verbose;
    }

    static constexpr bool has_shuffle() {
        return base_traits::has_shuffle;
    }

    static constexpr bool is_dbn_only() {
        return base_traits::is_dbn_only;
    }

    static constexpr bool has_sparsity() {
        return base_traits::has_sparsity;
    }

    static constexpr dll::sparsity_method sparsity_method() {
        return base_traits::sparsity_method;
    }

    static constexpr enum dll::bias_mode bias_mode() {
        return base_traits::bias_mode;
    }

    static constexpr decay_type decay() {
        return base_traits::decay;
    }

    static constexpr bool init_weights() {
        return base_traits::has_init_weights;
    }

    static constexpr bool free_energy() {
        return base_traits::has_free_energy;
    }
};

template <typename T>
using decay_layer_traits = layer_traits<std::decay_t<T>>;

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t get_batch_size(const RBM& rbm) {
    return rbm.batch_size;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t get_batch_size(const RBM&) {
    return RBM::batch_size;
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t get_nc(const RBM& rbm) {
    return rbm.nc;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t get_nc(const RBM&) {
    return RBM::NC;
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t get_k(const RBM& rbm) {
    return rbm.k;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t get_k(const RBM&) {
    return RBM::K;
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t get_nv1(const RBM& rbm) {
    return rbm.nv1;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t get_nv1(const RBM&) {
    return RBM::NV1;
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t get_nv2(const RBM& rbm) {
    return rbm.nv2;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t get_nv2(const RBM&) {
    return RBM::NV2;
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t get_nw1(const RBM& rbm) {
    return rbm.nw1;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t get_nw1(const RBM&) {
    return RBM::NW1;
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t get_nw2(const RBM& rbm) {
    return rbm.nw2;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t get_nw2(const RBM&) {
    return RBM::NW2;
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t num_visible(const RBM& rbm) {
    return rbm.num_visible;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t num_visible(const RBM&) {
    return RBM::desc::num_visible;
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t num_hidden(const RBM& rbm) {
    return rbm.num_hidden;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t num_hidden(const RBM&) {
    return RBM::desc::num_hidden;
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t output_size(const RBM&) {
    return RBM::output_size();
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t output_size(const RBM& rbm) {
    return rbm.output_size();
}

template <typename RBM, cpp_enable_if(layer_traits<RBM>::is_dynamic())>
std::size_t input_size(const RBM& rbm) {
    return rbm.input_size();
}

template <typename RBM, cpp_disable_if(layer_traits<RBM>::is_dynamic())>
constexpr std::size_t input_size(const RBM&) {
    return RBM::input_size();
}

} //end of dll namespace
