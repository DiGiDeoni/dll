//=======================================================================
// Copyright (c) 1814-1816 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * This is mostly a compilation test to ensure that DBN is accepting
 * enough input types
 */

#include <vector>
#include <list>
#include <deque>

#include "catch.hpp"
#include "template_test.hpp"

#include "dll/neural/conv_layer.hpp"
#include "dll/neural/dense_layer.hpp"
#include "dll/dbn.hpp"
#include "dll/trainer/stochastic_gradient_descent.hpp"

#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

namespace {

struct dbn_double {
    using dbn_t =
        dll::dbn_desc<
            dll::dbn_layers<
                dll::conv_desc<1, 28, 28, 5, 11, 11, dll::weight_type<double>>::layer_t,
                dll::dense_desc<5 * 18 * 18, 10, dll::weight_type<double>>::layer_t
            >
            , dll::trainer<dll::sgd_trainer>
            , dll::batch_size<10>
            , dll::momentum
        >::dbn_t;

    static void init(dbn_t& net){
        net.learning_rate = 0.05;
        net.initial_momentum = 0.9;
    }
};

struct dbn_float {
    using dbn_t =
        dll::dbn_desc<
            dll::dbn_layers<
                dll::conv_desc<1, 28, 28, 5, 11, 11, dll::weight_type<float>>::layer_t,
                dll::dense_desc<5 * 18 * 18, 10, dll::weight_type<float>>::layer_t
            >
            , dll::trainer<dll::sgd_trainer>
            , dll::batch_size<10>
            , dll::momentum
        >::dbn_t;

    static void init(dbn_t& net){
        net.learning_rate = 0.05;
        net.initial_momentum = 0.9;
    }
};

struct dyn_dbn_float {
    using dbn_t =
        dll::dbn_desc<
            dll::dbn_layers<
                dll::dyn_conv_desc<dll::weight_type<float>>::layer_t,
                dll::dyn_dense_desc<dll::weight_type<float>>::layer_t
            >
            , dll::trainer<dll::sgd_trainer>
            , dll::batch_size<10>
            , dll::momentum
        >::dbn_t;

    static void init(dbn_t& net){
        net.initial_momentum = 0.9;
        net.learning_rate = 0.05;
        net.template init_layer<0>(1, 28, 28, 5, 11, 11);
        net.template init_layer<1>(5 * 18 * 18, 10);
    }
};

struct dyn_dbn_double {
    using dbn_t =
        dll::dbn_desc<
            dll::dbn_layers<
                dll::dyn_conv_desc<dll::weight_type<double>>::layer_t,
                dll::dyn_dense_desc<dll::weight_type<double>>::layer_t
            >
            , dll::trainer<dll::sgd_trainer>
            , dll::batch_size<10>
            , dll::momentum
        >::dbn_t;

    static void init(dbn_t& net){
        net.initial_momentum = 0.9;
        net.learning_rate = 0.05;
        net.template init_layer<0>(1, 28, 28, 5, 11, 11);
        net.template init_layer<1>(5 * 18 * 18, 10);
    }
};

} // end of anonymous namespace

#define TYPES_TEST_PREFIX "conv_dbn"
#define TYPES_TEST_T1 dbn_float
#define TYPES_TEST_T2 dbn_double
#define TYPES_TEST_T3 dyn_dbn_float
#define TYPES_TEST_T4 dyn_dbn_double

#include "dbn_types_test.inl"
