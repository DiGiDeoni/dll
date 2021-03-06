//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "dll/base_traits.hpp"
#include "dll/rbm/standard_crbm.hpp" //The base class

namespace dll {

/*!
 * \brief Convolutional Restricted Boltzmann Machine
 *
 * This follows the definition of a CRBM by Honglak Lee.
 */
template <typename Desc>
struct dyn_conv_rbm final : public standard_crbm<dyn_conv_rbm<Desc>, Desc> {
    using desc      = Desc;
    using weight    = typename desc::weight;
    using this_type = dyn_conv_rbm<desc>;
    using base_type = standard_crbm<this_type, desc>;

    static constexpr const unit_type visible_unit = desc::visible_unit;
    static constexpr const unit_type hidden_unit  = desc::hidden_unit;

    static constexpr const bool dbn_only = rbm_layer_traits<this_type>::is_dbn_only();

    using w_type = etl::dyn_matrix<weight, 4>;
    using b_type = etl::dyn_vector<weight>;
    using c_type = etl::dyn_vector<weight>;

    using input_t      = typename rbm_base_traits<this_type>::input_t;
    using output_t     = typename rbm_base_traits<this_type>::output_t;
    using input_one_t  = typename rbm_base_traits<this_type>::input_one_t;
    using output_one_t = typename rbm_base_traits<this_type>::output_one_t;

    w_type w; //!< shared weights
    b_type b; //!< hidden biases bk
    c_type c; //!< visible single bias c

    std::unique_ptr<w_type> bak_w; //!< backup shared weights
    std::unique_ptr<b_type> bak_b; //!< backup hidden biases bk
    std::unique_ptr<c_type> bak_c; //!< backup visible single bias c

    etl::dyn_matrix<weight, 3> v1; ///< visible units

    etl::dyn_matrix<weight, 3> h1_a; ///< Activation probabilities of reconstructed hidden units
    etl::dyn_matrix<weight, 3> h1_s; ///< Sampled values of reconstructed hidden units

    etl::dyn_matrix<weight, 3> v2_a; ///< Activation probabilities of reconstructed visible units
    etl::dyn_matrix<weight, 3> v2_s; ///< Sampled values of reconstructed visible units

    etl::dyn_matrix<weight, 3> h2_a; ///< Activation probabilities of reconstructed hidden units
    etl::dyn_matrix<weight, 3> h2_s; ///< Sampled values of reconstructed hidden units

    size_t nv1; ///< The first visible dimension
    size_t nv2; ///< The second visible dimension
    size_t nh1; ///< The first output dimension
    size_t nh2; ///< The second output dimension
    size_t nc;  ///< The number of input channels
    size_t k;   ///< The number of filters

    size_t nw1; ///< The first dimension of the filters
    size_t nw2; ///< The second dimension of the filters

    size_t batch_size = 25;

    dyn_conv_rbm() : base_type() {
        // Nothing else to init
    }

    void prepare_input(input_one_t& input) const {
        input = input_one_t(nc, nv1, nv2);
    }

    void init_layer(size_t nc, size_t nv1, size_t nv2, size_t k, size_t nw1, size_t nw2){
        this->nv1 = nv1;
        this->nv2 = nv2;
        this->nw1 = nw1;
        this->nw2 = nw2;
        this->nc = nc;
        this->k = k;

        this->nh1 = nv1 - nw1 + 1;
        this->nh2 = nv2 - nw2 + 1;

        w = etl::dyn_matrix<weight, 4>(k, nc, nw1, nw2);

        b = etl::dyn_vector<weight>(k);
        c = etl::dyn_vector<weight>(nc);

        v1 = etl::dyn_matrix<weight, 3>(nc, nv1, nv2);

        h1_a = etl::dyn_matrix<weight, 3>(k, nh1, nh2);
        h1_s = etl::dyn_matrix<weight, 3>(k, nh1, nh2);

        v2_a = etl::dyn_matrix<weight, 3>(nc, nv1, nv2);
        v2_s = etl::dyn_matrix<weight, 3>(nc, nv1, nv2);

        h2_a = etl::dyn_matrix<weight, 3>(k, nh1, nh2);
        h2_s = etl::dyn_matrix<weight, 3>(k, nh1, nh2);

        if (is_relu(hidden_unit)) {
            w = etl::normal_generator(0.0, 0.01);
            b = 0.0;
            c = 0.0;
        } else {
            w = 0.01 * etl::normal_generator();
            b = -0.1;
            c = 0.0;
        }
    }

    std::size_t input_size() const noexcept {
        return nv1 * nv2 * nc;
    }

