#include <nxtgm/functions/array_constraint_function.hpp>
#include <nxtgm/functions/discrete_constraint_function_base.hpp>
#include <nxtgm/functions/unique_labels_constraint_function.hpp>
#include <nxtgm/utils/tuple_for_each.hpp>

namespace nxtgm
{

void DiscreteConstraintFunctionBase::add_to_lp(IlpData &ilp_data, const std::size_t *indicator_variables_mapping) const
{
    throw std::runtime_error("Not implemented");
}

std::unique_ptr<DiscreteConstraintFunctionBase> DiscreteConstraintFunctionBase::bind(
    const span<std::size_t> &binded_vars, const span<discrete_label_type> &binded_vars_labels) const
{
    throw std::runtime_error("DiscreteConstraintFunctionBase::bind is not implemented");
}

template <class T>
struct Identity
{
    using type = T;
};

using AllInternalDiscreteConstraintFunctionTypes =
    std::tuple<Identity<UniqueLables>, Identity<ArrayDiscreteConstraintFunction>>;

std::unique_ptr<DiscreteConstraintFunctionBase> discrete_constraint_function_deserialize_json(
    const nlohmann::json &json, const DiscretConstraintFunctionSerializationFactory &user_factory)
{
    const std::string type = json.at("type").get<std::string>();
    std::unique_ptr<DiscreteConstraintFunctionBase> result;

    AllInternalDiscreteConstraintFunctionTypes all_types;
    tuple_breakable_for_each(all_types, [&](auto &&tuple_element) {
        using function_type = typename std::decay_t<decltype(tuple_element)>::type;
        const std::string name = function_type::serialization_key();
        if (name == type)
        {
            result = std::move(function_type::deserialize_json(json));
            return false;
        }
        return true;
    });
    if (result)
    {
        return std::move(result);
    }
    else
    {
        // check if in user factory
        auto factory = user_factory.find(type);
        if (factory == user_factory.end())
        {
            throw std::runtime_error("Unknown type: `" + type + "`");
        }
        else
        {
            return factory->second(json);
        }
    }
}

std::unique_ptr<DiscreteConstraintFunctionBase> discrete_constraint_function_deserialize(Deserializer &deserializer)
{
    std::string type;
    deserializer(type);

    std::unique_ptr<DiscreteConstraintFunctionBase> result;

    AllInternalDiscreteConstraintFunctionTypes all_types;
    tuple_breakable_for_each(all_types, [&](auto &&tuple_element) {
        using function_type = typename std::decay_t<decltype(tuple_element)>::type;
        const std::string name = function_type::serialization_key();
        if (name == type)
        {
            result = std::move(function_type::deserialize(deserializer));
            return false;
        }
        return true;
    });
    if (result)
    {
        return std::move(result);
    }
    else
    {
        throw std::runtime_error("Unknown type: `" + type + "`");
    }
}

} // namespace nxtgm
