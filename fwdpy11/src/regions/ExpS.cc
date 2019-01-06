#include <pybind11/pybind11.h>
#include <fwdpy11/regions/ExpS.hpp>

namespace py = pybind11;

void
init_ExpS(py::module& m)
{
    py::class_<fwdpy11::ExpS, fwdpy11::Sregion>(
        m, "ExpS", "Exponential distribution of effect sizes")
        .def(py::init<double, double, double, double, double, bool,
                      std::uint16_t, double>(),
             py::arg("beg"), py::arg("end"), py::arg("weight"),
             py::arg("mean"), py::arg("h") = 1.0, py::arg("coupled") = true,
             py::arg("label") = 0, py::arg("scaling") = 1.0,
             R"delim(
        Constructor

        :param beg: the beginning of the region
        :param end: the end of the region
        :param weight: the weight to assign
        :param mean: the mean selection coefficient
        :param h: the dominance
        :param coupled: if True, the weight is converted to (end-beg)*weight
        :param label: Not relevant to recombining regions.
            Otherwise, this value will be used to take
            mutations from this region.
        :param scaling: The scaling of the DFE

        When coupled is True, the "weight" may be
        interpreted as a "per base pair"
        (or per unit, generally speaking) term.

        Example:

        .. testcode::

            #A simple case
            import fwdpy11
            #s is exp(-0.1) and recessive
            expS = fwdpy11.ExpS(0,1,1,-0.1,0)
        )delim")
        .def_readonly("mean", &fwdpy11::ExpS::mean)
        .def_readonly("h", &fwdpy11::ExpS::dominance);
}
