//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "dll/neural_layer.hpp"

namespace dll {

/*!
 * \brief Standard convolutional layer of neural network.
 */
template <typename Desc>
struct deconv_layer final : neural_layer<deconv_layer<Desc>, Desc> {
    using desc      = Desc;
    using weight    = typename desc::weight;
    using this_type = deconv_layer<desc>;
    using base_type = neural_layer<this_type, desc>;

    static constexpr size_t NC  = desc::NC;  ///< The number of input channels
    static constexpr size_t NV1 = desc::NV1; ///< The first dimension of the visible units
    static constexpr size_t NV2 = desc::NV2; ///< The second dimension of the visible units
    static constexpr size_t K   = desc::K;   ///< The number of filters
    static constexpr size_t NW1 = desc::NW1; ///< The first dimension of the hidden units
    static constexpr size_t NW2 = desc::NW2; ///< The second dimension of the hidden units

    static constexpr size_t NH1 = NV1 + NW1 - 1; //By definition
    static constexpr size_t NH2 = NV2 + NW2 - 1; //By definition

    static constexpr auto activation_function = desc::activation_function;
    static constexpr auto w_initializer       = desc::w_initializer;
    static constexpr auto b_initializer       = desc::b_initializer;

    using input_one_t  = etl::fast_dyn_matrix<weight, NC, NV1, NV2>;
    using output_one_t = etl::fast_dyn_matrix<weight, K, NH1, NH2>;
    using input_t      = std::vector<input_one_t>;
    using output_t     = std::vector<output_one_t>;

    using w_type = etl::fast_matrix<weight, NC, K, NW1, NW2>;
    using b_type = etl::fast_matrix<weight, K>;

    //Weights and biases
    w_type w; //!< Weights
    b_type b; //!< Hidden biases

    //Backup weights and biases
    std::unique_ptr<w_type> bak_w; //!< Backup Weights
    std::unique_ptr<b_type> bak_b; //!< Backup Hidden biases

    /*!
     * \brief Initialize a conv layer with basic weights.
     */
    deconv_layer() : base_type() {
        initializer_function<w_initializer>::initialize(w, input_size(), output_size());
        initializer_function<b_initializer>::initialize(b, input_size(), output_size());
    }

    static constexpr size_t input_size() noexcept {
        return NC * NV1 * NV2;
    }

    static constexpr size_t output_size() noexcept {
        return K * NH1 * NH2;
    }

    static constexpr size_t parameters() noexcept {
        return K * NW1 * NW2;
    }

    static std::string to_short_string() {
        char buffer[1024];
        snprintf(buffer, 1024, "Deconv: %lux%lux%lu -> (%lux%lux%lu) -> %s -> %lux%lux%lu", NC, NV1, NV2, K, NW1, NW2, to_string(activation_function).c_str(), K, NH1, NH2);
        return {buffer};
    }

    void activate_hidden(output_one_t& output, const input_one_t& v) const {
        auto b_rep = etl::force_temporary(etl::rep<NH1, NH2>(b));

        etl::reshape<1, K, NH1, NH2>(output) = etl::conv_4d_full_flipped(etl::reshape<1, NC, NV1, NV2>(v), w);

        output = f_activate<activation_function>(b_rep + output);
    }

    template <typename V>
    void activate_hidden(output_one_t& output, const V& v) const {
        decltype(auto) converted = converter_one<V, input_one_t>::convert(*this, v);
        activate_hidden(output, converted);
    }

    template <typename H1, typename V>
    void batch_activate_hidden(H1&& output, const V& v) const {
        output = etl::conv_4d_full_flipped(v, w);

        static constexpr auto batch_size = etl::decay_traits<H1>::template dim<0>();

        auto b_rep = etl::force_temporary(etl::rep_l<batch_size>(etl::rep<NH1, NH2>(b)));

        output = f_activate<activation_function>(b_rep + output);
    }

    template <typename Input>
    output_one_t prepare_one_output() const {
        return {};
    }

    template <typename Input>
    static output_t prepare_output(size_t samples) {
        return output_t{samples};
    }

    template<typename DRBM>
    static void dyn_init(DRBM& dyn){
        dyn.init_layer(NC, NV1, NV2, K, NW1, NW2);
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
        if(activation_function != function::IDENTITY){
            context.errors = f_derivative<activation_function>(context.output) >> context.errors;
        }
    }