    std::size_t output_size() const noexcept {
        return nh1 * nh2 * k;
    }

    std::size_t parameters() const noexcept {
        return nc * k * nw1 * nw2;
    }

    std::string to_short_string() const {
        char buffer[1024];
        snprintf(
            buffer, 1024, "CRBM(dyn)(%s): %lux%lux%lu -> (%lux%lu) -> %lux%lux%lu",
            to_string(hidden_unit).c_str(), nv1, nv2, nc, nw1, nw2, nh1, nh2, k);
        return {buffer};
    }

    template <typename Input>
    output_t prepare_output(std::size_t samples) const {
        output_t output;
        output.reserve(samples);
        for(size_t i = 0; i < samples; ++i){
            output.emplace_back(k, nh1, nh2);
        }
        return output;
    }

    template <typename Input>
    output_one_t prepare_one_output() const {
        return output_one_t(k, nh1, nh2);
    }

    template <typename DBN>
    void init_sgd_context() {
        this->sgd_context_ptr = std::make_shared<sgd_context<DBN, this_type>>(nc, nv1, nv2, k, nh1, nh2);
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
        static_assert(
            hidden_unit == unit_type::BINARY || hidden_unit == unit_type::RELU || hidden_unit == unit_type::SOFTMAX,
            "Only (C)RBM with binary, softmax or RELU hidden unit are supported");

        static constexpr const function activation_function =
            hidden_unit == unit_type::BINARY
                ? function::SIGMOID
                : (hidden_unit == unit_type::SOFTMAX ? function::SOFTMAX : function::RELU);

        context.errors = f_derivative<activation_function>(context.output) >> context.errors;
    }

    /*!
     * \brief Backpropagate the errors to the previous layers
     * \param output The ETL expression into which write the output
     * \param context The training context
     */
    template<typename H, typename C>
    void backward_batch(H&& output, C& context) const {
        output = etl::conv_4d_full_flipped(context.errors, w);
    }

    /*!
     * \brief Compute the gradients for this layer, if any
     * \param context The trainng context
     */
    template<typename C>
    void compute_gradients(C& context) const {
        context.w_grad = conv_4d_valid_filter_flipped(context.input, context.errors);
        context.b_grad = etl::mean_r(etl::sum_l(context.errors));
    }

    friend base_type;

private:
    auto get_b_rep() const {
        return etl::force_temporary(etl::rep(b, nh1, nh2));
    }

    auto get_c_rep() const {
        return etl::force_temporary(etl::rep(c, nv1, nv2));
    }

    template<typename V>
    auto get_batch_b_rep(V&& v) const {
        const auto batch_size = etl::dim<0>(v);
        return etl::force_temporary(etl::rep_l(etl::rep(b, nh1, nh2), batch_size));
    }

    template<typename H>
    auto get_batch_c_rep(H&& h) const {
        const auto batch_size = etl::dim<0>(h);
        return etl::force_temporary(etl::rep_l(etl::rep(c, nv1, nv2), batch_size));
    }

    template<typename H>
    auto reshape_h_a(H&& h_a) const {
        return etl::reshape(h_a, 1, k, nh1, nh2);
    }

    template<typename V>
    auto reshape_v_a(V&& v_a) const {
        return etl::reshape(v_a, 1, nc, nv1, nv2);
    }

    auto energy_tmp() const {
        return etl::dyn_matrix<weight, 4>(1UL, k, nh1, nh2);
    }

    template <typename V1, typename V2, std::size_t Off = 0>
    static void validate_inputs() {
        static_assert(etl::decay_traits<V1>::dimensions() == 3 + Off, "Inputs must be 3D");
        static_assert(etl::decay_traits<V2>::dimensions() == 3 + Off, "Inputs must be 3D");
    }

    template <typename H1, typename H2, std::size_t Off = 0>
    static void validate_outputs() {
        static_assert(etl::decay_traits<H1>::dimensions() == 3 + Off, "Outputs must be 3D");
        static_assert(etl::decay_traits<H2>::dimensions() == 3 + Off, "Outputs must be 3D");
    }
};

/*!
 * \brief Simple traits to pass information around from the real
 * class to the CRTP class.
 */
template <typename Desc>
struct rbm_base_traits<dyn_conv_rbm<Desc>> {
    using desc      = Desc;
    using weight    = typename desc::weight;

