//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "dll/base_traits.hpp"
#include "dll/layer.hpp"

namespace dll {

/*!
 * \brief Layer to cut images into patches.
 */
template <typename Desc>
struct dyn_patches_layer : layer<dyn_patches_layer<Desc>> {
    using desc = Desc;

    using weight = typename desc::weight;

    using input_one_t = etl::dyn_matrix<weight, 3>;
    using input_t     = std::vector<input_one_t>;

    using output_one_t = std::vector<etl::dyn_matrix<weight, 3>>;
    using output_t     = std::vector<output_one_t>;

    std::size_t width;
    std::size_t height;
    std::size_t v_stride;
    std::size_t h_stride;

    dyn_patches_layer() = default;

    void init_layer(std::size_t width, std::size_t height, std::size_t v_stride, std::size_t h_stride){
        this->width    = width;
        this->height   = height;
        this->v_stride = v_stride;
        this->h_stride = h_stride;
    }

    std::string to_short_string() const {
        char buffer[1024];
        snprintf(buffer, 1024, "Patches(dyn) -> (%lu:%lux%lu:%lu)", height, v_stride, width, h_stride);
        return {buffer};
    }

    std::size_t output_size() const noexcept {
        return width * height;
    }

    void activate_hidden(output_one_t& h_a, const input_one_t& input) const {
        cpp_assert(etl::dim<0>(input) == 1, "Only one channel is supported for now");

        h_a.clear();

        for (std::size_t y = 0; y + height <= etl::dim<1>(input); y += v_stride) {
            for (std::size_t x = 0; x + width <= etl::dim<2>(input); x += h_stride) {
                h_a.emplace_back();

                auto& patch = h_a.back();

                patch.inherit_if_null(etl::dyn_matrix<weight,3>(1UL, height, width));

                for (std::size_t yy = 0; yy < height; ++yy) {
                    for (std::size_t xx = 0; xx < width; ++xx) {
                        patch(0, yy, xx) = input(0, y + yy, x + xx);
                    }
                }
            }
        }
    }

    void activate_many(output_t& h_a, const input_t& input) const {
        for (std::size_t i = 0; i < input.size(); ++i) {
            activate_one(input[i], h_a[i]);
        }
    }

    template <typename Input>
    output_t prepare_output(std::size_t samples) const {
        output_t output;
        output.resize(samples);
        return output;
    }

    template <typename Input>
    output_one_t prepare_one_output() const {
        return output_one_t();
    }

    template<typename DRBM>
    static void dyn_init(DRBM&){
        //Nothing to change
    }
};

// Declare the traits for the layer

template<typename Desc>
struct layer_base_traits<dyn_patches_layer<Desc>> {
    static constexpr bool is_neural     = false; ///< Indicates if the layer is a neural layer
    static constexpr bool is_dense      = false; ///< Indicates if the layer is dense
    static constexpr bool is_conv       = false; ///< Indicates if the layer is convolutional
    static constexpr bool is_deconv     = false; ///< Indicates if the layer is deconvolutional
    static constexpr bool is_standard   = false; ///< Indicates if the layer is standard
    static constexpr bool is_rbm        = false; ///< Indicates if the layer is RBM
    static constexpr bool is_pooling    = false; ///< Indicates if the layer is a pooling layer
    static constexpr bool is_unpooling  = false; ///< Indicates if the layer is an unpooling laye
    static constexpr bool is_transform  = false;  ///< Indicates if the layer is a transform layer
    static constexpr bool is_patches    = true; ///< Indicates if the layer is a patches layer
    static constexpr bool is_augment    = false; ///< Indicates if the layer is an augment layer
    static constexpr bool is_dynamic    = true; ///< Indicates if the layer is dynamic
    static constexpr bool pretrain_last = false; ///< Indicates if the layer is dynamic
    static constexpr bool sgd_supported = true;  ///< Indicates if the layer is supported by SGD
};

} //end of dll namespace