    /*!
     * \brief Backpropagate the errors to the previous layers
     * \param output The ETL expression into which write the output
     * \param context The training context
     */
    template<typename H, typename C, cpp_enable_if(etl::decay_traits<H>::dimensions() == 4)>
    void backward_batch(H&& output, C& context) const {
        output = etl::conv_4d_valid_flipped(context.errors, w);
    }

    /*!
     * \brief Backpropagate the errors to the previous layers
     * \param output The ETL expression into which write the output
     * \param context The training context
     */
    template<typename H, typename C, cpp_enable_if(etl::decay_traits<H>::dimensions() != 4)>
    void backward_batch(H&& output, C& context) const {
        static constexpr auto B = etl::decay_traits<H>::template dim<0>();
        etl::reshape<B, NC, NV1, NV2>(output) = etl::conv_4d_valid_flipped(context.errors, w);
    }

    /*!
     * \brief Compute the gradients for this layer, if any
     * \param context The trainng context
     */
    template<typename C>
    void compute_gradients(C& context) const {
        //context.w_grad = conv_4d_valid_filter(context.errors, context.input);
        context.b_grad = etl::mean_r(etl::sum_l(context.errors));
    }
};

//Allow odr-use of the constexpr static members

template <typename Desc>
const size_t deconv_layer<Desc>::NV1;

template <typename Desc>
const size_t deconv_layer<Desc>::NV2;

template <typename Desc>
const size_t deconv_layer<Desc>::NH1;

template <typename Desc>
const size_t deconv_layer<Desc>::NH2;

template <typename Desc>
const size_t deconv_layer<Desc>::NC;

template <typename Desc>
const size_t deconv_layer<Desc>::NW1;

template <typename Desc>
const size_t deconv_layer<Desc>::NW2;

template <typename Desc>
const size_t deconv_layer<Desc>::K;

// Declare the traits for the Layer

template<typename Desc>
struct layer_base_traits<deconv_layer<Desc>> {
    static constexpr bool is_neural     = true;  ///< Indicates if the layer is a neural layer
    static constexpr bool is_dense      = false; ///< Indicates if the layer is dense
    static constexpr bool is_conv       = false; ///< Indicates if the layer is convolutional
    static constexpr bool is_deconv     = true;  ///< Indicates if the layer is deconvolutional
    static constexpr bool is_standard   = true;  ///< Indicates if the layer is standard
    static constexpr bool is_rbm        = false; ///< Indicates if the layer is RBM
    static constexpr bool is_pooling    = false; ///< Indicates if the layer is a pooling layer
    static constexpr bool is_unpooling  = false; ///< Indicates if the layer is an unpooling laye
    static constexpr bool is_transform  = false; ///< Indicates if the layer is a transform layer
    static constexpr bool is_patches    = false; ///< Indicates if the layer is a patches layer
    static constexpr bool is_augment    = false; ///< Indicates if the layer is an augment layer
    static constexpr bool is_dynamic    = false; ///< Indicates if the layer is dynamic
    static constexpr bool pretrain_last = false; ///< Indicates if the layer is dynamic
    static constexpr bool sgd_supported = true;  ///< Indicates if the layer is supported by SGD
};

/*!
 * \brief Specialization of the SGD Context for the deconvolutional layer
 */
template <typename DBN, typename Desc>
struct sgd_context<DBN, deconv_layer<Desc>> {
    using layer_t = deconv_layer<Desc>;
    using weight  = typename layer_t::weight;

    static constexpr size_t NV1 = layer_t::NV1;
    static constexpr size_t NV2 = layer_t::NV2;
    static constexpr size_t NH1 = layer_t::NH1;
    static constexpr size_t NH2 = layer_t::NH2;
    static constexpr size_t NW1 = layer_t::NW1;
    static constexpr size_t NW2 = layer_t::NW2;
    static constexpr size_t NC  = layer_t::NC;
    static constexpr size_t K   = layer_t::K;

    static constexpr auto batch_size = DBN::batch_size;

    etl::fast_matrix<weight, NC, K, NW1, NW2> w_grad;
    etl::fast_matrix<weight, K> b_grad;

    etl::fast_matrix<weight, NC, K, NW1, NW2> w_inc;
    etl::fast_matrix<weight, K> b_inc;

    etl::fast_matrix<weight, batch_size, NC, NV1, NV2> input;
    etl::fast_matrix<weight, batch_size, K, NH1, NH2> output;
    etl::fast_matrix<weight, batch_size, K, NH1, NH2> errors;

    sgd_context()
            : w_inc(0.0), b_inc(0.0), output(0.0), errors(0.0) {}
};

} //end of dll namespace
