
#include <nxtgm/constraint_functions/discrete_constraints.hpp>

#include <iostream>

namespace nxtgm
{
    
    PairwiseUniqueLables::PairwiseUniqueLables(discrete_label_type n_labels, energy_type scale) 
    :   n_labels_(n_labels),
        scale_(scale){
    }

    std::size_t PairwiseUniqueLables::arity() const  {
        return 2;
    }

    discrete_label_type PairwiseUniqueLables::shape(std::size_t ) const  {
        return n_labels_;
    }   
    
    std::size_t PairwiseUniqueLables::size() const  {
        return n_labels_ * n_labels_;
    }

    
    std::pair<bool, energy_type>  PairwiseUniqueLables::feasible(const discrete_label_type * discrete_labels) const {
        return discrete_labels[0] != discrete_labels[1] ? std::make_pair(true, energy_type(0)) : std::make_pair(false, scale_);
    }

    std::unique_ptr<DiscreteConstraintFunctionBase> PairwiseUniqueLables::clone() const {
        return std::make_unique<PairwiseUniqueLables>(n_labels_, scale_);
    }

    void PairwiseUniqueLables::add_to_lp(
        IlpData & ilp_data,  
        const std::size_t * indicator_variables_mapping, 
        IlpConstraintBuilderBuffer & /*buffer*/ 
    )const
    {
        for(discrete_label_type l=0; l < n_labels_; ++l){

            ilp_data.begin_constraint(0.0, 1.0);
            ilp_data.add_constraint_coefficient(indicator_variables_mapping[0] + l, 1.0);
            ilp_data.add_constraint_coefficient(indicator_variables_mapping[1] + l, 1.0);

        }
    }

} 
