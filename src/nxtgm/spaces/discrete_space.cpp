#include  <nxtgm/nxtgm.hpp>
#include <nxtgm/spaces/discrete_space.hpp>

namespace nxtgm
{
    IndicatorVariableMapping::IndicatorVariableMapping(const DiscreteSpace & space)
    :   space_(space),
        mapping_(space.is_simple() ? 1 : space.size())
    {
        if(space_.is_simple())
        {
            mapping_[0] = space[0];
            n_variables_ =  space.size() * space[0];
        }
        else
        {
            n_variables_ = 0;
            for(std::size_t vi = 0; vi <space.size(); ++vi){
                mapping_[vi] = n_variables_;
                n_variables_ += space[vi];
            }
        }
    
    }

    std::size_t IndicatorVariableMapping::operator[](std::size_t variable)const
    {
        // when simple, mapping_[0] is the number of labels
        return (space_.is_simple() ? variable * mapping_[0] : mapping_[variable]);
    }
}