    using input_one_t         = etl::dyn_matrix<weight, 3>;
    using output_one_t        = etl::dyn_matrix<weight, 3>;
    using hidden_output_one_t = output_one_t;
    using input_t             = std::vector<input_one_t>;
    using output_t            = std::vector<output_one_t>;
};

// Declare the traits for the RBM

template<typename Desc>
struct layer_base_traits<dyn_conv_rbm<Desc>> {
    static constexpr bool is_neural     = true;  ///< Indicates if the layer is a neural layer
    static constexpr bool is_dense      = false; ///< Indicates if the layer is dense
    static constexpr bool is_conv       = true;  ///< Indicates if the layer is convolutional
    static constexpr bool is_deconv     = false; ///< Indicates if the layer is deconvolutional
    static constexpr bool is_standard   = false; ///< Indicates if the layer is standard
    static constexpr bool is_rbm        = true;  ///< Indicates if the layer is RBM
    static constexpr bool is_pooling    = false; ///< Indicates if the layer is a pooling layer
    static constexpr bool is_unpooling  = false; ///< Indicates if the layer is an unpooling laye
    static constexpr bool is_transform  = false; ///< Indicates if the layer is a transform layer
    static constexpr bool is_patches    = false; ///< Indicates if the layer is a patches layer
    static constexpr bool is_augment    = false; ///< Indicates if the layer is an augment layer
    static constexpr bool is_dynamic    = true;  ///< Indicates if the layer is dynamic
    static constexpr bool pretrain_last = Desc::hidden_unit != dll::unit_type::SOFTMAX; ///< Indicates if the layer is dynamic
    static constexpr bool sgd_supported = true;  ///< Indicates if the layer is supported by SGD
};

template<typename Desc>
struct rbm_layer_base_traits<dyn_conv_rbm<Desc>> {
    using param = typename Desc::parameters;

    static constexpr bool has_momentum       = param::template contains<momentum>();                            ///< Does the RBM has momentum
    static constexpr bool has_clip_gradients = param::template contains<clip_gradients>();                      ///< Does the RBM has gradient clipping
    static constexpr bool is_parallel_mode   = param::template contains<parallel_mode>();                       ///< Does the RBM is in parallel
    static constexpr bool is_serial          = param::template contains<serial>();                              ///< Does the RBM is in serial mode
    static constexpr bool is_verbose         = param::template contains<verbose>();                             ///< Does the RBM is verbose
    static constexpr bool has_shuffle        = param::template contains<shuffle>();                             ///< Does the RBM has shuffle
    static constexpr bool is_dbn_only        = param::template contains<dbn_only>();                            ///< Does the RBM is only used inside a DBN
    static constexpr bool has_init_weights   = param::template contains<init_weights>();                        ///< Does the RBM use weights initialization
    static constexpr bool has_free_energy    = param::template contains<free_energy>();                         ///< Does the RBM displays the free energy
    static constexpr auto sparsity_method    = get_value_l<sparsity<dll::sparsity_method::NONE>, param>::value; ///< The RBM's sparsity method
    static constexpr auto bias_mode          = get_value_l<bias<dll::bias_mode::NONE>, param>::value;           ///< The RBM's sparsity bias mode
    static constexpr auto decay              = get_value_l<weight_decay<dll::decay_type::NONE>, param>::value;  ///< The RMB's sparsity decay type
    static constexpr bool has_sparsity       = sparsity_method != dll::sparsity_method::NONE;                   ///< Does the RBM has sparsity
};

/*!
 * \brief Specialization of sgd_context for dyn_conv_rbm
 */
template <typename DBN, typename Desc>
struct sgd_context<DBN, dyn_conv_rbm<Desc>> {
    using layer_t = dyn_conv_rbm<Desc>;
    using weight  = typename layer_t::weight;

    static constexpr auto batch_size = DBN::batch_size;

    etl::dyn_matrix<weight, 4> w_grad;
    etl::dyn_matrix<weight, 1> b_grad;

    etl::dyn_matrix<weight, 4> w_inc;
    etl::dyn_matrix<weight, 1> b_inc;

    etl::dyn_matrix<weight, 4> input;
    etl::dyn_matrix<weight, 4> output;
    etl::dyn_matrix<weight, 4> errors;

    sgd_context(size_t nc, size_t nv1, size_t nv2, size_t k, size_t nh1, size_t nh2)
            : w_grad(k, nc, nv1 - nh1 + 1, nv2 - nh2 + 1), b_grad(k),
              w_inc(k, nc, nv1 - nh1 + 1, nv2 - nh2 + 1), b_inc(k),
              input(batch_size, nc, nv1, nv2),
              output(batch_size, k, nh1, nh2), errors(batch_size, k, nh1, nh2) {}
};

} //end of dll namespace
