#include <nxtgm/functions/discrete_energy_functions.hpp>

#include <algorithm>
#include <cmath>

namespace nxtgm
{
// find the arg minimum and second arg minimum value in a range
inline std::pair<std::size_t, std::size_t> arg2min(const energy_type *begin, const energy_type *end)
{
    std::pair<std::size_t, std::size_t> res;
    energy_type min0 = std::numeric_limits<energy_type>::infinity();
    energy_type min1 = min0;
    const auto dist = std::distance(begin, end);
    for (auto i = 0; i < dist; ++i)
    {
        const energy_type val = begin[i];
        if (val < min1)
        {
            if (val < min0)
            {

                min1 = min0;
                res.second = res.first;
                min0 = val;
                res.first = i;
                continue;
            }
            else
            {
                min1 = val;
                res.second = i;
            }
        }
    }
    return res;
}

Potts::Potts(std::size_t num_labels, energy_type beta)
    : num_labels_(num_labels),
      beta_(beta)
{
}

std::size_t Potts::arity() const
{
    return 2;
}

discrete_label_type Potts::shape(std::size_t) const
{
    return num_labels_;
}

std::size_t Potts::size() const
{
    return num_labels_ * num_labels_;
}

energy_type Potts::value(const discrete_label_type *discrete_labels) const
{
    return beta_ * (discrete_labels[0] != discrete_labels[1]);
}
std::unique_ptr<DiscreteEnergyFunctionBase> Potts::clone() const
{
    return std::make_unique<Potts>(num_labels_, beta_);
}

void Potts::copy_values(energy_type *energies) const
{
    for (std::size_t i = 0; i < num_labels_; ++i)
    {
        for (std::size_t j = 0; j < num_labels_; ++j)
        {
            energies[i * num_labels_ + j] = beta_ * (i != j);
        }
    }
}
void Potts::add_values(energy_type *energies) const
{
    for (std::size_t i = 0; i < num_labels_; ++i)
    {
        for (std::size_t j = 0; j < num_labels_; ++j)
        {
            if (i != j)
            {
                energies[i * num_labels_ + j] += beta_;
            }
        }
    }
}

void Potts::compute_factor_to_variable_messages(const energy_type *const *in_messages, energy_type **out_messages) const
{

    if (beta_ >= 0)
    {
        const energy_type minIn0Beta = *std::min_element(in_messages[0], in_messages[0] + num_labels_) + beta_;
        const energy_type minIn1Beta = *std::min_element(in_messages[1], in_messages[1] + num_labels_) + beta_;
        for (discrete_label_type l = 0; l < num_labels_; ++l)
        {
            out_messages[0][l] = std::min(in_messages[1][l], minIn1Beta);
            out_messages[1][l] = std::min(in_messages[0][l], minIn0Beta);
        }
    }
    else
    {
        // base class implementation
        // base_type::compute_factor_to_variable_messages(in_messages, out_messages);

        discrete_label_type aMin0, aMin1, aSMin0, aSMin1;
        std::tie(aMin0, aSMin0) = arg2min(in_messages[0], in_messages[0] + num_labels_);
        std::tie(aMin1, aSMin1) = arg2min(in_messages[1], in_messages[1] + num_labels_);
        const energy_type min0 = in_messages[0][aMin0];
        const energy_type min1 = in_messages[1][aMin1];
        const energy_type min0Beta = min0 + beta_;
        const energy_type min1Beta = min1 + beta_;
        const energy_type smin0Beta = std::min(in_messages[0][aSMin0] + beta_, min0);
        const energy_type smin1Beta = std::min(in_messages[1][aSMin1] + beta_, min1);
        for (discrete_label_type l = 0; l < num_labels_; ++l)
        {
            out_messages[0][l] = aMin1 != l ? min1Beta : smin1Beta;
            out_messages[1][l] = aMin0 != l ? min0Beta : smin0Beta;
        }
    }
}

std::unique_ptr<DiscreteEnergyFunctionBase> Potts::bind(span<const std::size_t> binded_vars,
                                                        span<const discrete_label_type> binded_vars_labels) const
{
    auto values = xt::xtensor<energy_type, 1>::from_shape({std::size_t(num_labels_)});
    values.fill(beta_);
    values[binded_vars_labels[0]] = 0.0;
    return std::make_unique<Unary>(std::move(values));
}

nlohmann::json Potts::serialize_json() const
{
    return {{"type", Potts::serialization_key()}, {"num_labels", num_labels_}, {"beta", beta_}};
}
void Potts::serialize(Serializer &serializer) const
{
    serializer(Potts::serialization_key());
    serializer(num_labels_);
    serializer(beta_);
}

std::unique_ptr<DiscreteEnergyFunctionBase> Potts::deserialize_json(const nlohmann::json &json)
{
    return std::make_unique<Potts>(json["num_labels"], json["beta"]);
}
std::unique_ptr<DiscreteEnergyFunctionBase> Potts::deserialize(Deserializer &deserializer)
{
    auto p = new Potts();
    deserializer(p->num_labels_);
    deserializer(p->beta_);
    return std::unique_ptr<DiscreteEnergyFunctionBase>(p);
}

XArray::XArray(const xarray_type &values)
    : values_(values)
{
}

discrete_label_type XArray::shape(std::size_t index) const
{
    return values_.shape()[index];
}

std::size_t XArray::arity() const
{
    return values_.dimension();
}

std::size_t XArray::size() const
{
    return values_.size();
}

energy_type XArray::value(const discrete_label_type *discrete_labels) const
{
    const_discrete_label_span discrete_labels_span(discrete_labels, values_.dimension());
    return values_[discrete_labels_span];
}
std::unique_ptr<DiscreteEnergyFunctionBase> XArray::clone() const
{
    return std::make_unique<XArray>(values_);
}

void XArray::copy_values(energy_type *energies) const
{
    std::copy(values_.begin(), values_.end(), energies);
}
void XArray::add_values(energy_type *energies) const
{
    std::transform(values_.data(), values_.data() + values_.size(), energies, energies, std::plus<energy_type>());
}

nlohmann::json XArray::serialize_json() const
{

    nlohmann::json shape = nlohmann::json::array();
    for (auto s : values_.shape())
    {
        shape.push_back(s);
    }

    auto values = nlohmann::json::array();
    for (auto it = values_.begin(); it != values_.end(); ++it)
    {
        values.push_back(*it);
    }

    return {{"type", XArray::serialization_key()}, {"shape", shape}, {"values", values}};
}

void XArray::serialize(Serializer &serializer) const
{
    serializer(XArray::serialization_key());
    serializer(values_);
}
std::unique_ptr<DiscreteEnergyFunctionBase> XArray::deserialize(Deserializer &deserializer)
{
    auto f = new XArray();
    deserializer(f->values_);
    return std::unique_ptr<DiscreteEnergyFunctionBase>(f);
}

std::unique_ptr<DiscreteEnergyFunctionBase> XArray::deserialize_json(const nlohmann::json &json)
{
    std::vector<std::size_t> shape;
    for (auto s : json["shape"])
    {
        shape.push_back(s);
    }
    typename XArray::xarray_type array(shape);
    std::copy(json["values"].begin(), json["values"].end(), array.begin());
    return std::make_unique<XArray>(array);
}

discrete_label_type LabelCosts::shape(std::size_t index) const
{
    return costs_.size();
}

std::size_t LabelCosts::arity() const
{
    return arity_;
}

std::size_t LabelCosts::size() const
{
    return std::pow(costs_.size(), arity_);
}

energy_type LabelCosts::value(const discrete_label_type *discrete_labels) const
{

#ifndef NXTGM_NO_THREADS
    std::lock_guard<std::mutex> lck(mtx_);
#endif

    std::fill(is_used_.begin(), is_used_.end(), 0);
    for (std::size_t i = 0; i < arity_; ++i)
    {
        is_used_[discrete_labels[i]] = 1;
    }
    energy_type result = 0;
    for (std::size_t i = 0; i < is_used_.size(); ++i)
    {
        result += is_used_[i] * costs_[i];
    }
    return result;
}

std::unique_ptr<DiscreteEnergyFunctionBase> LabelCosts::clone() const
{
    return std::make_unique<LabelCosts>(arity_, costs_.begin(), costs_.end());
}

void LabelCosts::add_to_lp(IlpData &ilp_data, const std::size_t *indicator_variables_mapping) const
{
    const auto label_indicator_variables_begin = ilp_data.num_variables();

    // add n_labels varialbes
    ilp_data.add_variables(0, 1, costs_.begin(), costs_.end(), false);

    for (std::size_t ai = 0; ai < arity_; ++ai)
    {

        for (discrete_label_type l = 0; l < static_cast<discrete_label_type>(costs_.size()); ++l)
        {

            ilp_data.begin_constraint(0.0, 1.0);
            ilp_data.add_constraint_coefficient(label_indicator_variables_begin + l, 1.0);
            ilp_data.add_constraint_coefficient(indicator_variables_mapping[ai] + l, -1.0);
        }
    }

    for (discrete_label_type l = 0; l < static_cast<discrete_label_type>(costs_.size()); ++l)
    {
        ilp_data.begin_constraint(-1.0 * arity_, 0);
        ilp_data.add_constraint_coefficient(label_indicator_variables_begin + l, 1.0);

        for (std::size_t ai = 0; ai < arity_; ++ai)
        {
            ilp_data.add_constraint_coefficient(indicator_variables_mapping[ai] + l, -1.0);
        }
    }
}

nlohmann::json LabelCosts::serialize_json() const
{
    return {{"type", LabelCosts::serialization_key()}, {"arity", arity_}, {"values", costs_}};
}

void LabelCosts::serialize(Serializer &serializer) const
{
    serializer(LabelCosts::serialization_key());
    serializer(arity_);
    serializer(costs_);
}
std::unique_ptr<DiscreteEnergyFunctionBase> LabelCosts::deserialize(Deserializer &deserializer)
{
    auto f = new LabelCosts();
    deserializer(f->arity_);
    deserializer(f->costs_);
    return std::unique_ptr<DiscreteEnergyFunctionBase>(f);
}

std::unique_ptr<DiscreteEnergyFunctionBase> LabelCosts::deserialize_json(const nlohmann::json &json)
{
    return std::make_unique<LabelCosts>(json["arity"], json["values"].begin(), json["values"].end());
}

std::size_t SparseDiscreteEnergyFunction::arity() const
{
    return data_.dimension();
}
discrete_label_type SparseDiscreteEnergyFunction::shape(std::size_t i) const
{
    return data_.shape(i);
}
std::size_t SparseDiscreteEnergyFunction::size() const
{
    return data_.size();
}
energy_type SparseDiscreteEnergyFunction::value(const discrete_label_type *discrete_labels) const
{
    return data_[discrete_labels];
}

std::unique_ptr<DiscreteEnergyFunctionBase> SparseDiscreteEnergyFunction::clone() const
{
    auto uptr = std::make_unique<SparseDiscreteEnergyFunction>(this->data_.shape());
    uptr.get()->data_.non_zero_entries() = this->data_.non_zero_entries();
    return uptr;
}

void SparseDiscreteEnergyFunction::copy_values(energy_type *energies) const
{
    std::fill(energies, energies + data_.size(), 0);

    for (const auto &item : data_.non_zero_entries())
    {
        energies[item.first] = item.second;
    }
}
void SparseDiscreteEnergyFunction::add_values(energy_type *energies) const
{
    for (const auto &item : data_.non_zero_entries())
    {
        energies[item.first] += item.second;
    }
}

std::unique_ptr<DiscreteEnergyFunctionBase> SparseDiscreteEnergyFunction::deserialize_json(const nlohmann::json &json)
{
    std::vector<discrete_label_type> shape = json["shape"];
    std::unordered_map<std::size_t, energy_type> non_zero_entries = json["non_zero_entries"];

    auto f = std::make_unique<SparseDiscreteEnergyFunction>(shape);
    f.get()->data_.non_zero_entries() = non_zero_entries;
    return f;
}

std::unique_ptr<DiscreteEnergyFunctionBase> SparseDiscreteEnergyFunction::deserialize(Deserializer &deserializer)
{
    std::vector<std::size_t> shape;
    deserializer(shape);
    auto f = std::make_unique<SparseDiscreteEnergyFunction>(shape);
    deserializer(f.get()->data_.non_zero_entries());
    return f;
}

void SparseDiscreteEnergyFunction::serialize(Serializer &serializer) const
{
    serializer(SparseDiscreteEnergyFunction::serialization_key());
    serializer(data_.shape());
    serializer(data_.non_zero_entries());
}

nlohmann::json SparseDiscreteEnergyFunction::serialize_json() const
{
    return {{"type", SparseDiscreteEnergyFunction::serialization_key()},
            {"shape", this->data_.shape()},
            {"non_zero_entries", this->data_.non_zero_entries()}};
}

void SparseDiscreteEnergyFunction::add_to_lp(IlpData &ilp_data, const std::size_t *indicator_variables_mapping) const
{
    const auto arity = this->arity();
    if (arity == 1)
    {
        for (const auto &[label, energy] : data_.non_zero_entries())
        {
            ilp_data.add_objective(indicator_variables_mapping[0] + label, energy);
        }
    }
    else
    {
        small_arity_vector<discrete_label_type> labels(arity);
        // for each entry in energies_
        for (const auto &[flat_index, energy] : data_.non_zero_entries())
        {
            // convert flat index to discrete labels
            data_.multindex_from_flat_index(flat_index, labels.data());

            // add indicator variable
            auto indicator_var =
                /* it MUST be an integer variable */
                ilp_data.add_variable(0.0, 1.0, double(energy), true);

            ilp_data.begin_constraint(0.0, double(arity) - 1);
            ilp_data.add_constraint_coefficient(indicator_var, -1.0 * arity);

            // add constraint
            for (std::size_t i = 0; i < arity; ++i)
            {
                ilp_data.add_constraint_coefficient(indicator_variables_mapping[i] + labels[i], 1.0);
            }
        }
    }
}

void SparseDiscreteEnergyFunction::set_energy(std::initializer_list<discrete_label_type> labels, energy_type energy)
{
    set_energy(labels.begin(), energy);
}

void SparseDiscreteEnergyFunction::set_energy(const discrete_label_type *labels, energy_type energy)
{
    data_[labels] = energy;
}

} // namespace nxtgm
