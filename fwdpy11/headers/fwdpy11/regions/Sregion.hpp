#ifndef FWDPY11_SREGION_HPP
#define FWDPY11_SREGION_HPP

#include <memory>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <fwdpp/forward_types.hpp>
#include <fwdpp/simfunctions/recycling.hpp>
#include <fwdpy11/types/Mutation.hpp>
#include <fwdpy11/rng.hpp>
#include "Region.hpp"

namespace fwdpy11
{
    struct Sregion
    {
        Region region; // For returning positions
        double scaling;
        Sregion(double b, double e, double w, bool c, std::uint16_t l,
                double s)
            : region(b, e, w, c, l), scaling(s)
        {
            if (!std::isfinite(scaling))
                {
                    throw std::invalid_argument("scaling must be finite");
                }
        }

        inline double
        beg() const
        {
            return region.beg;
        }

        inline double
        end() const
        {
            return region.end;
        }

        inline double
        weight() const
        {
            return region.weight;
        }

        inline double
        label() const
        {
            return region.label;
        }

        virtual std::unique_ptr<Sregion> clone() const = 0;
        virtual std::uint32_t operator()(
            fwdpp::flagged_mutation_queue& /*recycling_bin*/,
            std::vector<Mutation>& /*mutations*/,
            std::unordered_multimap<double, std::uint32_t>& /*lookup_table*/,
            const std::uint32_t /*generation*/,
            const GSLrng_t& /*rng*/) const = 0;
    };
} // namespace fwdpy11

#endif
