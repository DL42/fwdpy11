#ifndef FWDPY11_DISCRETE_DEMOGRAPHY_HPP
#define FWDPY11_DISCRETE_DEMOGRAPHY_HPP

#include <cstdint>
#include <vector>
#include <memory>
#include <pybind11/pybind11.h>

namespace fwdpy11
{
    class DiscreteDemography
    {
        private:
            const std::uint32_t maxdemes_;
            std::vector<std::uint32_t> deme_sizes_;
            std::vector<double> selfing_rates_, growth_rates_;
        public:
            virtual ~DiscreteDemography() = default;

            virtual pybind11::object pickle() const = 0;
            virtual std::unique_ptr<DiscreteDemography> clone() const = 0;
    };
}

#endif

