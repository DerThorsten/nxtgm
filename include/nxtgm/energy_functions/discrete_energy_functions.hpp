#pragma once

#include <nxtgm/nxtgm.hpp>
#include <nxtgm/energy_functions/discrete_energy_function_base.hpp>
#include <vector>
#include <xtensor/xtensor.hpp>
#include <xtensor/xarray.hpp>
#include <cstdint>

#ifndef NXTGM_NO_THREADS
#include <mutex>          // std::mutex
#endif

namespace nxtgm{



    class Unary : public DiscreteEnergyFunctionBase{
        public:
        using base_type = DiscreteEnergyFunctionBase;
        using base_type::energy;
        
        template<typename ITER>
        Unary(ITER begin, ITER end) : 
            values_(begin, end) 
        {
        }

        Unary(const std::vector<energy_type>& values);
        std::size_t arity() const override;
        discrete_label_type shape(std::size_t ) const override;
        energy_type energy(const discrete_label_type * discrete_labels) const override;

        std::unique_ptr<DiscreteEnergyFunctionBase> clone() const override;
        private:
        std::vector<energy_type> values_;
    };






    class Potts : public DiscreteEnergyFunctionBase{
        public:
        using base_type = DiscreteEnergyFunctionBase;
        using base_type::energy;
        
        Potts(std::size_t num_labels, energy_type beta);

        std::size_t arity() const override;
        discrete_label_type shape(std::size_t ) const override;
        std::size_t size() const override;
        energy_type energy(const discrete_label_type * discrete_labels) const override;
        std::unique_ptr<DiscreteEnergyFunctionBase> clone() const override;
        private:
        std::size_t num_labels_;
        energy_type beta_;
        
    };







    template<std::size_t ARITY>
    class XTensor : public DiscreteEnergyFunctionBase
    {
        public:
        using base_type = DiscreteEnergyFunctionBase;
        using base_type::energy;

        using xtensor_type = xt::xtensor<energy_type, ARITY>;

        XTensor(const xtensor_type & values) : 
            values_(values) 
        {
        }
        template<class TENSOR>
        XTensor(TENSOR && values) : 
            values_(std::forward<TENSOR>(values)) 
        {
        }

        discrete_label_type shape(std::size_t index) const override{
            return values_.shape()[index];   
        }

        std::size_t arity() const override {
            return ARITY;
        }   
    
        std::size_t size() const override {
            return values_.size();
        }

        energy_type energy(const discrete_label_type * discrete_labels) const override {
            const_discrete_label_span fixed(discrete_labels, ARITY);
            return values_[fixed];
        }
        std::unique_ptr<DiscreteEnergyFunctionBase> clone() const override{
            return std::make_unique<XTensor<ARITY>>(values_);
        }
        private:
            xtensor_type values_;
    };

    class Xarray : public DiscreteEnergyFunctionBase
    {
        public:
        using base_type = DiscreteEnergyFunctionBase;
        using base_type::energy;

        using xarray_type = xt::xarray<energy_type>;

        template<class TENSOR>
        Xarray(TENSOR && values) : 
            values_(std::forward<TENSOR>(values)) 
        {
        }

        Xarray(const xarray_type & values);
        discrete_label_type shape(std::size_t index) const override;

        std::size_t arity() const override;

        std::size_t size() const override;

        energy_type energy(const discrete_label_type * discrete_labels) const override;
        std::unique_ptr<DiscreteEnergyFunctionBase> clone() const override;
        private:
            xarray_type values_;
    };


    // label costs
    // pay a cost once a label is used, but only
    // once, no matter how often it is used
    class LabelCosts : public DiscreteEnergyFunctionBase
    {    
        public:
        using base_type = DiscreteEnergyFunctionBase;
        using base_type::energy;

        template<typename ITER>
        LabelCosts(std::size_t arity, ITER begin , ITER end) : 
            arity_(arity),
            costs_(begin, end),
            is_used_(costs_.size(), 0)
        {
        }
        
        discrete_label_type shape(std::size_t index) const override;

        std::size_t arity() const override;

        std::size_t size() const override;

        energy_type energy(const discrete_label_type * discrete_labels) const override;

        std::unique_ptr<DiscreteEnergyFunctionBase> clone() const override;
        void add_to_lp(
            IlpData & ilp_data, 
            const std::size_t * indicator_variables_mapping,
            IlpFactorBuilderBuffer & buffer
        ) const override;

        private:
            std::size_t arity_;
            std::vector<energy_type> costs_;
            mutable std::vector<std::uint8_t> is_used_;

#ifndef NXTGM_NO_THREADS
            mutable std::mutex mtx_;
#endif
    };
}